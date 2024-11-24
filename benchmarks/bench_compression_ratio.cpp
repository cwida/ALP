#include "alp.hpp"
#include "alp_result.hpp"
#include "data.hpp"
#include "test/mapper.hpp"
#include "gtest/gtest.h"
#include <unordered_map>

// ALP overhead per vector : bit_width + factor-idx + exponent-idx + ffor base;
template <typename PT>
PT get_overhead_per_vector() {
	return static_cast<PT>(8 +               // bit_width
	                       8 +               // factor-idx
	                       8 +               // exponent-idx
	                       (sizeof(PT) * 8)) // ffor base
	       / alp::config::VECTOR_SIZE;
};

std::string get_alp_scheme_string(alp::Scheme& scheme) {
	switch (scheme) {
	case alp::Scheme::ALP:
		return "ALP_PDE";
	case alp::Scheme::ALP_RD:
		return "ALP_RD";
	default:
		return "INVALID";
	}
}

class ALPBench : public ::testing::Test {
public:
	~ALPBench() override = default;

	template <typename PT, size_t N_COLS>
	void bench_alp_compression_ratio(std::array<alp_bench::Column, N_COLS> columns,
	                                 const std::string&                    result_file_path) {

		// Internal Type
		using UT = typename alp::inner_t<PT>::ut;
		using ST = typename alp::inner_t<PT>::st;

		// init
		PT*       sample_buf {};
		PT*       intput_buf {};
		PT*       exc_arr {};
		uint16_t* rd_exc_arr {};
		uint16_t* pos_arr {};
		uint16_t* exc_c_arr {};
		ST*       ffor_buf {};
		ST*       unffor_arr {};
		ST*       base_buf {};
		ST*       encoded_buf {};
		PT*       decoded_buf {};
		UT*       ffor_right_buf {};
		uint16_t* ffor_left_arr {};
		UT*       right_buf {};
		uint16_t* left_arr {};
		UT*       unffor_right_buf {};
		uint16_t* unffor_left_arr {};
		PT*       glue_buf {};
		uint8_t   bit_width {};

		// allocate
		intput_buf       = new PT[alp::config::VECTOR_SIZE];
		exc_arr          = new PT[alp::config::VECTOR_SIZE];
		rd_exc_arr       = new uint16_t[alp::config::VECTOR_SIZE];
		pos_arr          = new uint16_t[alp::config::VECTOR_SIZE];
		encoded_buf      = new ST[alp::config::VECTOR_SIZE];
		decoded_buf      = new PT[alp::config::VECTOR_SIZE];
		exc_c_arr        = new uint16_t[alp::config::VECTOR_SIZE];
		ffor_buf         = new ST[alp::config::VECTOR_SIZE];
		unffor_arr       = new ST[alp::config::VECTOR_SIZE];
		base_buf         = new ST[alp::config::VECTOR_SIZE];
		sample_buf       = new PT[alp::config::VECTOR_SIZE];
		right_buf        = new UT[alp::config::VECTOR_SIZE];
		left_arr         = new uint16_t[alp::config::VECTOR_SIZE];
		ffor_right_buf   = new UT[alp::config::VECTOR_SIZE];
		ffor_left_arr    = new uint16_t[alp::config::VECTOR_SIZE];
		unffor_right_buf = new UT[alp::config::VECTOR_SIZE];
		unffor_left_arr  = new uint16_t[alp::config::VECTOR_SIZE];
		glue_buf         = new PT[alp::config::VECTOR_SIZE];

		std::ofstream ofile(result_file_path, std::ios::out);
		ofile << "idx,dataset,size,alp_scheme,rowgroups_count,vectors_count\n";

		for (auto& dataset : columns) {
			std::cout << dataset.name << std::endl;

			std::vector<PT> data;
			alp_data::read_data(data, dataset.csv_file_path, dataset.binary_file_path);
			PT*    data_column = data.data();
			size_t n_tuples    = data.size();

			std::vector<alp_bench::VectorMetadata> compression_metadata;
			PT                                     value_to_encode {0.0};
			size_t                                 vector_idx {0};
			size_t                                 rowgroup_counter {0};
			size_t                                 rowgroup_offset {0};
			alp::state<PT>                         stt;
			size_t rowgroups_count = std::ceil(static_cast<double>(n_tuples) / alp::config::ROWGROUP_SIZE);
			size_t vectors_count   = n_tuples / alp::config::VECTOR_SIZE;

			/* Init */
			alp::encoder<PT>::init(data_column, rowgroup_offset, n_tuples, sample_buf, stt);

			double compression_ratio {0};
			switch (stt.scheme) {
			case alp::Scheme::ALP_RD: {
				alp::rd_encoder<PT>::init(data_column, rowgroup_offset, n_tuples, sample_buf, stt);

				/* Encode - Decode - Validate. */
				for (size_t i = 0; i < n_tuples; i++) {
					value_to_encode        = data_column[i];
					intput_buf[vector_idx] = value_to_encode;
					vector_idx             = vector_idx + 1;
					rowgroup_offset        = rowgroup_offset + 1;
					rowgroup_counter       = rowgroup_counter + 1;

					if (vector_idx != alp::config::VECTOR_SIZE) { continue; }

					if (rowgroup_counter == alp::config::ROWGROUP_SIZE) {
						rowgroup_counter = 0;
						alp::encoder<PT>::init(data_column, rowgroup_offset, n_tuples, sample_buf, stt);
					}

					// Encode
					alp::rd_encoder<PT>::encode(intput_buf, rd_exc_arr, pos_arr, exc_c_arr, right_buf, left_arr, stt);
					ffor::ffor(right_buf, ffor_right_buf, stt.right_bit_width, &stt.right_for_base);
					ffor::ffor(left_arr, ffor_left_arr, stt.left_bit_width, &stt.left_for_base);

					// Decode
					unffor::unffor(ffor_right_buf, unffor_right_buf, stt.right_bit_width, &stt.right_for_base);
					unffor::unffor(ffor_left_arr, unffor_left_arr, stt.left_bit_width, &stt.left_for_base);
					alp::rd_encoder<PT>::decode(
					    glue_buf, unffor_right_buf, unffor_left_arr, rd_exc_arr, pos_arr, exc_c_arr, stt);

					auto* dbl_glue_arr = reinterpret_cast<PT*>(glue_buf);
					for (size_t j = 0; j < alp::config::VECTOR_SIZE; ++j) {
						auto l = intput_buf[j];
						auto r = dbl_glue_arr[j];
						if (l != r) { std::cerr << j << ", " << dataset.name << "\n"; }

						ASSERT_EQ(intput_buf[j], dbl_glue_arr[j]);
					}

					alp_bench::VectorMetadata vector_metadata;
					vector_metadata.right_bit_width  = stt.right_bit_width;
					vector_metadata.left_bit_width   = stt.left_bit_width;
					vector_metadata.exceptions_count = stt.exceptions_count;

					compression_metadata.push_back(vector_metadata);
					vector_idx = 0;
					bit_width  = 0;

					vectors_count = vectors_count + 1;
				}

				compression_ratio = calculate_alprd_compression_size(compression_metadata);

			} break;
			case alp::Scheme::ALP: {
				/* Encode - Decode - Validate. */
				for (size_t i = 0; i < n_tuples; i++) {
					value_to_encode        = data_column[i];
					intput_buf[vector_idx] = value_to_encode;
					vector_idx             = vector_idx + 1;
					rowgroup_offset        = rowgroup_offset + 1;
					rowgroup_counter       = rowgroup_counter + 1;

					if (vector_idx != alp::config::VECTOR_SIZE) { continue; }
					if (rowgroup_counter == alp::config::ROWGROUP_SIZE) {
						rowgroup_counter = 0;
						rowgroups_count  = rowgroups_count + 1;
						alp::encoder<PT>::init(data_column, rowgroup_offset, n_tuples, sample_buf, stt);
					}
					alp::encoder<PT>::encode(intput_buf, exc_arr, pos_arr, exc_c_arr, encoded_buf, stt);
					alp::encoder<PT>::analyze_ffor(encoded_buf, bit_width, base_buf);
					ffor::ffor(encoded_buf, ffor_buf, bit_width, base_buf);

					unffor::unffor(ffor_buf, unffor_arr, bit_width, base_buf);
					alp::decoder<PT>::decode(unffor_arr, stt.fac, stt.exp, decoded_buf);
					alp::decoder<PT>::patch_exceptions(decoded_buf, exc_arr, pos_arr, exc_c_arr);

					for (size_t j = 0; j < alp::config::VECTOR_SIZE; j++) {
						auto l = intput_buf[j];
						auto r = decoded_buf[j];
						if (l != r) { std::cerr << j << ", " << rowgroup_offset << ", " << dataset.name << "\n"; }
						ASSERT_EQ(intput_buf[j], decoded_buf[j]);
					}
					compression_metadata.push_back({bit_width, exc_c_arr[0]});
					vector_idx = 0;
					bit_width  = 0;
				}
				compression_ratio = calculate_alp_compression_size<PT>(compression_metadata);
			} break;
			default:
				ASSERT_TRUE(false);
			}

			ofile << std::fixed << std::setprecision(2) << dataset.id << "," << dataset.name << "," << compression_ratio
			      << "," << get_alp_scheme_string(stt.scheme) << "," << rowgroups_count << "," << vectors_count
			      << std::endl;

			if (alp_bench::results.find(dataset.name) !=
			    alp_bench::results.end()) { // To avoid error when tested dataset is not found on results
				ASSERT_EQ(alp_bench::to_str(compression_ratio), alp_bench::results.find(dataset.name)->second);
			}
		}
		delete[] sample_buf;
		delete[] intput_buf;
		delete[] exc_arr;
		delete[] rd_exc_arr;
		delete[] pos_arr;
		delete[] encoded_buf;
		delete[] decoded_buf;
		delete[] exc_c_arr;
		delete[] ffor_buf;
		delete[] unffor_arr;
		delete[] base_buf;
		delete[] right_buf;
		delete[] left_arr;
		delete[] unffor_right_buf;
		delete[] unffor_left_arr;
	}

