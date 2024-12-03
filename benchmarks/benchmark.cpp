#include "benchmark.hpp"
#include "bench_alp.hpp"

namespace alp_bench {

#include <cmath>
#include <gtest/gtest.h> // Assuming Google Test is being used

template <typename T>
void ALP_ASSERT(T original_val, T decoded_val, size_t idx) {
	if (original_val == 0.0 && std::signbit(original_val)) {
		if (!(decoded_val == 0.0 && std::signbit(decoded_val))) {
			std::cerr << "Assertion failed: decoded_val is not -0.0 as expected.\n";
			std::cerr << idx << " | original_val: " << original_val << ", decoded_val: " << decoded_val << "\n";
			std::terminate(); // Immediately stop the program
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
	//
	ofile << "idx,column,data_type,size,rowgroups_count,vectors_count,decompression_speed(cycles_per_value),"
	         "compression_speed(cycles_per_value),\n";
}

#include <cstdint> // For fixed-width integer types

template <typename PT>
BenchSpeedResult ALPBench::typed_bench_speed_column(const std::vector<PT>& data) {
	// Internal Type
	using UT = typename alp::inner_t<PT>::ut;
	using ST = typename alp::inner_t<PT>::st;

	BenchSpeedResult result {0, 0};

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
	PT*   glue_arr         = reinterpret_cast<PT*>(glue_buf);
	PT*   data_arr         = reinterpret_cast<PT*>(data_buf);

	for (size_t idx {0}; idx < VECTOR_SIZE; idx++) {
		data_arr[idx] = data[idx];
	}

// benchmark alp encode
#ifdef NDEBUG
	uint64_t iterations = 30000;
#else
	uint64_t iterations = 1;
#endif

	// init
	alp::state<PT> stt;
	alp::encoder<PT>::init(data.data(), 0, 1024, sample_arr, stt);

	switch (stt.scheme) {
	case alp::Scheme::ALP_RD: {
		alp::rd_encoder<PT>::init(data_arr, 0, 1024, sample_arr, stt);

		uint64_t cycles = benchmark::cycleclock::Now();
		for (uint64_t i = 0; i < iterations; ++i) {
			alp::rd_encoder<PT>::encode(data_arr, rd_exc_arr, pos_arr, exc_c_arr, right_arr, left_arr, stt);
			ffor::ffor(right_arr, ffor_right_arr, stt.right_bit_width, &stt.right_for_base);
			ffor::ffor(left_arr, ffor_left_arr, stt.left_bit_width, &stt.left_for_base);
		}
		cycles                   = benchmark::cycleclock::Now() - cycles;
		result.compression_speed = double(cycles) / (double(iterations) * VECTOR_SIZE);

		// Decode
		cycles = benchmark::cycleclock::Now();
		for (uint64_t i = 0; i < iterations; ++i) {
			unffor::unffor(ffor_right_arr, unffor_right_arr, stt.right_bit_width, &stt.right_for_base);
			unffor::unffor(ffor_left_arr, unffor_left_arr, stt.left_bit_width, &stt.left_for_base);
			alp::rd_encoder<PT>::decode(
			    glue_arr, unffor_right_arr, unffor_left_arr, rd_exc_arr, pos_arr, exc_c_arr, stt);
		}
		cycles                     = benchmark::cycleclock::Now() - cycles;
		result.decompression_speed = double(cycles) / (double(iterations) * VECTOR_SIZE);

		for (size_t j = 0; j < VECTOR_SIZE; ++j) {
			auto l = data_arr[j];
			auto r = glue_arr[j];
			ALP_ASSERT<PT>(l, r, j);
		}
	} break;
	case alp::Scheme::ALP: {

		stt.bit_width = 10;

		uint64_t cycles = benchmark::cycleclock::Now();
		for (uint64_t i = 0; i < iterations; ++i) {
			alp::encoder<PT>::encode(data_arr, exc_arr, pos_arr, exc_c_arr, encoded_arr, stt);
			alp::encoder<PT>::analyze_ffor(encoded_arr, stt.bit_width, base_arr);
			ffor::ffor(encoded_arr, ffor_arr, stt.bit_width, base_arr);
		}

		cycles                   = benchmark::cycleclock::Now() - cycles;
		result.compression_speed = double(cycles) / (double(iterations) * VECTOR_SIZE);

		cycles = benchmark::cycleclock::Now();

		for (uint64_t i = 0; i < iterations; ++i) {
			unffor::unffor(ffor_arr, unffor_arr, stt.bit_width, base_arr);
			alp::decoder<PT>::decode(unffor_arr, stt.fac, stt.exp, decoded_arr);
			alp::decoder<PT>::patch_exceptions(decoded_arr, exc_arr, pos_arr, exc_c_arr);
		}

		cycles                     = benchmark::cycleclock::Now() - cycles;
		result.decompression_speed = double(cycles) / (double(iterations) * VECTOR_SIZE);

		for (size_t idx = 0; idx < VECTOR_SIZE; idx++) {
			auto original_value = data.data()[idx];
			auto decoded_val    = decoded_arr[idx];
			ALP_ASSERT<PT>(original_value, decoded_val, idx);
		}
	} break;
	default:
		std::cerr << "validity test failed.";
	}

	return result;
}

template <typename PT>
void ALPBench::typed_bench_column(const ColumnDescriptor& column, std::ofstream& ofile) {
	// Internal Type
	using UT = typename alp::inner_t<PT>::ut;
	using ST = typename alp::inner_t<PT>::st;

	// init
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
	PT*   glue_arr         = reinterpret_cast<PT*>(glue_buf);

	std::fill_n(sample_arr, VECTOR_SIZE, 0);
	std::fill_n(glue_arr, VECTOR_SIZE, 1);

	std::cout << column.name << std::endl;

	// read data
	std::vector<PT> data;
	alp_data::read_data(data, column);
	PT*    data_column = data.data();
	size_t n_tuples    = data.size();

	benchmark::cycleclock::Init();
	bench_speed_result = typed_bench_speed_column<PT>(data);

	size_t n_vecs      = n_tuples / VECTOR_SIZE;
	auto   n_rowgroups = static_cast<size_t>(std::ceil(static_cast<double>(n_tuples) / ROWGROUP_SIZE));
	std::vector<VectorMetadata> compression_metadata;
	PT                          value_to_encode {0.0};
	size_t                      rowgroup_counter {0};
	alp::state<PT>              stt;

	/* Encode - Decode - Validate. */
	double compression_ratio {0};
	for (size_t rg_idx = 0; rg_idx < n_rowgroups; rg_idx++) {
		/* Init */
		PT* cur_rg_p = get_data(rg_idx, data_column);

		size_t n_vec_per_current_rg;

		if (n_rowgroups == 1) {
			// Single row group: all vectors belong to it
			n_vec_per_current_rg = n_vecs;
		} else if (rg_idx == n_rowgroups - 1) {
			// Last row group: remainder vectors
			n_vec_per_current_rg = n_vecs % N_VECTORS_PER_ROWGROUP;
		} else {
			// Regular row groups
			n_vec_per_current_rg = N_VECTORS_PER_ROWGROUP;
		}

		auto n_values_per_current_rg = n_vec_per_current_rg * VECTOR_SIZE;
		alp::encoder<PT>::init(cur_rg_p, rg_idx, n_values_per_current_rg, sample_arr, stt);

		switch (stt.scheme) {
		case alp::Scheme::ALP_RD: {
			alp::rd_encoder<PT>::init(cur_rg_p, 0, n_values_per_current_rg, sample_arr, stt);
			for (size_t vector_idx {0}; vector_idx < n_vec_per_current_rg; vector_idx++) {
				const PT* cur_vec_p = get_data(rg_idx, data_column, vector_idx);

				// Encode
				alp::rd_encoder<PT>::encode(cur_vec_p, rd_exc_arr, pos_arr, exc_c_arr, right_arr, left_arr, stt);
				ffor::ffor(right_arr, ffor_right_arr, stt.right_bit_width, &stt.right_for_base);
				ffor::ffor(left_arr, ffor_left_arr, stt.left_bit_width, &stt.left_for_base);

				// Decode
				unffor::unffor(ffor_right_arr, unffor_right_arr, stt.right_bit_width, &stt.right_for_base);
				unffor::unffor(ffor_left_arr, unffor_left_arr, stt.left_bit_width, &stt.left_for_base);
				alp::rd_encoder<PT>::decode(
				    glue_arr, unffor_right_arr, unffor_left_arr, rd_exc_arr, pos_arr, exc_c_arr, stt);

				auto* dbl_glue_arr = reinterpret_cast<PT*>(glue_arr);
				for (size_t j = 0; j < VECTOR_SIZE; ++j) {
					auto l = cur_vec_p[j];
					auto r = dbl_glue_arr[j];
					ALP_ASSERT<PT>(cur_vec_p[j], dbl_glue_arr[j], j);
				}

				VectorMetadata vector_metadata;
				vector_metadata.right_bit_width  = stt.right_bit_width;
				vector_metadata.left_bit_width   = stt.left_bit_width;
				vector_metadata.exceptions_count = stt.exceptions_count;
				vector_metadata.scheme           = alp::Scheme::ALP_RD;

				compression_metadata.push_back(vector_metadata);
			}
		} break;
		case alp::Scheme::ALP: {
			/* Encode - Decode - Validate. */
			for (size_t vector_idx {0}; vector_idx < n_vec_per_current_rg; vector_idx++) {
				const PT* data_p = get_data(rg_idx, data_column, vector_idx);

				alp::encoder<PT>::encode(data_p, exc_arr, pos_arr, exc_c_arr, encoded_arr, stt);
				alp::encoder<PT>::analyze_ffor(encoded_arr, stt.bit_width, base_arr);
				ffor::ffor(encoded_arr, ffor_arr, stt.bit_width, base_arr);

				unffor::unffor(ffor_arr, unffor_arr, stt.bit_width, base_arr);
				alp::decoder<PT>::decode(unffor_arr, stt.fac, stt.exp, decoded_arr);
				alp::decoder<PT>::patch_exceptions(decoded_arr, exc_arr, pos_arr, exc_c_arr);

				for (size_t j = 0; j < VECTOR_SIZE; j++) {
					auto l = data_p[j];
					auto r = decoded_arr[j];
					//					ALP_ASSERT<PT>(data_p[j], decoded_arr[j]);
				}

				VectorMetadata vector_metadata;
				vector_metadata.bit_width        = stt.bit_width;
				vector_metadata.exceptions_count = exc_c_arr[0];
				vector_metadata.scheme           = alp::Scheme::ALP;

				compression_metadata.push_back(vector_metadata);
				bit_width = 0;
			}

		} break;
		default:
			ASSERT_TRUE(false);
		}
	}

	compression_ratio = calculate_alp_compression_size<PT>(compression_metadata);
	ofile << std::fixed << std::setprecision(2)            //
	      << column.id << ","                              //
	      << column.name << ","                            //
	      << get_type_string<PT>() << ","                  //
	      << compression_ratio << ","                      //
	      << n_rowgroups << ","                            //
	      << n_vecs << ","                                 //
	      << bench_speed_result.decompression_speed << "," //
	      << bench_speed_result.compression_speed << std::endl;
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