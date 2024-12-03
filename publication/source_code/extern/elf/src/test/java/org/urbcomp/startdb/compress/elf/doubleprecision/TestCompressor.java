package org.urbcomp.startdb.compress.elf.doubleprecision;

import org.junit.jupiter.api.Test;
import org.urbcomp.startdb.compress.elf.compressor.*;
import org.urbcomp.startdb.compress.elf.decompressor.*;

import java.io.*;
import java.nio.ByteBuffer;
import java.util.*;

import oshi.SystemInfo;
import oshi.hardware.CentralProcessor;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertEquals;

public class TestCompressor {
    private static final String[] FILENAMES = {
            "neon_air_pressure.bin",
            "arade4.bin",
            "basel_temp_f.bin",
            "basel_wind_f.bin",
            "bird_migration_f.bin",
            "bitcoin_f.bin",
            "bitcoin_transactions_f.bin",
            "city_temperature_f.bin",
            "cms1.bin",
            "cms9.bin",
            "cms25.bin",
            "neon_dew_point_temp.bin",
            "neon_bio_temp_c.bin",
            "food_prices.bin",
            "gov10.bin",
            "gov26.bin",
            "gov30.bin",
            "gov31.bin",
            "gov40.bin",
            "medicare1.bin",
            "medicare9.bin",
            "neon_pm10_dust.bin",
            "nyc29.bin",
            "poi_lat.bin",
            "poi_lon.bin",
            "ssd_hdd_benchmarks_f.bin",
            "stocks_de.bin",
            "stocks_uk.bin",
            "stocks_usa_c.bin",
            "neon_wind_dir.bin"
    };
    private static final double TIME_PRECISION = 1000000.0;  // Nano to Miliseconds!
    List<Map<String, ResultStructure>> allResult = new ArrayList<>();
    List<Map<String, ResultStructure>> crResult = new ArrayList<>();

    @Test
    public void testCompressor() throws IOException {
        for (String filename : FILENAMES) {
            Map<String, List<ResultStructure>> result = new HashMap<>();
            testELFSpeed(filename, result);
            for (Map.Entry<String, List<ResultStructure>> kv : result.entrySet()) {
                Map<String, ResultStructure> r = new HashMap<>();
                r.put(kv.getKey(), computeAvg(kv.getValue()));
                allResult.add(r);
            }
        }
        storeResult("../../../results/i4i_4xlarge/elf_raw.csv");

        for (String filename : FILENAMES) {
            Map<String, List<ResultStructure>> result = new HashMap<>();
            testELFCompressor(filename, result);
            for (Map.Entry<String, List<ResultStructure>> kv : result.entrySet()) {
                Map<String, ResultStructure> r = new HashMap<>();
                r.put(kv.getKey(), computeAvg(kv.getValue()));
                crResult.add(r);
            }
        }
        storeCompressionRatioResult("../../../compression_ratio_result/double/elf.csv");

    }

