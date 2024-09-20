#include "alp.hpp"
#include "alp_result.hpp"
#include "data.hpp"
#include "test/mapper.hpp"
#include "gtest/gtest.h"
#include <unordered_map>

// NOLINTBEGIN

using namespace alp::config;
/*
 * ALP overhead per vector in a hypothetic file format = bit_width + factor-idx + exponent-idx + ffor base;
 */
double overhead_per_vector {static_cast<double>(8 + 8 + 8 + 64) / VECTOR_SIZE};

double calculate_alp_compression_size(std::vector<alp_bench::VectorMetadata>& vector_metadata) {
	double avg_bits_per_value {0};
	for (auto& metadata : vector_metadata) {
		avg_bits_per_value = avg_bits_per_value + metadata.bit_width;
		avg_bits_per_value = avg_bits_per_value +
		                     (static_cast<double>(metadata.exceptions_count) *
		                      (alp::Constants<double>::EXCEPTION_SIZE + alp::EXCEPTION_POSITION_SIZE) / VECTOR_SIZE);
	}

	avg_bits_per_value = avg_bits_per_value / vector_metadata.size();
	avg_bits_per_value = avg_bits_per_value + overhead_per_vector;
	return avg_bits_per_value;
}

/*
 * ALPRD Overhead per vector in a hypothetic file format in which the left parts dictionary is at the start of a
 * rowgroup
 */
double alprd_overhead_per_vector {static_cast<double>(MAX_RD_DICTIONARY_SIZE * 16) / ROWGROUP_SIZE};

double calculate_alprd_compression_size(std::vector<alp_bench::VectorMetadata>& vector_metadata) {
	double avg_bits_per_value {0};
	for (auto& metadata : vector_metadata) {
		avg_bits_per_value = avg_bits_per_value + metadata.right_bit_width + metadata.left_bit_width +
		                     static_cast<double>(metadata.exceptions_count *
		                                         (alp::RD_EXCEPTION_SIZE + alp::RD_EXCEPTION_POSITION_SIZE)) /
		                         VECTOR_SIZE;
	}

	avg_bits_per_value = avg_bits_per_value / vector_metadata.size();
	avg_bits_per_value = avg_bits_per_value + alprd_overhead_per_vector;

	return avg_bits_per_value;
}

double get_average_exception_count(std::vector<alp_bench::VectorMetadata>& vector_metadata) {
	double avg_exceptions_count {0};
	for (auto& metadata : vector_metadata) {
		avg_exceptions_count = avg_exceptions_count + metadata.exceptions_count;
	}

	avg_exceptions_count = avg_exceptions_count / vector_metadata.size();
	return avg_exceptions_count;
}

// we prefer the binary_path over csv_path
void read_data(std::vector<double>& data, const std::string& csv_file_path, const std::string& bin_file_path) {
	if (!bin_file_path.empty()) {

		// Open the binary file in input mode
		std::ifstream file(bin_file_path, std::ios::binary | std::ios::in);

		if (!file) { throw std::runtime_error("Failed to open file: " + bin_file_path); }

		// Get the size of the file
		file.seekg(0, std::ios::end);
		std::streamsize fileSize = file.tellg();
		file.seekg(0, std::ios::beg);

		// Ensure the file size is a multiple of the size of a double
		if (fileSize % sizeof(double) != 0) { throw std::runtime_error("File size is not a multiple of double size!"); }
		// Calculate the number of doubles
		std::size_t numDoubles = fileSize / sizeof(double);

		// Resize the vector to hold all the doubles
		data.resize(numDoubles);

		// Read the data into the vector
		file.read(reinterpret_cast<char*>(data.data()), fileSize);

		// Close the file
		file.close();
		return;
	}
	if (!csv_file_path.empty()) {
		const auto&   path = csv_file_path;
		std::ifstream file(path);

		if (!file) { throw std::runtime_error("Failed to open file: " + path); }

		std::string line;
		// Read each line, convert it to double, and store it in the vector
		while (std::getline(file, line)) {
			try {
				// Convert the string to double and add to the vector
				data.push_back(std::stod(line));
			} catch (const std::invalid_argument& e) {
				throw std::runtime_error("Invalid data in file: " + line);
			} catch (const std::out_of_range& e) {
				//
				throw std::runtime_error("Number out of range in file: " + line);
			}
		}

		file.close();
		return;
	}
	throw std::runtime_error("No bin or csv file specified");
}

class alp_test : public ::testing::Test {
public:
	double*   intput_buf {};
	double*   exc_arr {};
	uint16_t* rd_exc_arr {};
	uint16_t* pos_arr {};
	uint16_t* exc_c_arr {};
	int64_t*  ffor_buf {};
	int64_t*  unffor_arr {};
	int64_t*  base_buf {};
	int64_t*  encoded_buf {};
	double*   decoded_buf {};
	double*   sample_buf {};
	uint64_t* ffor_right_buf {};
	uint16_t* ffor_left_arr {};
	uint64_t* right_buf {};
	uint16_t* left_arr {};
	uint64_t* unffor_right_buf {};
	uint16_t* unffor_left_arr {};
	double*   glue_buf {};

