#include "alp.hpp"
#include "helper.hpp"
#include <array>
#include <cassert>
#include <iostream>

using namespace alp;
int main() {
	constexpr std::array<int, 3> n_tup_vec {1, 1025, 1024 * 100};
	for (const auto n_tup : n_tup_vec) {
		size_t n_vec = ceil(n_tup / config::VECTOR_SIZE);
		// extra size needed if all values are exceptions.
		size_t if_all_exceptions_size =
		    n_vec * config::VECTOR_SIZE * sizeof(double) + sizeof(exp_p_t) + n_vec * sizeof(exp_c_t);

		size_t buffer_sz =
		    n_tup * sizeof(double) + if_all_exceptions_size; // We leave some headroom in case of negative compression
		size_t original_sz = n_tup * sizeof(double);

		double  in[n_tup];
		uint8_t out[buffer_sz];
		example::fill_random_data<double>(in, n_tup, 2);

		if (n_tup == 1) { in[0] = 204.92; }
		//  compress
		auto compressor = AlpCompressor<double>();
		compressor.compress(in, n_tup, out);

		// decompress
		auto   decompressed_buffer_size = AlpApiUtils<double>::align_value<size_t, config::VECTOR_SIZE>(n_tup);
		double decompressed[decompressed_buffer_size];
		auto   decompressor = AlpDecompressor<double>();
		decompressor.decompress(out, n_tup, decompressed);

		// check the correctnes
		for (size_t i = 0; i < n_tup; i++) {
			assert(in[i] == decompressed[i]);
		}

		// print result
		size_t compressed_size   = compressor.get_size();
		double compression_ratio = static_cast<double>(original_sz) / static_cast<double>(compressed_size);

		std::cout << "-- Compressing          " << n_tup << " random values with ALP" << std::endl;
		std::cout << "-- Uncompressed size:   " << original_sz << " bytes" << std::endl;
		std::cout << "-- Compressed   size:   " << compressed_size << " bytes" << std::endl;
		std::cout << "-- Compression ratio:   " << compression_ratio << std::endl;
		std::cout << "-----" << std::endl;
	}

	return 0;
}