    public void testELFCompressor(String fileName, Map<String, List<ResultStructure>> resultCompressor) throws IOException {
        String datasets_dir_path = System.getenv("ALP_DATASET_DIR_PATH");
        System.out.println("ELF Compression Ratios: " + datasets_dir_path + fileName);
        FileReader fileReader = new FileReader(datasets_dir_path + fileName);

        float totalBlocks = 0;
        double[] values;

        HashMap<String, List<Double>> totalCompressionTime = new HashMap<>();
        HashMap<String, List<Double>> totalDecompressionTime = new HashMap<>();
        HashMap<String, Long> key2TotalSize = new HashMap<>();
        
        while ((values = fileReader.nextBlock()) != null) {
            totalBlocks += 1;
            ICompressor compressor = new ElfCompressor();

            double encodingDuration;
            double decodingDuration;
            long start = System.nanoTime();
            for (double value : values) {
                compressor.addValue(value);
            }
            compressor.close();

            encodingDuration = System.nanoTime() - start;

            start = System.nanoTime();
            // NO DECOMPRESSION NEEDED HERE
            decodingDuration = System.nanoTime() - start;

            String key = compressor.getKey();
            if (!totalCompressionTime.containsKey(key)) {
                totalCompressionTime.put(key, new ArrayList<>());
                totalDecompressionTime.put(key, new ArrayList<>());
                key2TotalSize.put(key, 0L);
            }
            totalCompressionTime.get(key).add(encodingDuration / TIME_PRECISION);
            totalDecompressionTime.get(key).add(decodingDuration / TIME_PRECISION);
            key2TotalSize.put(key, compressor.getSize() + key2TotalSize.get(key));
        }

        for (Map.Entry<String, Long> kv: key2TotalSize.entrySet()) {
            String key = kv.getKey();
            Long totalSize = kv.getValue();
            ResultStructure r = new ResultStructure(fileName, key,
                            totalSize / (totalBlocks * FileReader.DEFAULT_BLOCK_SIZE * 64.0),
                            totalCompressionTime.get(key),
                            totalDecompressionTime.get(key)
            );
            if (!resultCompressor.containsKey(key)) {
                resultCompressor.put(key, new ArrayList<>());
            }
            resultCompressor.get(key).add(r);
        }
    }

    public void testELFSpeed(String fileName, Map<String, List<ResultStructure>> resultCompressor) throws IOException {
        String datasets_dir_path = System.getenv("ALP_DATASET_DIR_PATH");
        System.out.println("ELF Speed Micro-benchmark: " + datasets_dir_path + fileName);
        FileReader fileReader = new FileReader(datasets_dir_path + fileName);

        float totalBlocks = 0;
        double[] values;

        HashMap<String, List<Double>> totalCompressionTime = new HashMap<>();
        HashMap<String, List<Double>> totalDecompressionTime = new HashMap<>();
        HashMap<String, Long> key2TotalSize = new HashMap<>();

        String key = "ElfSpeedMicroBenchmark";

        values = fileReader.nextBlock();

        int repetition = 30000;
        ICompressor compressor = new ElfCompressor();
        for (int i = 0; i < repetition; i++) {
            totalBlocks += 1;
            compressor = new ElfCompressor();
            double encodingDuration;
            long start = System.nanoTime();
            for (double value : values) {
                compressor.addValue(value);
            }
            compressor.close();
            encodingDuration = System.nanoTime() - start;

            if (!totalCompressionTime.containsKey(key)) {
                totalCompressionTime.put(key, new ArrayList<>());
            }
            totalCompressionTime.get(key).add(encodingDuration / TIME_PRECISION);
        }

        byte[] result = compressor.getBytes();
        IDecompressor decompressor;

        for (int i = 0; i < repetition; i++) {
            totalBlocks += 1;
            double decodingDuration;
            decompressor = new ElfDecompressor(result);

            long start = System.nanoTime();
            List<Double> uncompressedValues = decompressor.decompress();
            decodingDuration = System.nanoTime() - start;


            if (!totalDecompressionTime.containsKey(key)) {
                totalDecompressionTime.put(key, new ArrayList<>());
                key2TotalSize.put(key, 0L);
            }
            totalDecompressionTime.get(key).add(decodingDuration / TIME_PRECISION);
            key2TotalSize.put(key, compressor.getSize() + key2TotalSize.get(key));
        }

        for (Map.Entry<String, Long> kv: key2TotalSize.entrySet()) {
            String inner_key = kv.getKey();
            Long totalSize = kv.getValue();
            ResultStructure r = new ResultStructure(fileName, inner_key,
                    totalSize / (totalBlocks * FileReader.DEFAULT_BLOCK_SIZE * 64.0),
                    totalCompressionTime.get(inner_key),
                    totalDecompressionTime.get(inner_key)
            );
            if (!resultCompressor.containsKey(inner_key)) {
                resultCompressor.put(inner_key, new ArrayList<>());
            }
            resultCompressor.get(inner_key).add(r);
        }
    }


