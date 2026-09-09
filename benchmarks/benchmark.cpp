#include "benchmark.hpp"
#include "bench_alp.hpp"
#include "fastalp.h"

#include <cmath>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>
#include <gtest/gtest.h>

namespace alp_bench {

template <typename T>
void ALP_ASSERT(T original_val, T decoded_val, size_t idx) {
	if (original_val == 0.0 && std::signbit(original_val)) {
		if (!(decoded_val == 0.0 && std::signbit(decoded_val))) {
			std::cerr << "Assertion failed: decoded_val is not -0.0 as expected.\n";
			std::cerr << idx << " | original_val: " << original_val << ", decoded_val: " << decoded_val << "\n";
			std::terminate();
		}
	} else if (std::isnan(original_val)) {
		if (!std::isnan(decoded_val)) {
			std::cerr << "Assertion failed: decoded_val is not NaN as expected.\n";
			std::cerr << idx << " | original_val: " << original_val << ", decoded_val: " << decoded_val << "\n";
			std::terminate();
		}
	} else {
		if (original_val != decoded_val) {
			std::cerr << "Assertion failed: original_val != decoded_val.\n";
			std::cerr << idx << "| original_val: " << original_val << ", decoded_val: " << decoded_val << "\n";
			std::terminate();
		}
	}
}

void write_result_header(std::ofstream& ofile) {
	ofile << "idx,column,data_type,"
	         "cpp_size(bits/val),fastalp_size(bits/val),"
	         "cpp_enc_sampled(GB/s),fastalp_enc_sampled(GB/s),"
	         "cpp_enc_kernel(GB/s),fastalp_enc_kernel(GB/s),"
	         "cpp_dec(GB/s),fastalp_dec(GB/s)\n";
}

template <typename PT>
BenchSpeedResult ALPBench::typed_bench_speed_column(const std::vector<PT>& data) {
	using UT = typename alp::inner_t<PT>::ut;
	using ST = typename alp::inner_t<PT>::st;

	BenchSpeedResult result {};

	PT*   sample_arr       = reinterpret_cast<PT*>(sample_buf);
	PT*   exc_arr          = reinterpret_cast<PT*>(exc_buf);
	auto* rd_exc_arr       = reinterpret_cast<uint16_t*>(rd_exc_buf);
	auto* pos_arr          = reinterpret_cast<uint16_t*>(pos_buf);
	auto* exc_c_arr        = reinterpret_cast<uint16_t*>(exc_c_buf);
	ST*   ffor_arr         = reinterpret_cast<ST*>(ffor_buf);
	ST*   unffor_arr       = reinterpret_cast<ST*>(unffor_buf);
	ST*   base_arr         = reinterpret_cast<ST*>(base_buf);
	ST*   encoded_arr      = reinterpret_cast<ST*>(encoded_buf);
	PT*   decoded_arr      = reinterpret_cast<PT*>(decoded_buf);
	UT*   ffor_right_arr   = reinterpret_cast<UT*>(ffor_right_buf);
	auto* ffor_left_arr    = reinterpret_cast<uint16_t*>(ffor_left_buf);
	UT*   right_arr        = reinterpret_cast<UT*>(right_buf);
	auto* left_arr         = reinterpret_cast<uint16_t*>(left_buf);
	UT*   unffor_right_arr = reinterpret_cast<UT*>(unffor_right_buf);
	auto* unffor_left_arr  = reinterpret_cast<uint16_t*>(unffor_left_buf);
	auto* glue_arr         = reinterpret_cast<PT*>(glue_buf);
	const PT* data_arr     = data.data();

#ifdef NDEBUG
	uint64_t iterations = 1000;
#else
	uint64_t iterations = 10;
#endif

	// -------------------------------------------------------------
	// 1. C++ ALP Benchmark
	// -------------------------------------------------------------
	alp::state<PT> stt;
	alp::encoder<PT>::init(data.data(), 0, 1024, sample_arr, stt);

	switch (stt.scheme) {
	case alp::Scheme::ALP_RD: {
		// C++ Sampled (includes init / dictionary selection)
		auto t0_enc_samp = std::chrono::high_resolution_clock::now();
		for (uint64_t i = 0; i < iterations; ++i) {
			alp::rd_encoder<PT>::init(data_arr, 0, 1024, sample_arr, stt);
			alp::rd_encoder<PT>::encode(data_arr, rd_exc_arr, pos_arr, exc_c_arr, right_arr, left_arr, stt);
			ffor::ffor(right_arr, ffor_right_arr, stt.right_bit_width, &stt.right_for_base);
			ffor::ffor(left_arr, ffor_left_arr, stt.left_bit_width, &stt.left_for_base);
		}
		auto t1_enc_samp = std::chrono::high_resolution_clock::now();
		double dt_enc_samp = std::chrono::duration<double, std::nano>(t1_enc_samp - t0_enc_samp).count();
		result.cpp_enc_sampled = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_enc_samp;

		// C++ Kernel (pure encoding without sampling)
		auto t0_enc_kern = std::chrono::high_resolution_clock::now();
		for (uint64_t i = 0; i < iterations; ++i) {
			alp::rd_encoder<PT>::encode(data_arr, rd_exc_arr, pos_arr, exc_c_arr, right_arr, left_arr, stt);
			ffor::ffor(right_arr, ffor_right_arr, stt.right_bit_width, &stt.right_for_base);
			ffor::ffor(left_arr, ffor_left_arr, stt.left_bit_width, &stt.left_for_base);
		}
		auto t1_enc_kern = std::chrono::high_resolution_clock::now();
		double dt_enc_kern = std::chrono::duration<double, std::nano>(t1_enc_kern - t0_enc_kern).count();
		result.cpp_enc_kernel = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_enc_kern;

		// C++ Decompression
		auto t0_dec = std::chrono::high_resolution_clock::now();
		for (uint64_t i = 0; i < iterations; ++i) {
			unffor::unffor(ffor_right_arr, unffor_right_arr, stt.right_bit_width, &stt.right_for_base);
			unffor::unffor(ffor_left_arr, unffor_left_arr, stt.left_bit_width, &stt.left_for_base);
			alp::rd_encoder<PT>::decode(
			    glue_arr, unffor_right_arr, unffor_left_arr, rd_exc_arr, pos_arr, exc_c_arr, stt);
		}
		auto t1_dec = std::chrono::high_resolution_clock::now();
		double dt_dec = std::chrono::duration<double, std::nano>(t1_dec - t0_dec).count();
		result.cpp_dec = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_dec;

		for (size_t j = 0; j < VECTOR_SIZE; ++j) {
			ALP_ASSERT<PT>(data_arr[j], glue_arr[j], j);
		}
		break;
	}
	case alp::Scheme::ALP:
	default: {
		stt.bit_width = 10;

		// C++ Sampled (includes init / parameter search)
		auto t0_enc_samp = std::chrono::high_resolution_clock::now();
		for (uint64_t i = 0; i < iterations; ++i) {
			alp::encoder<PT>::init(data.data(), 0, 1024, sample_arr, stt);
			alp::encoder<PT>::encode(data_arr, exc_arr, pos_arr, exc_c_arr, encoded_arr, stt);
			alp::encoder<PT>::analyze_ffor(encoded_arr, stt.bit_width, base_arr);
			ffor::ffor(encoded_arr, ffor_arr, stt.bit_width, base_arr);
		}
		auto t1_enc_samp = std::chrono::high_resolution_clock::now();
		double dt_enc_samp = std::chrono::duration<double, std::nano>(t1_enc_samp - t0_enc_samp).count();
		result.cpp_enc_sampled = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_enc_samp;

		// C++ Kernel (pure encoding without sampling)
		auto t0_enc_kern = std::chrono::high_resolution_clock::now();
		for (uint64_t i = 0; i < iterations; ++i) {
			alp::encoder<PT>::encode(data_arr, exc_arr, pos_arr, exc_c_arr, encoded_arr, stt);
			alp::encoder<PT>::analyze_ffor(encoded_arr, stt.bit_width, base_arr);
			ffor::ffor(encoded_arr, ffor_arr, stt.bit_width, base_arr);
		}
		auto t1_enc_kern = std::chrono::high_resolution_clock::now();
		double dt_enc_kern = std::chrono::duration<double, std::nano>(t1_enc_kern - t0_enc_kern).count();
		result.cpp_enc_kernel = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_enc_kern;

		// C++ Decompression
		auto t0_dec = std::chrono::high_resolution_clock::now();
		for (uint64_t i = 0; i < iterations; ++i) {
			unffor::unffor(ffor_arr, unffor_arr, stt.bit_width, base_arr);
			alp::decoder<PT>::decode(unffor_arr, stt.fac, stt.exp, decoded_arr);
			alp::decoder<PT>::patch_exceptions(decoded_arr, exc_arr, pos_arr, exc_c_arr);
		}
		auto t1_dec = std::chrono::high_resolution_clock::now();
		double dt_dec = std::chrono::duration<double, std::nano>(t1_dec - t0_dec).count();
		result.cpp_dec = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_dec;

		for (size_t idx = 0; idx < VECTOR_SIZE; idx++) {
			ALP_ASSERT<PT>(data.data()[idx], decoded_arr[idx], idx);
		}
		break;
	}
	}

	// -------------------------------------------------------------
	// 2. fastalp (Rust) Benchmark
	// -------------------------------------------------------------
	std::vector<uint8_t> fastalp_comp_buf(65536);
	size_t fa_written = 0;

	// fastalp Sampled (end-to-end with dynamic sampling)
	auto t0_fa_samp = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < iterations; ++i) {
		if constexpr (std::is_same_v<PT, double>) {
			fa_written = fastalp_compress_f64(data.data(), data.size(), fastalp_comp_buf.data(), fastalp_comp_buf.size());
		} else {
			fa_written = fastalp_compress_f32(data.data(), data.size(), fastalp_comp_buf.data(), fastalp_comp_buf.size());
		}
	}
	auto t1_fa_samp = std::chrono::high_resolution_clock::now();
	double dt_fa_samp = std::chrono::duration<double, std::nano>(t1_fa_samp - t0_fa_samp).count();
	result.fastalp_enc_sampled = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_fa_samp;
	result.fastalp_size = (fa_written * 8.0) / data.size();

