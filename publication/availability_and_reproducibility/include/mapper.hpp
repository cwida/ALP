#ifndef TEST_MAPPER_HPP
#define TEST_MAPPER_HPP

#include <fcntl.h>
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

namespace mapper {

template <typename T>
inline T* mmap_file(size_t& n_values, const std::string& filename) {
	struct stat file_stats;

	const int fd = ::open(filename.c_str(), O_RDONLY);
	fstat(fd, &file_stats);
	const size_t file_size    = file_stats.st_size;
	auto*        file_pointer = static_cast<T*>(mmap(0, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
	n_values                  = file_size / sizeof(T);
	return file_pointer;
}

} // namespace mapper

#endif