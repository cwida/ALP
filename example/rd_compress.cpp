#include "alp.hpp"
#include "helper.hpp"
#include <cassert>

int main() {
	size_t tuples_count = 1024 * 100;
	size_t out_buffer_size =
	    (tuples_count * sizeof(double)) + 8096; // We leave some headroom in case of negative compression
	size_t uncompressed_size = tuples_count * sizeof(double);

	double  in[tuples_count];
	uint8_t out[out_buffer_size];
	example::fill_random_data<double>(in, tuples_count, 20);

	/*
	 * If you know your doubles will have a random precision, you can directly use compress_rd
	 */
	alp::AlpCompressor compressor = alp::AlpCompressor<double>();
	printf("Compressing with ALPRD...\n");
	compressor.compress_rd(in, tuples_count, out);
	size_t compressed_size   = compressor.get_size();
	double compression_ratio = (double)uncompressed_size / compressed_size;
	printf("Uncompressed size (in bytes): %zu\n", uncompressed_size);
	printf("Compressed size (in bytes): %zu\n", compressed_size);
	printf("Compression Ratio: %f\n\n", compression_ratio);

	/*
	 * Decompress
	 */
	auto decompressed_buffer_size =
	    alp::AlpApiUtils<double>::align_value<size_t, alp::config::VECTOR_SIZE>(tuples_count);
	double decompressed[decompressed_buffer_size];
	auto   decompressor = alp::AlpDecompressor<double>();
	printf("Decompressing with ALPRD...\n");
	decompressor.decompress(out, tuples_count, decompressed);

	/*
	 * Validity Test
	 */
	for (size_t i = 0; i < tuples_count; i++) {
		assert(in[i] == decompressed[i]);
	}
	printf("OK\n");

	return 0;
}