	// fastalp Kernel (pure encoding without sampling - reusing cached model)
	if constexpr (std::is_same_v<PT, double>) {
		fastalp_reset_encoder_f64();
		fastalp_compress_cached_f64(data.data(), data.size(), fastalp_comp_buf.data(), fastalp_comp_buf.size());
	} else {
		fastalp_reset_encoder_f32();
		fastalp_compress_cached_f32(data.data(), data.size(), fastalp_comp_buf.data(), fastalp_comp_buf.size());
	}
	auto t0_fa_kern = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < iterations; ++i) {
		if constexpr (std::is_same_v<PT, double>) {
			fastalp_compress_cached_f64(data.data(), data.size(), fastalp_comp_buf.data(), fastalp_comp_buf.size());
		} else {
			fastalp_compress_cached_f32(data.data(), data.size(), fastalp_comp_buf.data(), fastalp_comp_buf.size());
		}
	}
	auto t1_fa_kern = std::chrono::high_resolution_clock::now();
	double dt_fa_kern = std::chrono::duration<double, std::nano>(t1_fa_kern - t0_fa_kern).count();
	result.fastalp_enc_kernel = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_fa_kern;

	// fastalp Decompression
	std::vector<PT> fastalp_dec_buf(data.size());
	auto t0_fa_dec = std::chrono::high_resolution_clock::now();
	for (uint64_t i = 0; i < iterations; ++i) {
		if constexpr (std::is_same_v<PT, double>) {
			fastalp_decompress_f64(fastalp_comp_buf.data(), fa_written, fastalp_dec_buf.data(), fastalp_dec_buf.size());
		} else {
			fastalp_decompress_f32(fastalp_comp_buf.data(), fa_written, fastalp_dec_buf.data(), fastalp_dec_buf.size());
		}
	}
	auto t1_fa_dec = std::chrono::high_resolution_clock::now();
	double dt_fa_dec = std::chrono::duration<double, std::nano>(t1_fa_dec - t0_fa_dec).count();
	result.fastalp_dec = (double(iterations) * VECTOR_SIZE * sizeof(PT)) / dt_fa_dec;

	// Lossless verification for fastalp
	for (size_t idx = 0; idx < data.size(); ++idx) {
		ALP_ASSERT<PT>(data[idx], fastalp_dec_buf[idx], idx);
	}

	return result;
}

