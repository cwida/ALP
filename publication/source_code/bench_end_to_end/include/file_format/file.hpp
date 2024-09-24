#ifndef FILE_HPP
#define FILE_HPP

#include <filesystem>
#include <fstream>
#include <string>

namespace ff {
struct file {
	static std::vector<std::string> get_col_paths(const std::string& col_name, encoding::scheme& scheme) {
		std::vector<std::string> result_vec;

		for (size_t i {0}; i < scheme.c; ++i) {
			result_vec.push_back(dataset::paths::get_cache_dir_path() + scheme.prefix + "_" + col_name + "_" +
			                     std::to_string(i) + scheme.extension);
		}

		return result_vec;
	}

	static bool contains_col(const std::string& col_name, encoding::scheme& scheme) {

		std::vector<std::string> file_path_vec = ff::file::get_col_paths(col_name, scheme);
		bool                     result        = true;

		for (auto& path : file_path_vec) {
			if (!std::ifstream(path)) { result = false; }
		}

		return false;
	}

	static size_t get_compressed_size(const std::string& col_name, encoding::scheme& scheme) {
		std::vector<std::string> file_path_vec = ff::file::get_col_paths(col_name, scheme);
		size_t                   ttl_sz {0};

		for (size_t i {1}; i < file_path_vec.size(); ++i) {
			std::uintmax_t size = std::filesystem::file_size(file_path_vec[i]);

			ttl_sz += size;
		}

		return ttl_sz;
	}

	static std::string get_details(const std::string& col_name, encoding::scheme& scheme) {
		std::vector<std::string> file_path_vec = ff::file::get_col_paths(col_name, scheme);
		std::string              detail {0};

		for (size_t i {0}; i < file_path_vec.size(); ++i) {
			std::uintmax_t size = std::filesystem::file_size(file_path_vec[i]);
			detail += std::to_string(i) + " | " + std::to_string((double(size * 8) / cfg::t_c)) + "\n";
		}

		//			std::uintmax_t size = (std::filesystem::file_size(file_path_vec[3]) / 2) / (1024 * 1024);
		//			detail += "extra | " + std::to_string(size) + "\n";

		return detail;
	}

	static size_t get_mtd_size(const std::string& col_name, encoding::scheme& scheme) {
		std::vector<std::string> file_path_vec = ff::file::get_col_paths(col_name, scheme);
		size_t                   ttl_sz {0};

		std::uintmax_t size = std::filesystem::file_size(file_path_vec[0]);

		ttl_sz += size;

		return ttl_sz;
	}
};
} // namespace ff

#endif // FILE_HPP
