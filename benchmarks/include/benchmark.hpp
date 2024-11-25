#include "alp.hpp"
#include "alp_result.hpp"
#include "data.hpp"
#include "gtest/gtest.h"
#include <unordered_map>

#ifndef ALP_BENCH_ALP_HPP
#define ALP_BENCH_ALP_HPP

using namespace alp::config;
using namespace alp_bench;

// ALP overhead per vector : bit_width + factor-idx + exponent-idx + ffor base;
template <typename PT>
PT get_overhead_per_vector() {
	return static_cast<PT>(8 +               // bit_width
	                       8 +               // factor-idx
	                       8 +               // exponent-idx
	                       (sizeof(PT) * 8)) // ffor base
	       / VECTOR_SIZE;
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

template <typename PT>
PT* get_data(size_t rg_idx, PT* data_column, size_t vector_idx) {
	size_t offset = (rg_idx * N_VECTORS_PER_ROWGROUP + vector_idx) * VECTOR_SIZE;
	PT*    data_p = data_column + offset;
	return data_p;
}

template <typename PT>
PT* get_data(size_t rg_idx, PT* data_column) {
	size_t offset = (rg_idx * N_VECTORS_PER_ROWGROUP) * VECTOR_SIZE;
	PT*    data_p = data_column + offset;
	return data_p;
}

// Mapping Definitions
std::unordered_map<std::string, DataType> data_type_map = {
    {"invalid", DataType::INVALID},
    {"double", DataType::DOUBLE},
    {"float", DataType::FLOAT},
};

std::unordered_map<std::string, FileType> file_type_map = {
    {"invalid", FileType::INVALID},
    {"binary", FileType::BINARY},
    {"csv", FileType::CSV},
};

// Helper Function to Convert String to Lowercase
std::string to_lower(const std::string& input) {
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(),
	               [](unsigned char c) { return std::tolower(c); });
	return result;
}

// Parse Function
std::vector<ColumnDescriptor> parse_column_records(const std::string& filename) {
	std::vector<ColumnDescriptor> records;
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error("Could not open file: " + filename);
	}

	std::string line;
	// Skip the header line
	std::getline(file, line);

	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string id, column_name, data_type, path, file_type;

		// Parse each column
		std::getline(stream, id, ',');
		std::getline(stream, column_name, ',');
		std::getline(stream, data_type, ',');
		std::getline(stream, path, ',');
		std::getline(stream, file_type, ',');

		// Convert strings to lowercase and map to enums
		DataType data_type_enum = data_type_map.count(to_lower(data_type)) ? data_type_map[to_lower(data_type)]
		                                                                   : DataType::INVALID;
		FileType file_type_enum = file_type_map.count(to_lower(file_type)) ? file_type_map[to_lower(file_type)]
		                                                                     : FileType::INVALID;

		// Add to records
		records.push_back({
		    std::stoi(id),  // Convert ID to integer
		    data_type_enum, // Data type enum
		    column_name,    // Column name
		    path,           // File path
		    file_type_enum  // File type enum
		});
	}

	file.close();
	return records;
}

template <typename PT>
std::string get_type_string() {
	if constexpr (std::is_same_v<PT, double>) {
		return "double";
	} else if constexpr (std::is_same_v<PT, float>) {
		return "float";
	} else {
		throw std::runtime_error("not supported!");
	}
}

void write_result_header(std::ofstream& ofile) { ofile << "idx,column,data_type,size,rowgroups_count,vectors_count\n"; }

