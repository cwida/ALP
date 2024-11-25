#ifndef DATA_HPP
#define DATA_HPP

#include "column.hpp"
#include "double/double_dataset.hpp"
#include "edge_case.hpp"
#include "evalimplsts.hpp"
#include "float/float_dataset.hpp"
#include "generated_columns.hpp"
#include <fstream>

namespace alp_data {

// we prefer the binary_path over csv_path
template <typename PT>
inline void read_data(std::vector<PT>& data, const alp_bench::ColumnDescriptor& column_descriptor) {
	switch (column_descriptor.file_type) {

	case alp_bench::FileType::BINARY: {
		// Open the binary file in input mode
		std::ifstream file(column_descriptor.path, std::ios::binary | std::ios::in);

		if (!file) { throw std::runtime_error("Failed to open file: " + column_descriptor.path); }

		// Get the size of the file
		file.seekg(0, std::ios::end);
		std::streamsize file_size = file.tellg();
		file.seekg(0, std::ios::beg);

		// Ensure the file size is a multiple of the size of a double
		// if (fileSize % sizeof(double) != 0) { throw std::runtime_error("File size is not a multiple of double
		// size!"); } Calculate the number of doubles
		std::size_t n_values = file_size / sizeof(PT);

		// Resize the vector to hold all the doubles
		data.resize(n_values);

		// Read the data into the vector
		file.read(reinterpret_cast<char*>(data.data()), file_size);

		// Close the file
		file.close();
	} break;
	case alp_bench::FileType::CSV: {
		const auto&   path = column_descriptor.path;
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
	} break;
	case alp_bench::FileType::INVALID:
	default: {
		throw std::runtime_error("No bin or csv file specified");
	}
	}
}

} // namespace alp_data

#endif // DATA_HPP