	uint8_t bit_width {};

	void SetUp() override {
		intput_buf       = new double[VECTOR_SIZE];
		exc_arr          = new double[VECTOR_SIZE];
		rd_exc_arr       = new uint16_t[VECTOR_SIZE];
		pos_arr          = new uint16_t[VECTOR_SIZE];
		encoded_buf      = new int64_t[VECTOR_SIZE];
		decoded_buf      = new double[VECTOR_SIZE];
		exc_c_arr        = new uint16_t[VECTOR_SIZE];
		ffor_buf         = new int64_t[VECTOR_SIZE];
		unffor_arr       = new int64_t[VECTOR_SIZE];
		base_buf         = new int64_t[VECTOR_SIZE];
		sample_buf       = new double[VECTOR_SIZE];
		right_buf        = new uint64_t[VECTOR_SIZE];
		left_arr         = new uint16_t[VECTOR_SIZE];
		ffor_right_buf   = new uint64_t[VECTOR_SIZE];
		ffor_left_arr    = new uint16_t[VECTOR_SIZE];
		unffor_right_buf = new uint64_t[VECTOR_SIZE];
		unffor_left_arr  = new uint16_t[VECTOR_SIZE];
		glue_buf         = new double[VECTOR_SIZE];
	}

	~alp_test() override {
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
		delete[] sample_buf;
		delete[] right_buf;
		delete[] left_arr;
		delete[] unffor_right_buf;
		delete[] unffor_left_arr;
	}

	void bench_alp_compression_ratio(const alp_bench::Column& dataset, std::ofstream& ofile) {
		if (dataset.suitable_for_cutting) { return; }

		std::cout << dataset.name << std::endl;

		std::vector<double> data;
		read_data(data, dataset.csv_file_path, dataset.binary_file_path);
		double* data_column = data.data();
		size_t  n_tuples    = data.size();

		std::vector<alp_bench::VectorMetadata> compression_metadata;
		double                                 value_to_encode {0.0};
		size_t                                 vector_idx {0};
		size_t                                 rowgroup_counter {0};
		size_t                                 rowgroup_offset {0};
		alp::state<double>                     stt;
		size_t rowgroups_count = std::ceil(static_cast<double>(n_tuples) / ROWGROUP_SIZE);
		size_t vectors_count   = n_tuples / VECTOR_SIZE;

		/* Init */
		alp::encoder<double>::init(data_column, rowgroup_offset, n_tuples, sample_buf, stt);
		/* Encode - Decode - Validate. */
		for (size_t i = 0; i < n_tuples; i++) {
			value_to_encode        = data_column[i];
			intput_buf[vector_idx] = value_to_encode;
			vector_idx             = vector_idx + 1;
			rowgroup_offset        = rowgroup_offset + 1;
			rowgroup_counter       = rowgroup_counter + 1;

			if (vector_idx != VECTOR_SIZE) { continue; }
			if (rowgroup_counter == ROWGROUP_SIZE) {
				rowgroup_counter = 0;
				alp::encoder<double>::init(data_column, rowgroup_offset, n_tuples, sample_buf, stt);
			}
			alp::encoder<double>::encode(intput_buf, exc_arr, pos_arr, exc_c_arr, encoded_buf, stt);
			alp::encoder<double>::analyze_ffor(encoded_buf, bit_width, base_buf);
			ffor::ffor(encoded_buf, ffor_buf, bit_width, base_buf);

			unffor::unffor(ffor_buf, unffor_arr, bit_width, base_buf);
			alp::decoder<double>::decode(unffor_arr, stt.fac, stt.exp, decoded_buf);
			alp::decoder<double>::patch_exceptions(decoded_buf, exc_arr, pos_arr, exc_c_arr);

			for (size_t j = 0; j < VECTOR_SIZE; j++) {
				auto l = intput_buf[j];
				auto r = decoded_buf[j];
				if (l != r) { std::cerr << j << ", " << rowgroup_offset << ", " << dataset.name << "\n"; }
				ASSERT_EQ(intput_buf[j], decoded_buf[j]);
			}
			compression_metadata.push_back({bit_width, exc_c_arr[0]});
			vector_idx = 0;
			bit_width  = 0;
		}
		auto compression_ratio = calculate_alp_compression_size(compression_metadata);

		ofile << std::fixed << std::setprecision(2) << dataset.name << "," << compression_ratio << ","
		      << rowgroups_count << "," << vectors_count << std::endl;

		if (alp_bench::results.find(dataset.name) !=
		    alp_bench::results.end()) { // To avoid error when tested dataset is not found on results
			ASSERT_EQ(alp_bench::to_str(compression_ratio), alp_bench::results.find(dataset.name)->second);
		}
	}

