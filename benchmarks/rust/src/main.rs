use std::{
  collections::HashMap,
  env, fs,
  io::Read,
  path::{Path, PathBuf},
  time::Instant,
};

use fastalp::{compress_into, decompress_into};

#[derive(Debug)]
struct DatasetConfig {
  _id: usize,
  column_name: String,
  data_type: String,
  path: PathBuf,
  file_type: String,
}

fn parse_config(csv_path: &Path) -> Vec<DatasetConfig> {
  let content = match fs::read_to_string(csv_path) {
    Ok(c) => c,
    Err(e) => {
      eprintln!("Failed to read dataset config {:?}: {}", csv_path, e);
      return Vec::new();
    }
  };

  let mut configs = Vec::new();
  for (idx, line) in content.lines().enumerate() {
    let line = line.trim();
    if line.is_empty() || idx == 0 {
      continue;
    }
    let parts: Vec<&str> = line.split(',').map(|s| s.trim()).collect();
    if parts.len() < 5 {
      continue;
    }
    let id = parts[0].parse::<usize>().unwrap_or(idx);
    let column_name = parts[1].to_string();
    let data_type = parts[2].to_lowercase();
    let path = PathBuf::from(parts[3]);
    let file_type = parts[4].to_lowercase();
    configs.push(DatasetConfig {
      _id: id,
      column_name,
      data_type,
      path,
      file_type,
    });
  }
  configs
}

fn load_doubles(config: &DatasetConfig) -> Option<Vec<f64>> {
  if !config.path.exists() {
    return None;
  }
  if config.file_type == "binary" {
    let mut file = fs::File::open(&config.path).ok()?;
    let mut bytes = Vec::new();
    file.read_to_end(&mut bytes).ok()?;
    if bytes.len() % 8 != 0 {
      return None;
    }
    let count = bytes.len() / 8;
    let mut vals = Vec::with_capacity(count);
    for chunk in bytes.chunks_exact(8) {
      let arr: [u8; 8] = chunk.try_into().ok()?;
      vals.push(f64::from_le_bytes(arr));
    }
    Some(vals)
  } else {
    let content = fs::read_to_string(&config.path).ok()?;
    let vals: Vec<f64> = content
      .lines()
      .filter_map(|l| {
        let t = l.trim();
        if t.is_empty() {
          None
        } else {
          t.parse::<f64>().ok()
        }
      })
      .collect();
    if vals.is_empty() {
      None
    } else {
      Some(vals)
    }
  }
}

fn load_floats(config: &DatasetConfig) -> Option<Vec<f32>> {
  if !config.path.exists() {
    return None;
  }
  if config.file_type == "binary" {
    let mut file = fs::File::open(&config.path).ok()?;
    let mut bytes = Vec::new();
    file.read_to_end(&mut bytes).ok()?;
    if bytes.len() % 4 != 0 {
      return None;
    }
    let count = bytes.len() / 4;
    let mut vals = Vec::with_capacity(count);
    for chunk in bytes.chunks_exact(4) {
      let arr: [u8; 4] = chunk.try_into().ok()?;
      vals.push(f32::from_le_bytes(arr));
    }
    Some(vals)
  } else {
    let content = fs::read_to_string(&config.path).ok()?;
    let vals: Vec<f32> = content
      .lines()
      .filter_map(|l| {
        let t = l.trim();
        if t.is_empty() {
          None
        } else {
          t.parse::<f32>().ok()
        }
      })
      .collect();
    if vals.is_empty() {
      None
    } else {
      Some(vals)
    }
  }
}

fn load_cpp_results(path: &Path) -> HashMap<String, f64> {
  let mut map = HashMap::new();
  if let Ok(content) = fs::read_to_string(path) {
    for (idx, line) in content.lines().enumerate() {
      if idx == 0 || line.trim().is_empty() {
        continue;
      }
      let parts: Vec<&str> = line.split(',').map(|s| s.trim()).collect();
      if parts.len() >= 4 {
        let col = parts[1].to_string();
        if let Ok(size) = parts[3].parse::<f64>() {
          map.insert(col, size);
        }
      }
    }
  }
  map
}