template <typename PT>
ColumnDescriptor extract_column_descriptor(const ALPColumnDescriptor& alp_column) {
	if (alp_column.csv_file_path.empty() && alp_column.binary_file_path.empty()) {
		throw std::invalid_argument("Both csv_file_path and binary_file_path cannot be empty.");
	}

	// Determine the file path and file type
	std::string path = !alp_column.binary_file_path.empty() ? alp_column.binary_file_path : alp_column.csv_file_path;
	FileType    file_type = !alp_column.binary_file_path.empty() ? FileType::BINARY : FileType::CSV;

	// Infer the data type from PT
	DataType data_type;
	if constexpr (std::is_same_v<PT, float>) {
		data_type = DataType::FLOAT;
	} else if constexpr (std::is_same_v<PT, double>) {
		data_type = DataType::DOUBLE;
	} else {
		data_type = DataType::INVALID;
	}

	// Create and return the ColumnDescriptor
	return ColumnDescriptor {
	    static_cast<int>(alp_column.id), // Convert ID to int
	    data_type,                       // Inferred data type
	    alp_column.name,                 // Column name
	    path,                            // File path
	    file_type                        // Inferred file type
	};
}

class ALPBench : public ::testing::Test {
public:
	uint64_t* sample_buf {};
	uint64_t* exc_buf {};
	uint16_t* rd_exc_buf {};
	uint16_t* pos_buf {};
	uint16_t* exc_c_buf {};
	uint64_t* ffor_buf {};
	uint64_t* unffor_buf {};
	uint64_t* base_buf {};
	uint64_t* encoded_buf {};
	uint64_t* decoded_buf {};
	uint64_t* ffor_right_buf {};
	uint16_t* ffor_left_buf {};
	uint64_t* right_buf {};
	uint16_t* left_buf {};
	uint64_t* unffor_right_buf {};
	uint16_t* unffor_left_buf {};
	uint64_t* glue_buf {};
	uint8_t   bit_width {};

public:
	~ALPBench() override = default;

	void SetUp() override {
		exc_buf          = new uint64_t[VECTOR_SIZE];
		rd_exc_buf       = new uint16_t[VECTOR_SIZE];
		pos_buf          = new uint16_t[VECTOR_SIZE];
		encoded_buf      = new uint64_t[VECTOR_SIZE];
		decoded_buf      = new uint64_t[VECTOR_SIZE];
		exc_c_buf        = new uint16_t[VECTOR_SIZE];
		ffor_buf         = new uint64_t[VECTOR_SIZE];
		unffor_buf       = new uint64_t[VECTOR_SIZE];
		base_buf         = new uint64_t[VECTOR_SIZE];
		sample_buf       = new uint64_t[VECTOR_SIZE];
		right_buf        = new uint64_t[VECTOR_SIZE];
		left_buf         = new uint16_t[VECTOR_SIZE];
		ffor_right_buf   = new uint64_t[VECTOR_SIZE];
		ffor_left_buf    = new uint16_t[VECTOR_SIZE];
		unffor_right_buf = new uint64_t[VECTOR_SIZE];
		unffor_left_buf  = new uint16_t[VECTOR_SIZE];
		glue_buf         = new uint64_t[VECTOR_SIZE];
	}

	void TearDown() override {
		delete[] sample_buf;
		delete[] exc_buf;
		delete[] rd_exc_buf;
		delete[] pos_buf;
		delete[] encoded_buf;
		delete[] decoded_buf;
		delete[] exc_c_buf;
		delete[] ffor_buf;
		delete[] unffor_buf;
		delete[] base_buf;
		delete[] right_buf;
		delete[] left_buf;
		delete[] unffor_right_buf;
		delete[] unffor_left_buf;
	}

