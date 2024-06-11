#include "alp.hpp"
#include "helper.hpp"
#include <cassert>

int main() {
	size_t tuples_count = (1024 * 100);
	size_t out_buffer_size =
	    (tuples_count * sizeof(float)) + 8096; // We leave some headroom in case of negative compression
	size_t uncompressed_size = tuples_count * sizeof(float);

	float   in[tuples_count];
	uint8_t out[out_buffer_size];
	example::fill_random_data<float>(in, tuples_count, 20);

	/*
	 * Compress
	 */
	alp::AlpCompressor compressor = alp::AlpCompressor<float>();
	printf("Compressing with ALP...\n");
	compressor.compress_rd(in, tuples_count, out);
	size_t compressed_size   = compressor.get_size();
	double compression_ratio = (double)uncompressed_size / compressed_size;
	printf("Uncompressed size (in bytes): %zu\n", uncompressed_size);
	printf("Compressed size (in bytes): %zu\n", compressed_size);
	printf("Compression Ratio: %f\n\n", compression_ratio);

	/*
	 * Decompress
	 */
	size_t decompressed_buffer_size =
	    alp::AlpApiUtils<float>::align_value<size_t, alp::config::VECTOR_SIZE>(tuples_count);
	float                decompressed[decompressed_buffer_size];
	alp::AlpDecompressor decompressor = alp::AlpDecompressor<float>();
	printf("Decompressing with ALP...\n");
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