template <typename PT>
void ALPBench::typed_bench_column(const ColumnDescriptor& column, std::ofstream& ofile) {
	using UT = typename alp::inner_t<PT>::ut;
	using ST = typename alp::inner_t<PT>::st;

	BenchSpeedResult bench_speed_result;

	PT*   sample_arr       = reinterpret_cast<PT*>(sample_buf);
	PT*   exc_arr          = reinterpret_cast<PT*>(exc_buf);
	auto* rd_exc_arr       = reinterpret_cast<uint16_t*>(rd_exc_buf);
	auto* pos_arr          = reinterpret_cast<uint16_t*>(pos_buf);
	auto* exc_c_arr        = reinterpret_cast<uint16_t*>(exc_c_buf);
	ST*   ffor_arr         = reinterpret_cast<ST*>(ffor_buf);
	ST*   unffor_arr       = reinterpret_cast<ST*>(unffor_buf);
	ST*   base_arr         = reinterpret_cast<ST*>(base_buf);
	ST*   encoded_arr      = reinterpret_cast<ST*>(encoded_buf);
	PT*   decoded_arr      = reinterpret_cast<PT*>(decoded_buf);
	UT*   ffor_right_arr   = reinterpret_cast<UT*>(ffor_right_buf);
	auto* ffor_left_arr    = reinterpret_cast<uint16_t*>(ffor_left_buf);
	UT*   right_arr        = reinterpret_cast<UT*>(right_buf);
	auto* left_arr         = reinterpret_cast<uint16_t*>(left_buf);
	UT*   unffor_right_arr = reinterpret_cast<UT*>(unffor_right_buf);
	auto* unffor_left_arr  = reinterpret_cast<uint16_t*>(unffor_left_buf);
	auto* glue_arr         = reinterpret_cast<PT*>(glue_buf);

	auto  data_column = reinterpret_cast<PT*>(data_buf);
	std::vector<PT> data;
	alp_data::read_data<PT>(data, column);

	size_t n_tuples = data.size();
	std::copy(data.begin(), data.end(), data_column);

	bench_speed_result = typed_bench_speed_column<PT>(data);

	size_t n_vecs      = n_tuples / VECTOR_SIZE;
	auto   n_rowgroups = static_cast<size_t>(std::ceil(static_cast<double>(n_tuples) / ROWGROUP_SIZE));
	std::vector<VectorMetadata> compression_metadata;
	alp::state<PT>              stt;

	double compression_ratio {0};
	for (size_t rg_idx = 0; rg_idx < n_rowgroups; rg_idx++) {
		PT* cur_rg_p = get_data(rg_idx, data_column);
		size_t n_vec_per_current_rg = (n_rowgroups == 1) ? n_vecs : ((rg_idx == n_rowgroups - 1) ? n_vecs % N_VECTORS_PER_ROWGROUP : N_VECTORS_PER_ROWGROUP);
		auto n_values_per_current_rg = n_vec_per_current_rg * VECTOR_SIZE;
		alp::encoder<PT>::init(cur_rg_p, rg_idx, n_values_per_current_rg, sample_arr, stt);

		switch (stt.scheme) {
		case alp::Scheme::ALP_RD: {
			alp::rd_encoder<PT>::init(cur_rg_p, 0, n_values_per_current_rg, sample_arr, stt);
			for (size_t vector_idx {0}; vector_idx < n_vec_per_current_rg; vector_idx++) {
				const PT* cur_vec_p = get_data(rg_idx, data_column, vector_idx);
				alp::rd_encoder<PT>::encode(cur_vec_p, rd_exc_arr, pos_arr, exc_c_arr, right_arr, left_arr, stt);
				ffor::ffor(right_arr, ffor_right_arr, stt.right_bit_width, &stt.right_for_base);
				ffor::ffor(left_arr, ffor_left_arr, stt.left_bit_width, &stt.left_for_base);

				unffor::unffor(ffor_right_arr, unffor_right_arr, stt.right_bit_width, &stt.right_for_base);
				unffor::unffor(ffor_left_arr, unffor_left_arr, stt.left_bit_width, &stt.left_for_base);
				alp::rd_encoder<PT>::decode(
				    glue_arr, unffor_right_arr, unffor_left_arr, rd_exc_arr, pos_arr, exc_c_arr, stt);

				VectorMetadata vector_metadata;
				vector_metadata.right_bit_width  = stt.right_bit_width;
				vector_metadata.left_bit_width   = stt.left_bit_width;
				vector_metadata.exceptions_count = stt.exceptions_count;
				vector_metadata.scheme           = alp::Scheme::ALP_RD;
				compression_metadata.push_back(vector_metadata);
			}
		} break;
		case alp::Scheme::ALP: {
			for (size_t vector_idx {0}; vector_idx < n_vec_per_current_rg; vector_idx++) {
				const PT* data_p = get_data(rg_idx, data_column, vector_idx);
				alp::encoder<PT>::encode(data_p, exc_arr, pos_arr, exc_c_arr, encoded_arr, stt);
				alp::encoder<PT>::analyze_ffor(encoded_arr, stt.bit_width, base_arr);
				ffor::ffor(encoded_arr, ffor_arr, stt.bit_width, base_arr);

				unffor::unffor(ffor_arr, unffor_arr, stt.bit_width, base_arr);
				alp::decoder<PT>::decode(unffor_arr, stt.fac, stt.exp, decoded_arr);
				alp::decoder<PT>::patch_exceptions(decoded_arr, exc_arr, pos_arr, exc_c_arr);

				VectorMetadata vector_metadata;
				vector_metadata.bit_width        = stt.bit_width;
				vector_metadata.exceptions_count = exc_c_arr[0];
				vector_metadata.scheme           = alp::Scheme::ALP;
				compression_metadata.push_back(vector_metadata);
			}
		} break;
		default:
			ASSERT_TRUE(false);
		}
	}

	compression_ratio = calculate_alp_compression_size<PT>(compression_metadata);
	bench_speed_result.cpp_size = compression_ratio;

	ofile << std::fixed << std::setprecision(2)
	      << column.id << ","
	      << column.name << ","
	      << get_type_string<PT>() << ","
	      << bench_speed_result.cpp_size << ","
	      << bench_speed_result.fastalp_size << ","
	      << bench_speed_result.cpp_enc_sampled << ","
	      << bench_speed_result.fastalp_enc_sampled << ","
	      << bench_speed_result.cpp_enc_kernel << ","
	      << bench_speed_result.fastalp_enc_kernel << ","
	      << bench_speed_result.cpp_dec << ","
	      << bench_speed_result.fastalp_dec << std::endl;

	std::cout << std::left << std::setw(23) << column.name << " | "
	          << "Size: C++ " << std::setw(5) << bench_speed_result.cpp_size << " / fa " << std::setw(5) << bench_speed_result.fastalp_size << " b/v | "
	          << "Enc(samp): " << std::setw(5) << bench_speed_result.cpp_enc_sampled << " / " << std::setw(5) << bench_speed_result.fastalp_enc_sampled << " GB/s | "
	          << "Enc(kern): " << std::setw(5) << bench_speed_result.cpp_enc_kernel << " / " << std::setw(5) << bench_speed_result.fastalp_enc_kernel << " GB/s | "
	          << "Dec: " << std::setw(5) << bench_speed_result.cpp_dec << " / " << std::setw(5) << bench_speed_result.fastalp_dec << " GB/s" << std::endl;
}