fn main() {
  let manifest_dir = PathBuf::from(env!("CARGO_MANIFEST_DIR"));
  let benchmarks_dir = manifest_dir.parent().unwrap();
  let default_config_path = benchmarks_dir.join("your_own_dataset.csv");

  let config_path = env::args()
    .nth(1)
    .map(PathBuf::from)
    .unwrap_or(default_config_path);

  println!("fastalp (Rust implementation of ALP) Benchmark Comparison Runner");
  println!("Loading dataset configuration from: {:?}", config_path);

  let configs = parse_config(&config_path);
  if configs.is_empty() {
    println!("No datasets found in config file.");
    return;
  }

  let cpp_results_path = benchmarks_dir.join("your_own_dataset_result.csv");
  let cpp_sizes = load_cpp_results(&cpp_results_path);

  println!(
    "{:<24} | {:>8} | {:>14} | {:>14} | {:>12} | {:>12}",
    "Dataset", "Type", "C++ ALP (b/val)", "fastalp (b/val)", "Enc (GB/s)", "Dec (GB/s)"
  );
  println!("{:-<96}", "");

  let mut comp_buf = Vec::with_capacity(65536);
  let mut dec_f64_buf = Vec::with_capacity(1024);
  let mut dec_f32_buf = Vec::with_capacity(1024);

  let mut tested_count = 0;

  for cfg in &configs {
    if cfg.data_type == "double" {
      let data = match load_doubles(cfg) {
        Some(d) => d,
        None => continue,
      };
      if data.is_empty() {
        continue;
      }

      // Warmup & verify lossless bit-exact roundtrip
      comp_buf.clear();
      compress_into(&data, &mut comp_buf);
      dec_f64_buf.clear();
      let _ = decompress_into::<f64>(&comp_buf, &mut dec_f64_buf);
      for i in 0..data.len() {
        if data[i].to_bits() != dec_f64_buf[i].to_bits() {
          if data[i].is_nan() && dec_f64_buf[i].is_nan() {
            continue;
          }
          panic!("Lossless mismatch at index {} in dataset {}", i, cfg.column_name);
        }
      }

      let iters = 1000;
      let raw_bytes = data.len() * 8;

      let start_enc = Instant::now();
      for _ in 0..iters {
        comp_buf.clear();
        compress_into(&data, &mut comp_buf);
      }
      let enc_dur = start_enc.elapsed();
      let enc_gb_s = (raw_bytes as f64 * iters as f64) / (enc_dur.as_secs_f64() * 1e9);

      let start_dec = Instant::now();
      for _ in 0..iters {
        dec_f64_buf.clear();
        let _ = decompress_into::<f64>(&comp_buf, &mut dec_f64_buf);
      }
      let dec_dur = start_dec.elapsed();
      let dec_gb_s = (raw_bytes as f64 * iters as f64) / (dec_dur.as_secs_f64() * 1e9);

      let fastalp_bits_per_val = (comp_buf.len() * 8) as f64 / data.len() as f64;
      let cpp_size_str = cpp_sizes
        .get(&cfg.column_name)
        .map(|s| format!("{:.2}", s))
        .unwrap_or_else(|| "N/A".to_string());

      println!(
        "{:<24} | {:>8} | {:>14} | {:>14.2} | {:>12.2} | {:>12.2}",
        cfg.column_name, cfg.data_type, cpp_size_str, fastalp_bits_per_val, enc_gb_s, dec_gb_s
      );
      tested_count += 1;
    } else if cfg.data_type == "float" {
      let data = match load_floats(cfg) {
        Some(d) => d,
        None => continue,
      };
      if data.is_empty() {
        continue;
      }

      comp_buf.clear();
      compress_into(&data, &mut comp_buf);
      dec_f32_buf.clear();
      let _ = decompress_into::<f32>(&comp_buf, &mut dec_f32_buf);
      for i in 0..data.len() {
        if data[i].to_bits() != dec_f32_buf[i].to_bits() {
          if data[i].is_nan() && dec_f32_buf[i].is_nan() {
            continue;
          }
          panic!("Lossless mismatch at index {} in dataset {}", i, cfg.column_name);
        }
      }

      let iters = 1000;
      let raw_bytes = data.len() * 4;

      let start_enc = Instant::now();
      for _ in 0..iters {
        comp_buf.clear();
        compress_into(&data, &mut comp_buf);
      }
      let enc_dur = start_enc.elapsed();
      let enc_gb_s = (raw_bytes as f64 * iters as f64) / (enc_dur.as_secs_f64() * 1e9);

      let start_dec = Instant::now();
      for _ in 0..iters {
        dec_f32_buf.clear();
        let _ = decompress_into::<f32>(&comp_buf, &mut dec_f32_buf);
      }
      let dec_dur = start_dec.elapsed();
      let dec_gb_s = (raw_bytes as f64 * iters as f64) / (dec_dur.as_secs_f64() * 1e9);

      let fastalp_bits_per_val = (comp_buf.len() * 8) as f64 / data.len() as f64;
      let cpp_size_str = cpp_sizes
        .get(&cfg.column_name)
        .map(|s| format!("{:.2}", s))
        .unwrap_or_else(|| "N/A".to_string());

      println!(
        "{:<24} | {:>8} | {:>14} | {:>14.2} | {:>12.2} | {:>12.2}",
        cfg.column_name, cfg.data_type, cpp_size_str, fastalp_bits_per_val, enc_gb_s, dec_gb_s
      );
      tested_count += 1;
    }
  }

  println!("{:-<96}", "");
  println!("Completed benchmark across {} datasets.", tested_count);
}