	void bench_alp_rd_compression_ratio(const alp_bench::Column& dataset, std::ofstream& ofile) {
		if (!dataset.suitable_for_cutting) { return; }

		std::vector<alp_bench::VectorMetadata> compression_metadata;

		std::vector<double> data;
		read_data(data, dataset.csv_file_path, dataset.binary_file_path);
		double* data_column = data.data();
		size_t  n_tuples    = data.size();

		double             value_to_encode = 0.0;
		size_t             vector_idx {0};
		size_t             rowgroup_counter {0};
		size_t             rowgroup_offset {0};
		alp::state<double> stt;
		size_t             rowgroups_count {1};
		size_t             vectors_count {1};

		/* Init */
		alp::encoder<double>::init(data_column, rowgroup_offset, n_tuples, sample_buf, stt);

		ASSERT_EQ(stt.scheme, alp::Scheme::ALP_RD);

		alp::rd_encoder<double>::init(data_column, rowgroup_offset, n_tuples, sample_buf, stt);

		/* Encode - Decode - Validate. */
		for (size_t i = 0; i < n_tuples; i++) {
			value_to_encode        = data_column[i];
			intput_buf[vector_idx] = value_to_encode;
			vector_idx             = vector_idx + 1;
			rowgroup_offset        = rowgroup_offset + 1;
			rowgroup_counter       = rowgroup_counter + 1;

			if (vector_idx != VECTOR_SIZE) { continue; }

			if (rowgroup_counter == ROWGROUP_SIZE) {
				rowgroup_counter = 0;
				rowgroups_count  = rowgroups_count + 1;
			}

			// Encode
			alp::rd_encoder<double>::encode(intput_buf, rd_exc_arr, pos_arr, exc_c_arr, right_buf, left_arr, stt);
			ffor::ffor(right_buf, ffor_right_buf, stt.right_bit_width, &stt.right_for_base);
			ffor::ffor(left_arr, ffor_left_arr, stt.left_bit_width, &stt.left_for_base);

			// Decode
			unffor::unffor(ffor_right_buf, unffor_right_buf, stt.right_bit_width, &stt.right_for_base);
			unffor::unffor(ffor_left_arr, unffor_left_arr, stt.left_bit_width, &stt.left_for_base);
			alp::rd_encoder<double>::decode(
			    glue_buf, unffor_right_buf, unffor_left_arr, rd_exc_arr, pos_arr, exc_c_arr, stt);

			auto* dbl_glue_arr = reinterpret_cast<double*>(glue_buf);
			for (size_t j = 0; j < VECTOR_SIZE; ++j) {
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

		auto compression_ratio = calculate_alprd_compression_size(compression_metadata);

		ofile << std::fixed << std::setprecision(2) << dataset.name << "," << compression_ratio << ","
		      << rowgroups_count << "," << vectors_count << std::endl;

		if (alp_bench::results.find(dataset.name) !=
		    alp_bench::results.end()) { // To avoid error when tested dataset is not found on results
			ASSERT_EQ(alp_bench::to_str(compression_ratio), alp_bench::results.find(dataset.name)->second);
		}
	}
};

/*
 * Test to encode and decode whole datasets using ALP
 * This test will output and write a file with the estimated bits/value after compression with alp
 */
TEST_F(alp_test, test_alp_on_whole_datasets) {
	if (const auto v = std::getenv("ALP_DATASET_DIR_PATH"); v == nullptr) {
		throw std::runtime_error("Environment variable ALP_DATASET_DIR_PATH is not set!");
	}

	std::ofstream ofile(alp_bench::PATHS.RESULT_DIR_PATH + "alp_compression_ratio.csv", std::ios::out);
	ofile << "dataset,size,rowgroups_count,vectors_count\n";

	for (auto& dataset : alp_bench::alp_dataset) {
		bench_alp_compression_ratio(dataset, ofile);
	}
}

/*
 * Test to encode and decode whole datasets using ALP RD (aka ALP Cutter)
 * This test will output and write a file with the estimated bits/value after compression with alp
 */
TEST_F(alp_test, test_alprd_on_whole_datasets) {
	std::ofstream ofile(alp_bench::PATHS.RESULT_DIR_PATH + "alp_rd_compression_ratio.csv", std::ios::out);
	ofile << "dataset,size,rowgroups_count,vectors_count\n";

	for (auto& dataset : alp_bench::alp_dataset) {
		bench_alp_rd_compression_ratio(dataset, ofile);
	}
}

TEST_F(alp_test, test_alprd_on_evalimplsts) {
	std::ofstream ofile(alp_bench::PATHS.RESULT_DIR_PATH + "evalimplsts.csv", std::ios::out);
	ofile << "dataset,size,rowgroups_count,vectors_count\n";

	for (auto& dataset : alp_bench::evalimplsts) {
		bench_alp_rd_compression_ratio(dataset, ofile);
	}
}

// NOLINTEND
