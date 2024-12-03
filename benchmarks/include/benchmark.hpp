#include "alp.hpp"
#include "data.hpp"
#include "gtest/gtest.h"
#include <unordered_map>

#ifndef ALP_BENCH_ALP_HPP
#define ALP_BENCH_ALP_HPP

using namespace alp::config;
using namespace alp_bench;

namespace alp_bench {
// ALP overhead per vector : bit_width + factor-idx + exponent-idx + ffor base;
template <typename PT>
PT get_overhead_per_vector() {
	return static_cast<PT>(8 +               // bit_width
	                       8 +               // factor-idx
	                       8 +               // exponent-idx
	                       (sizeof(PT) * 8)) // ffor base
	       / VECTOR_SIZE;
};

struct VectorMetadata {
	uint8_t                                    bit_width {0};
	uint16_t                                   exceptions_count {0};
	uint64_t                                   unq_c {0};
	uint16_t                                   freq {0};
	double                                     size {0};
	uint64_t                                   right_bit_width {0};
	uint64_t                                   left_bit_width {0};
	std::vector<std::pair<uint16_t, uint64_t>> repetition_vec;
	alp::Scheme                                scheme;
};

inline std::string get_alp_scheme_string(alp::Scheme& scheme) {
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
const PT* get_data(const size_t rg_idx, const PT* data_column, const size_t vector_idx) {
	size_t    offset = (rg_idx * N_VECTORS_PER_ROWGROUP + vector_idx) * VECTOR_SIZE;
	const PT* data_p = data_column + offset;
	return data_p;
}

template <typename PT>
PT* get_data(size_t rg_idx, PT* data_column) {
	size_t offset = (rg_idx * N_VECTORS_PER_ROWGROUP) * VECTOR_SIZE;
	PT*    data_p = data_column + offset;
	return data_p;
}

// Mapping Definitions
inline std::unordered_map<std::string, DataType> data_type_map = {
    {"invalid", DataType::INVALID},
    {"double", DataType::DOUBLE},
    {"float", DataType::FLOAT},
};

inline std::unordered_map<std::string, FileType> file_type_map = {
    {"invalid", FileType::INVALID},
    {"binary", FileType::BINARY},
    {"csv", FileType::CSV},
};

// Helper Function to Convert String to Lowercase
inline std::string to_lower(const std::string& input) {
	std::string result = input;
	std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
	return result;
}

// Parse Function
inline std::vector<ColumnDescriptor> parse_column_records(const std::string& filename) {
	std::vector<ColumnDescriptor> records;
	std::ifstream                 file(filename);

	if (!file.is_open()) { throw std::runtime_error("Could not open file: " + filename); }

	std::string line;
	// Skip the header line
	std::getline(file, line);

	while (std::getline(file, line)) {
		std::istringstream stream(line);
		std::string        id, column_name, data_type, path, file_type;

		// Parse each column
		std::getline(stream, id, ',');
		std::getline(stream, column_name, ',');
		std::getline(stream, data_type, ',');
		std::getline(stream, path, ',');
		std::getline(stream, file_type, ',');

		// Convert strings to lowercase and map to enums
		DataType data_type_enum =
		    data_type_map.count(to_lower(data_type)) ? data_type_map[to_lower(data_type)] : DataType::INVALID;
		FileType file_type_enum =
		    file_type_map.count(to_lower(file_type)) ? file_type_map[to_lower(file_type)] : FileType::INVALID;

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

void write_result_header(std::ofstream& ofile);

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

struct BenchSpeedResult {
	double compression_speed;
	double decompression_speed;
};

class ALPBench : public ::testing::Test {
public:
	uint64_t* sample_buf {};
	uint64_t* data_buf {};
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
		data_buf         = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		exc_buf          = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		pos_buf          = new (std::align_val_t {64}) uint16_t[VECTOR_SIZE];
		encoded_buf      = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		exc_c_buf        = new (std::align_val_t {64}) uint16_t[VECTOR_SIZE];
		ffor_buf         = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		base_buf         = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		right_buf        = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		left_buf         = new (std::align_val_t {64}) uint16_t[VECTOR_SIZE];
		ffor_right_buf   = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		ffor_left_buf    = new (std::align_val_t {64}) uint16_t[VECTOR_SIZE];
		unffor_right_buf = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		unffor_left_buf  = new (std::align_val_t {64}) uint16_t[VECTOR_SIZE];
		glue_buf         = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		sample_buf       = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		rd_exc_buf       = new (std::align_val_t {64}) uint16_t[VECTOR_SIZE];
		unffor_buf       = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
		decoded_buf      = new (std::align_val_t {64}) uint64_t[VECTOR_SIZE];
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
	void typed_bench_column(const ColumnDescriptor& column, std::ofstream& ofile);

	template <typename PT, size_t N_COLS>
	void typed_bench_dataset(std::array<ALPColumnDescriptor, N_COLS> columns, const std::string& result_file_path);

	template <typename PT>
	double calculate_alp_compression_size(std::vector<VectorMetadata>& vector_metadatas) {
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
	double calculate_alp_pde_compression_size(VectorMetadata& vector_metadata) {
		double avg_bits_per_value {0};
		avg_bits_per_value = avg_bits_per_value + vector_metadata.bit_width;
		avg_bits_per_value = avg_bits_per_value +
		                     (static_cast<double>(vector_metadata.exceptions_count) *
		                      (alp::Constants<double>::EXCEPTION_SIZE + alp::EXCEPTION_POSITION_SIZE) / VECTOR_SIZE);

		avg_bits_per_value = avg_bits_per_value + get_overhead_per_vector<PT>();
		return avg_bits_per_value;
	}

	double alprd_overhead_per_vector {static_cast<double>(MAX_RD_DICTIONARY_SIZE * 16) / ROWGROUP_SIZE};

	double calculate_alprd_compression_size(VectorMetadata& vector_metadata) {
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

	template <typename PT>
	BenchSpeedResult typed_bench_speed_column(const std::vector<PT>& data);
};
} // namespace alp_bench

#endif // ALP_BENCH_ALP_HPP