	template <typename PT>
	void typed_bench_column(const ColumnDescriptor& column, std::ofstream& ofile) {
		// Internal Type
		using UT = typename alp::inner_t<PT>::ut;
		using ST = typename alp::inner_t<PT>::st;

		PT*       sample_arr       = reinterpret_cast<PT*>(sample_buf);
		PT*       exc_arr          = reinterpret_cast<PT*>(exc_buf);
		uint16_t* rd_exc_arr       = reinterpret_cast<uint16_t*>(rd_exc_buf);
		uint16_t* pos_arr          = reinterpret_cast<uint16_t*>(pos_buf);
		uint16_t* exc_c_arr        = reinterpret_cast<uint16_t*>(exc_c_buf);
		ST*       ffor_arr         = reinterpret_cast<ST*>(ffor_buf);
		ST*       unffor_arr       = reinterpret_cast<ST*>(unffor_buf);
		ST*       base_arr         = reinterpret_cast<ST*>(base_buf);
		ST*       encoded_arr      = reinterpret_cast<ST*>(encoded_buf);
		PT*       decoded_arr      = reinterpret_cast<PT*>(decoded_buf);
		UT*       ffor_right_arr   = reinterpret_cast<UT*>(ffor_right_buf);
		uint16_t* ffor_left_arr    = reinterpret_cast<uint16_t*>(ffor_left_buf);
		UT*       right_arr        = reinterpret_cast<UT*>(right_buf);
		uint16_t* left_arr         = reinterpret_cast<uint16_t*>(left_buf);
		UT*       unffor_right_arr = reinterpret_cast<UT*>(unffor_right_buf);
		uint16_t* unffor_left_arr  = reinterpret_cast<uint16_t*>(unffor_left_buf);
		PT*       glue_arr         = reinterpret_cast<PT*>(glue_buf);

		std::cout << column.name << std::endl;

		// read data
		std::vector<PT> data;
		alp_data::read_data(data, column);
		PT*    data_column = data.data();
		size_t n_tuples    = data.size();

		size_t n_vecs      = n_tuples / VECTOR_SIZE;
		auto   n_rowgroups = static_cast<size_t>(std::ceil(static_cast<double>(n_tuples) / ROWGROUP_SIZE));
		std::vector<alp_bench::VectorMetadata> compression_metadata;
		PT                                     value_to_encode {0.0};
		size_t                                 rowgroup_counter {0};
		alp::state<PT>                         stt;

		/* Encode - Decode - Validate. */
		double compression_ratio {0};
		for (size_t rg_idx = 0; rg_idx < n_rowgroups; rg_idx++) {
			/* Init */
			PT* cur_rg_p = get_data(rg_idx, data_column);

			auto n_vec_per_current_rg =
			    (rg_idx == n_rowgroups - 1) ? n_vecs % N_VECTORS_PER_ROWGROUP : N_VECTORS_PER_ROWGROUP;
			auto n_values_per_current_rg = n_vec_per_current_rg * VECTOR_SIZE;
			alp::encoder<PT>::init(cur_rg_p, rg_idx, n_values_per_current_rg, sample_arr, stt);

			switch (stt.scheme) {
			case alp::Scheme::ALP_RD: {
				alp::rd_encoder<PT>::init(cur_rg_p, 0, n_values_per_current_rg, sample_arr, stt);
				for (size_t vector_idx {0}; vector_idx < n_vec_per_current_rg; vector_idx++) {
					PT* cur_vec_p = get_data(rg_idx, data_column, vector_idx);

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
						if (l != r) { std::cerr << j << ", " << column.name << "\n"; }

						ASSERT_EQ(cur_vec_p[j], dbl_glue_arr[j]);
					}

					alp_bench::VectorMetadata vector_metadata;
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
					PT* data_p = get_data(rg_idx, data_column, vector_idx);

					alp::encoder<PT>::encode(data_p, exc_arr, pos_arr, exc_c_arr, encoded_arr, stt);
					alp::encoder<PT>::analyze_ffor(encoded_arr, bit_width, base_arr);
					ffor::ffor(encoded_arr, ffor_arr, bit_width, base_arr);

					unffor::unffor(ffor_arr, unffor_arr, bit_width, base_arr);
					alp::decoder<PT>::decode(unffor_arr, stt.fac, stt.exp, decoded_arr);
					alp::decoder<PT>::patch_exceptions(decoded_arr, exc_arr, pos_arr, exc_c_arr);

					for (size_t j = 0; j < VECTOR_SIZE; j++) {
						auto l = data_p[j];
						auto r = decoded_arr[j];
						if (l != r) { std::cerr << j << ", " << rg_idx << ", " << column.name << "\n"; }
						ASSERT_EQ(data_p[j], decoded_arr[j]);
					}

					alp_bench::VectorMetadata vector_metadata;
					vector_metadata.bit_width        = bit_width;
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
		ofile << std::fixed << std::setprecision(2) << column.id << "," << column.name << "," << get_type_string<PT>()
		      << "," << compression_ratio << "," << n_rowgroups << "," << n_vecs << std::endl;
	}

	template <typename PT, size_t N_COLS>
	void typed_bench_dataset(std::array<alp_bench::ALPColumnDescriptor, N_COLS> columns,
	                         const std::string&                                 result_file_path) {

		std::ofstream ofile(result_file_path, std::ios::out);
		write_result_header(ofile);

		for (auto& alp_column_descriptor : columns) {
			auto column_descriptor = extract_column_descriptor<PT>(alp_column_descriptor);
			typed_bench_column<PT>(column_descriptor, ofile);
		}
	}

	template <typename PT>
	double calculate_alp_compression_size(std::vector<alp_bench::VectorMetadata>& vector_metadatas) {
		double avg_bits_per_value {0};
		for (auto& vector_metadata : vector_metadatas) {
			switch (vector_metadata.scheme) {
			case alp::Scheme::ALP_RD: {
				avg_bits_per_value += calculate_alprd_compression_size(vector_metadata);
				break;
			}
			case alp::Scheme::ALP: {

				avg_bits_per_value += calculate_alp_pde_compression_size<PT>(vector_metadata);
				break;
			}
			default:
				throw std::runtime_error("not scheme is chosen");
			}
		}

		avg_bits_per_value = avg_bits_per_value / vector_metadatas.size();
		return avg_bits_per_value;
	}

	template <typename PT>
	double calculate_alp_pde_compression_size(alp_bench::VectorMetadata& vector_metadata) {
		double avg_bits_per_value {0};
		avg_bits_per_value = avg_bits_per_value + vector_metadata.bit_width;
		avg_bits_per_value = avg_bits_per_value +
		                     (static_cast<double>(vector_metadata.exceptions_count) *
		                      (alp::Constants<double>::EXCEPTION_SIZE + alp::EXCEPTION_POSITION_SIZE) / VECTOR_SIZE);

		avg_bits_per_value = avg_bits_per_value + get_overhead_per_vector<PT>();
		return avg_bits_per_value;
	}

	double alprd_overhead_per_vector {static_cast<double>(MAX_RD_DICTIONARY_SIZE * 16) / ROWGROUP_SIZE};

	double calculate_alprd_compression_size(alp_bench::VectorMetadata& vector_metadata) {
		double avg_bits_per_value {0};
		avg_bits_per_value = avg_bits_per_value + vector_metadata.right_bit_width + vector_metadata.left_bit_width +
		                     static_cast<double>(vector_metadata.exceptions_count *
		                                         (alp::RD_EXCEPTION_SIZE + alp::RD_EXCEPTION_POSITION_SIZE)) /
		                         VECTOR_SIZE;

		avg_bits_per_value = avg_bits_per_value + alprd_overhead_per_vector;
		return avg_bits_per_value;
	}

	void bench_dataset() {
		const std::string dataset_description_file =
		    ALP_CMAKE_SOURCE_DIR "/benchmarks/your_own_dataset.csv"; // Replace with your CSV file path

		const std::string result_file_path =
		    ALP_CMAKE_SOURCE_DIR "/benchmarks/your_own_dataset_result.csv"; // Replace with your CSV file path

		std::ofstream ofile(result_file_path, std::ios::out);
		write_result_header(ofile);

			std::vector<ColumnDescriptor> columns = parse_column_records(dataset_description_file);
			for (const auto& column : columns) {
				switch (column.data_type) {
				case DataType::DOUBLE: {
					typed_bench_column<double>(column, ofile);
					break;
				}
				case DataType::FLOAT: {
					typed_bench_column<float>(column, ofile);
					break;
				}
				case DataType::INVALID:
				default:
					throw std::runtime_error("NOT supported type.");
				}
			}
	}
};

#endif // ALP_BENCH_ALP_HPP