    public void storeResult(String filePath) throws IOException {
        File file = new File(filePath).getParentFile();
        if (!file.exists() && !file.mkdirs()) {
            throw new IOException("Create directory failed: " + file);
        }
        try (FileWriter fileWriter = new FileWriter(filePath)) {
            fileWriter.write(ResultStructure.getHead());
            for (Map<String, ResultStructure> result : allResult) {
                for (ResultStructure ls : result.values()) {
                    fileWriter.write(ls.toString());
                }
            }
        }
    }

    public void storeCompressionRatioResult(String filePath) throws IOException {
        File file = new File(filePath).getParentFile();
        if (!file.exists() && !file.mkdirs()) {
            throw new IOException("Create directory failed: " + file);
        }
        try (FileWriter fileWriter = new FileWriter(filePath)) {
            fileWriter.write(ResultStructure.getCRHead());
            for (Map<String, ResultStructure> result : crResult) {
                for (ResultStructure ls : result.values()) {
                    fileWriter.write(ls.toCRString());
                }
            }
        }
    }

    public ResultStructure computeAvg(List<ResultStructure> lr) {
        int num = lr.size();
        double compressionTime = 0;
        double maxCompressTime = 0;
        double minCompressTime = 0;
        double mediaCompressTime = 0;
        double decompressionTime = 0;
        double maxDecompressTime = 0;
        double minDecompressTime = 0;
        double mediaDecompressTime = 0;
        for (ResultStructure resultStructure : lr) {
            compressionTime += resultStructure.getCompressionTime();
            maxCompressTime += resultStructure.getMaxCompressTime();
            minCompressTime += resultStructure.getMinCompressTime();
            mediaCompressTime += resultStructure.getMediaCompressTime();
            decompressionTime += resultStructure.getDecompressionTime();
            maxDecompressTime += resultStructure.getMaxDecompressTime();
            minDecompressTime += resultStructure.getMinDecompressTime();
            mediaDecompressTime += resultStructure.getMediaDecompressTime();
        }
        return new ResultStructure(lr.get(0).getFilename(),
                lr.get(0).getCompressorName(),
                lr.get(0).getCompressorRatio(),
                computeTuplesPerCycle(compressionTime / num, FileReader.DEFAULT_BLOCK_SIZE),
                maxCompressTime / num,
                minCompressTime / num,
                mediaCompressTime / num,
                computeTuplesPerCycle(decompressionTime / num, FileReader.DEFAULT_BLOCK_SIZE),
                maxDecompressTime / num,
                minDecompressTime / num,
                mediaDecompressTime / num
        );
    }

    // Generated with ChatGPT
    private static double getCpuFrequencyGHz() {
        SystemInfo si = new SystemInfo();
        CentralProcessor processor = si.getHardware().getProcessor();
        long frequencyHz = processor.getProcessorIdentifier().getVendorFreq(); // Returns frequency in Hz
        return frequencyHz / 1_000_000_000.0; // Convert Hz to GHz
    }

    // Generated with ChatGPT
    public static double computeTuplesPerCycle(double runtimeMs, long tuples) {
        // Convert runtime from ms to seconds and compute total CPU cycles
        double cpuFreqGHz = getCpuFrequencyGHz();
        double runtimeSeconds = runtimeMs / 1000.0;
        double totalCpuCycles = cpuFreqGHz * 1_000_000_000 * runtimeSeconds;

        // Compute and return tuples per cycle
        return tuples / totalCpuCycles;
    }

    public static double[] toDoubleArray(byte[] byteArray) {
        int times = Double.SIZE / Byte.SIZE;
        double[] doubles = new double[byteArray.length / times];
        for (int i = 0; i < doubles.length; i++) {
            doubles[i] = ByteBuffer.wrap(byteArray, i * times, times).getDouble();
        }
        return doubles;
    }
}