template void ALPBench::typed_bench_column<double>(const ColumnDescriptor& column, std::ofstream& ofile);
template void ALPBench::typed_bench_column<float>(const ColumnDescriptor& column, std::ofstream& ofile);

template <typename PT, size_t N_COLS>
void ALPBench::typed_bench_dataset(std::array<ALPColumnDescriptor, N_COLS> columns,
                                   const std::string&                      result_file_path) {
	std::ofstream ofile(result_file_path, std::ios::out);
	write_result_header(ofile);

	for (auto& alp_column_descriptor : columns) {
		auto column_descriptor = extract_column_descriptor<PT>(alp_column_descriptor);
		typed_bench_column<PT>(column_descriptor, ofile);
	}
}

template void ALPBench::typed_bench_dataset<double, 2ul>(std::array<ALPColumnDescriptor, 2> columns,
                                                         const std::string&                 result_file_path);
template void ALPBench::typed_bench_dataset<double, 30ul>(std::array<ALPColumnDescriptor, 30> columns,
                                                          const std::string&                  result_file_path);
template void ALPBench::typed_bench_dataset<float, 4ul>(std::array<ALPColumnDescriptor, 4> columns,
                                                        const std::string&                 result_file_path);
template void ALPBench::typed_bench_dataset<float, 20ul>(std::array<ALPColumnDescriptor, 20> columns,
                                                         const std::string&                  result_file_path);
} // namespace alp_bench