	template <typename PT>
	double calculate_alp_compression_size(std::vector<alp_bench::VectorMetadata>& vector_metadata) {
		double avg_bits_per_value {0};
		for (auto& metadata : vector_metadata) {
			avg_bits_per_value = avg_bits_per_value + metadata.bit_width;
			avg_bits_per_value =
			    avg_bits_per_value +
			    (static_cast<double>(metadata.exceptions_count) *
			     (alp::Constants<double>::EXCEPTION_SIZE + alp::EXCEPTION_POSITION_SIZE) / alp::config::VECTOR_SIZE);
		}

		avg_bits_per_value = avg_bits_per_value / vector_metadata.size();
		avg_bits_per_value = avg_bits_per_value + get_overhead_per_vector<PT>();
		return avg_bits_per_value;
	}

	double alprd_overhead_per_vector {static_cast<double>(alp::config::MAX_RD_DICTIONARY_SIZE * 16) /
	                                  alp::config::ROWGROUP_SIZE};

	double calculate_alprd_compression_size(std::vector<alp_bench::VectorMetadata>& vector_metadata) {
		double avg_bits_per_value {0};
		for (auto& metadata : vector_metadata) {
			avg_bits_per_value = avg_bits_per_value + metadata.right_bit_width + metadata.left_bit_width +
			                     static_cast<double>(metadata.exceptions_count *
			                                         (alp::RD_EXCEPTION_SIZE + alp::RD_EXCEPTION_POSITION_SIZE)) /
			                         alp::config::VECTOR_SIZE;
		}

		avg_bits_per_value = avg_bits_per_value / vector_metadata.size();
		avg_bits_per_value = avg_bits_per_value + alprd_overhead_per_vector;

		return avg_bits_per_value;
	}
};

TEST_F(ALPBench, bench_alp_on_alp_dataset) {
	std::string result_path = alp_bench::get_paths().alp_result_dir_path + "compression_ratio/double/alp_dataset.csv";
	bench_alp_compression_ratio<double>(alp_bench::get_alp_dataset(), result_path);
}

TEST_F(ALPBench, bench_alp_on_sp_dataset) {
	std::string result_path = alp_bench::get_paths().alp_result_dir_path + "compression_ratio/float/sp_dataset.csv";
	bench_alp_compression_ratio<float>(alp_bench::get_sp_datasets(), result_path);
}

TEST_F(ALPBench, bench_alp_on_hurricane_isabel) {
	auto result_path =
	    alp_bench::get_paths().alp_result_dir_path + "compression_ratio/float/hurricane_isabel_dataset.csv";
	bench_alp_compression_ratio<float>(alp_bench::get_hurricane_isabel_dataset(), result_path);
}
