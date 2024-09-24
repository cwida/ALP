#include "alp.hpp"
#include "benchmarks/alp/config.hpp"
#include "benchmarks/alp/queries.hpp"
#include "chimp/chimp.hpp"
#include "chimp/chimp128.hpp"
#include "common/runtime/Types.hpp"
#include "encoding/helper.hpp"
#include "gorillas/gorillas.hpp"
#include "patas/patas.hpp"
#include "tbb/tbb.h"
#include "vectorwise/Operators.hpp"
#include "vectorwise/Primitives.hpp"
#include "vectorwise/VectorAllocator.hpp"
#include "zstd.h"
#include <iomanip>

// ---

#include "btrblocks/scheme/double/Pseudodecimal.hpp"
#include "scheme/SchemePool.hpp"
#include <iostream>

using namespace runtime;
using namespace std;
using namespace btrblocks;

namespace unffor = fastlanes::generated::unffor::fallback::scalar;

namespace alp_end_to_end { namespace function {
// using F2 = pos_t (*)(pos_t n, void* result, void* param1);

using pos_t = uint32_t;
pos_t bitunpack_32(pos_t n, void* result, void* param1) {
	auto in  = reinterpret_cast<uint32_t*>(param1);
	auto out = reinterpret_cast<uint32_t*>(result);
	return n;
}

pos_t bitunpack_32_nothing(pos_t n, void* result, void* param1) {
	auto in  = reinterpret_cast<uint32_t*>(param1);
	auto out = reinterpret_cast<uint32_t*>(result);
	return n;
}

pos_t bitunpack_32_scalar(pos_t n, void* result, void* param1) {
	auto in  = reinterpret_cast<uint32_t*>(param1);
	auto out = reinterpret_cast<uint32_t*>(result);
	for (size_t i {0}; i < 1024; i = i + 32) {}
	return n;
}

pos_t print_double(pos_t n, void* result, void* param1) {
	auto in  = reinterpret_cast<double*>(param1);
	auto out = reinterpret_cast<double*>(result);
	for (size_t i {0}; i < 1024; ++i) {
		std::cout << std::setprecision(13) << __FUNCTION__ << " : result[" << i << "] : " << in[i] << std::endl;
	}
	return n;
}

pos_t nothing(pos_t n, void* p0, void* p1, void* p2, void* p3, void* p4) { return n; }

pos_t alp_func(pos_t n, void* p0, void* p1, void* p2, void* p3, void* p4) {
	auto alp         = *reinterpret_cast<alp_m*>(p0);
	auto ffor_arr    = reinterpret_cast<uint8_t*>(p1);
	auto exc_arr     = reinterpret_cast<uint8_t*>(p2);
	auto pos_arr     = reinterpret_cast<uint8_t*>(p3);
	auto dec_dbl_arr = reinterpret_cast<double*>(p4);

	ffor_arr += alp.ffor_arr_byte_c;
	exc_arr += alp.exc_byte_c;
	pos_arr += alp.exc_pos_byte_c;

	generated::falp::x86_64::avx512bw::falp(reinterpret_cast<uint64_t*>(ffor_arr),
	                                        dec_dbl_arr,
	                                        alp.bw,
	                                        reinterpret_cast<uint64_t*>(&alp.base),
	                                        alp.fac,
	                                        alp.exp);

	alp::decoder<double>::patch_exceptions(dec_dbl_arr, reinterpret_cast<double*>(exc_arr), reinterpret_cast<uint16_t*>(pos_arr), &alp.exc_c);

	//	std::cout << __FUNCTION__ << " : ";
	//	print_alp(*reinterpret_cast<X*>(&alp));
	//
	//	alp_debug::print_arr<double>(dec_dbl_arr, 1, std::cout);

	return n;
}

pos_t aggr_plus(pos_t n, void* result, void* param1) {
	auto in  = reinterpret_cast<double*>(param1);
	auto out = reinterpret_cast<double*>(result);

	for (size_t i {0}; i < 1024; ++i) {
		out[0] += in[i];
	}

	return n;
}

pos_t scan(pos_t n, void* result, void* param1) { return n; }

pos_t chimp_func(pos_t n, void* p0, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6) {
	auto cmp_m = *reinterpret_cast<chimp_m*>(p0);

	auto data_arr         = reinterpret_cast<uint8_t*>(p1);
	auto flags_arr        = reinterpret_cast<uint8_t*>(p2);
	auto leading_zero_arr = reinterpret_cast<uint8_t*>(p3);

	auto dec_arr = reinterpret_cast<uint64_t*>(p6);

	data_arr += cmp_m.data_arr_byte_c;
	flags_arr += cmp_m.flags_arr_byte_c;
	leading_zero_arr += cmp_m.leading_zero_arr_byte_c;

	//	std::cout << __FUNCTION__ << " : ";
	//	print_chimp(*reinterpret_cast<X*>(&chimp));

	auto* flags                 = reinterpret_cast<alp_bench::ChimpConstants::Flags*>(p4);
	auto* leading_zero_unpacked = reinterpret_cast<uint8_t*>(p5);

	alp_bench::FlagBuffer<false>                 flag_buffer;
	alp_bench::LeadingZeroBuffer<false>          leading_zero_buffer;
	alp_bench::ChimpDecompressionState<uint64_t> chimp_de_state;
	uint32_t                                     leading_zero_index;
	uint16_t                                     leading_zero_block_count;
	idx_t                                        leading_zero_block_size;

	// Init decoding
	leading_zero_block_count = cmp_m.d;
	leading_zero_block_size  = static_cast<int64_t>(leading_zero_block_count) * 8;
	leading_zero_index       = 0;
	chimp_de_state.input.SetStream(data_arr);
	flag_buffer.SetBuffer(flags_arr);
	leading_zero_buffer.SetBuffer(leading_zero_arr);

	/*
	 *
	 * DECODE
	 *
	 */
	flags[0] = alp_bench::ChimpConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
	for (idx_t i = 0; i < 1023; i++) {
		flags[1 + i] = (alp_bench::ChimpConstants::Flags)flag_buffer.Extract();
	}

	for (idx_t i = 0; i < leading_zero_block_size; i++) {
		leading_zero_unpacked[i] =
		    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[leading_zero_buffer.Extract()];
	}

	for (idx_t i = 0; i < 1024; i++) {
		dec_arr[i] = alp_bench::ChimpDecompression<uint64_t>::Load(
		    flags[i], leading_zero_unpacked, leading_zero_index, chimp_de_state);
	}
	chimp_de_state.Reset();

	//	if (cmp_m.vec_n == 0) { alp_debug::print_arr<double>(reinterpret_cast<double*>(dec_arr), 1024, std::cout); }

	return n;
}

pos_t chimp128_func(pos_t n, void* p0, void* p1, void* p2, void* p3, void* p4, void* p5, void* p6, void* p7, void* p8) {
	auto  cmp_m            = *reinterpret_cast<chimp128_m*>(p0);
	auto* data_arr         = reinterpret_cast<uint8_t*>(p1);
	auto* flags_arr        = reinterpret_cast<uint8_t*>(p2);
	auto* leading_zero_arr = reinterpret_cast<uint8_t*>(p3);
	auto* packed_data_arr  = reinterpret_cast<uint8_t*>(p4);
	auto* dec_arr          = reinterpret_cast<uint64_t*>(p8);

	data_arr += cmp_m.data_arr_byte_c;
	flags_arr += cmp_m.flags_arr_byte_c;
	leading_zero_arr += cmp_m.leading_zero_arr_byte_c;
	packed_data_arr += cmp_m.packed_data_arr_byte_c;

	// Decode
	idx_t                                           leading_zero_block_size;
	alp_bench::FlagBuffer<false>                    flag_buffer;
	alp_bench::LeadingZeroBuffer<false>             leading_zero_buffer;
	alp_bench::Chimp128DecompressionState<uint64_t> chimp_de_state;
	auto*                                           flags = reinterpret_cast<alp_bench::ChimpConstants::Flags*>(p5);
	auto*                                           unpacked_data_arr = reinterpret_cast<alp_bench::UnpackedData*>(p6);
	auto*                                           leading_zero_unpacked = reinterpret_cast<uint8_t*>(p7);
	uint8_t                                         leading_zero_block_count;
	uint32_t                                        unpacked_index;
	uint32_t                                        leading_zero_index;

	leading_zero_block_count = cmp_m.e;
	leading_zero_block_size  = static_cast<idx_t>(leading_zero_block_count) * 8;
	unpacked_index           = 0;
	leading_zero_index       = 0;
	chimp_de_state.input.SetStream(data_arr);
	flag_buffer.SetBuffer(flags_arr);
	leading_zero_buffer.SetBuffer(leading_zero_arr);

	// Decode flags
	flags[0] = alp_bench::ChimpConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
	for (idx_t i = 0; i < 1023; i++) {
		flags[1 + i] = (alp_bench::ChimpConstants::Flags)flag_buffer.Extract();
	}

	// Decode leading zero
	for (idx_t i = 0; i < leading_zero_block_size; i++) {
		leading_zero_unpacked[i] =
		    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[leading_zero_buffer.Extract()];
	}

	/*
	 * count how many cases of 'TRAILING_EXCEEDS_THRESHOLD' are based on the flags
	 * that is the exact number of packed data blocks
	 * that is the case in which in Chimp128 they save data in a block of 16 bits
	 */
	idx_t packed_data_block_count = 0;
	for (idx_t i = 0; i < 1024; i++) {
		packed_data_block_count += flags[1 + i] == alp_bench::ChimpConstants::Flags::TRAILING_EXCEEDS_THRESHOLD;
	}

	for (idx_t i = 0; i < packed_data_block_count; i++) {
		alp_bench::PackedDataUtils<uint64_t>::Unpack(((uint16_t*)packed_data_arr)[i], unpacked_data_arr[i]);
		if (unpacked_data_arr[i].significant_bits == 0) { unpacked_data_arr[i].significant_bits = 64; }
		unpacked_data_arr[i].leading_zero =
		    alp_bench::ChimpConstants::Decompression::LEADING_REPRESENTATION[unpacked_data_arr[i].leading_zero];
	}

	for (idx_t i = 0; i < 1024; i++) {
		dec_arr[i] = alp_bench::Chimp128Decompression<uint64_t>::Load(
		    flags[i], leading_zero_unpacked, leading_zero_index, unpacked_data_arr, unpacked_index, chimp_de_state);
	}

	return n;
}

pos_t patas_func(pos_t n, void* p0, void* p1, void* p2, void* p3, void* p4) {
	auto patas           = *reinterpret_cast<patas_m*>(p0);
	auto data_arr        = reinterpret_cast<uint8_t*>(p1);
	auto packed_metadata = reinterpret_cast<uint8_t*>(p2);
	auto dec_arr         = reinterpret_cast<uint64_t*>(p4);

	data_arr += patas.data_arr_byte_c;
	packed_metadata += patas.packed_data_arr_byte_c;

	auto*                 unpacked_data = reinterpret_cast<alp_bench::patas::PatasUnpackedValueStats*>(p3);
	alp_bench::ByteReader byte_reader;

	// Init decoding
	byte_reader.SetStream(data_arr);

	// UNPACKING METADATA (16 bits - 3 bytes)
	for (idx_t i = 0; i < 1024; i++) {
		alp_bench::PackedDataUtils<uint64_t>::Unpack(((uint16_t*)packed_metadata)[i],
		                                             (alp_bench::UnpackedData&)unpacked_data[i]);
	}

	// USING UNPACKED METADATA AND DATA BUFFER WE LOAD THE DOUBLE VALUES
	dec_arr[0] = (uint64_t)0; // Not sure why without this, it does not work on the > 2nd iteration...
	for (idx_t i = 0; i < 1024; i++) {
		dec_arr[i] =
		    alp_bench::patas::PatasDecompression<uint64_t>::DecompressValue(byte_reader,
		                                                                    unpacked_data[i].significant_bytes,
		                                                                    unpacked_data[i].trailing_zeros,
		                                                                    dec_arr[i - unpacked_data[i].index_diff]);
	}

	//	std::cout << __FUNCTION__ << " : ";
	//	print_chimp(*reinterpret_cast<X*>(&chimp));

	//	alp_debug::print_arr<double>(reinterpret_cast<double*>(dec_arr), 2, std::cout);

	return n;
}

pos_t gorilla_func(pos_t n, void* p0, void* p1, void* p2, void* p3, void* p4) {
	auto grl_m     = *reinterpret_cast<gorilla_m*>(p0);
	auto data_arr  = reinterpret_cast<uint8_t*>(p1);
	auto flags_arr = reinterpret_cast<uint8_t*>(p2);
	auto dec_arr   = reinterpret_cast<uint64_t*>(p4);

	data_arr += grl_m.data_arr_byte_c;
	flags_arr += grl_m.flag_buffer_byte_c;

	auto* flags = reinterpret_cast<alp_bench::GorillasConstants::Flags*>(p3);

	alp_bench::GorillasDecompressionState<uint64_t> gorillas_de_state;
	alp_bench::FlagBuffer<false>                    flag_buffer;

	// Init decoding
	gorillas_de_state.Reset();
	gorillas_de_state.input.SetStream(data_arr);
	flag_buffer.SetBuffer(flags_arr);

	// UNPACKING METADATA (16 bits - 3 bytes)
	flags[0] = alp_bench::GorillasConstants::Flags::VALUE_IDENTICAL; // First value doesn't require a flag
	for (idx_t i = 0; i < 1023; i++) {
		flags[1 + i] = (alp_bench::GorillasConstants::Flags)flag_buffer.Extract();
	}

	for (idx_t i = 0; i < 1024; i++) {
		dec_arr[i] = alp_bench::GorillasDecompression<uint64_t>::Load(flags[i], gorillas_de_state);
	}
	gorillas_de_state.Reset();

	//	std::cout << __FUNCTION__ << " : ";
	//	print_gorilla(*reinterpret_cast<X*>(&grl_m));
	//
	//	alp_debug::print_arr<double>(reinterpret_cast<double*>(dec_arr), 2, std::cout);

	return n;
}

double* dec_buffer = new double[cfg::t_c];
pos_t   zstd(pos_t n, void* p0, void* p1, void* p2) {
    auto  mtd         = *reinterpret_cast<zstd_m*>(p0);
    auto  enc_dbl_arr = reinterpret_cast<uint8_t*>(p1);
    auto  dec_arr     = reinterpret_cast<uint64_t*>(p2);
    auto  rg_n        = mtd.vec_n / 128;
    auto  vec_n       = mtd.vec_n % 128;
    auto* rg_p        = dec_buffer + (rg_n * cfg::morsel_c);
    auto* vec_p       = rg_p + (vec_n * 1024);

    enc_dbl_arr = enc_dbl_arr + mtd.enc_arr_byte_c;

    if (vec_n == 0) { ZSTD_decompress(rg_p, cfg::morsel_c * 8, enc_dbl_arr, mtd.enc_size); }

    std::memcpy(dec_arr, vec_p, 1024 * 8);

    //    if (vec_n == 35) alp_debug::print_arr<double>(reinterpret_cast<double*>(dec_arr), 1024, std::cout);

    return n;
}

pos_t pde_func(pos_t n, void* p0, void* p1, void* p2) {
	auto mtd         = *reinterpret_cast<zstd_m*>(p0);
	auto enc_dbl_arr = reinterpret_cast<uint8_t*>(p1);
	auto dec_arr     = reinterpret_cast<double*>(p2);

	enc_dbl_arr = enc_dbl_arr + mtd.enc_arr_byte_c;
	//	setupSchemePool();
	btrblocks::doubles::Decimal pd;

	pd.decompress(dec_arr, nullptr, enc_dbl_arr, 1024, 2);

	return n;
}

pos_t uncompressed_func(pos_t n, void* p0, void* p1, void* p2) {
	auto mtd = *reinterpret_cast<uncompressed_m*>(p0);
	auto in  = reinterpret_cast<uint8_t*>(p1);
	auto out = reinterpret_cast<double*>(p2);

	in = in + mtd.enc_arr_byte_c;

	auto* in_double = reinterpret_cast<double*>(in);

	for (size_t i {0}; i < 1024; ++i) {
		out[0] += in_double[i];
	}

	return n;
}

pos_t memcpy_func(pos_t n, void* p0, void* p1, void* p2) {
	auto mtd = *reinterpret_cast<uncompressed_m*>(p0);
	auto in  = reinterpret_cast<uint8_t*>(p1);
	auto out = reinterpret_cast<double*>(p2);

	in = in + mtd.enc_arr_byte_c;

	std::memcpy(out, in, 1024 * 8);

	return n;
}

pos_t void_scan_func(pos_t n, void* p0, void* p1, void* p2) {
	auto in  = reinterpret_cast<uint8_t*>(p1);
	auto out = reinterpret_cast<double*>(p2);

	auto* in_double = reinterpret_cast<double*>(in);

	for (size_t i {0}; i < 1024; ++i) {
		out[0] += in_double[i];
	}

	return n;
}

/// aggregate column into single value
pos_t aggr_static_col_nothing(pos_t n, void* RES result, void* RES param1) { return n > 0; }
}} // namespace alp_end_to_end::function

std::unique_ptr<alp_q_builder::query> alp_q_builder::get_q() {
	using namespace vectorwise;
	// --- constants
	auto  res    = make_unique<query>();
	auto& consts = *res;
	enum { uncompressed, tmp_1, tmp_2, tmp_3 };

	auto scan_bdr = Scan(cfg::tbl_name);
	switch (runtime::cur_q_mtd.query.q_t) {
	case cfg::DEBUG: {
		throw std::runtime_error("DEBUG QUERY TYPE IS NOT SUPPORTED");
	}
	case cfg::SUM: {
		switch (runtime::cur_q_mtd.scheme.enc_t) {
		case encoding::scheme_t::ALP: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::alp_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            X(scan_bdr, cfg::schema, 3),
			                            Buffer(uncompressed, sizeof(double)) //
			                            )
			                     .addOp(alp_end_to_end::function::aggr_plus,
			                            Value(&consts.aggregator), //
			                            Buffer(uncompressed)));

			break;
		}
		case encoding::scheme_t::CHIMP: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::chimp_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            X(scan_bdr, cfg::schema, 3),
			                            Buffer(tmp_1, sizeof(double)),
			                            Buffer(tmp_2, sizeof(double)),
			                            Buffer(uncompressed, sizeof(double)) //
			                            )
			                     .addOp(alp_end_to_end::function::aggr_plus,
			                            Value(&consts.aggregator), //
			                            Buffer(uncompressed)));
			break;
		}
		case encoding::scheme_t::PATAS: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::patas_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            Buffer(tmp_1, sizeof(double)),       //
			                            Buffer(uncompressed, sizeof(double)) //
			                            )
			                     .addOp(alp_end_to_end::function::aggr_plus,
			                            Value(&consts.aggregator), //
			                            Buffer(uncompressed)));
			break;
		}
		case encoding::scheme_t::CHIMP128: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::chimp128_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            X(scan_bdr, cfg::schema, 3),
			                            X(scan_bdr, cfg::schema, 4),
			                            Buffer(tmp_1, sizeof(double)),
			                            Buffer(tmp_2, sizeof(double)),
			                            Buffer(tmp_3, sizeof(double)),
			                            Buffer(uncompressed, sizeof(double)) //
			                            )
			                     .addOp(alp_end_to_end::function::aggr_plus,
			                            Value(&consts.aggregator), //
			                            Buffer(uncompressed)));
			break;
		}
		case encoding::scheme_t::GORILLA: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::gorilla_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            Buffer(tmp_1, sizeof(double)),       //
			                            Buffer(uncompressed, sizeof(double)) //
			                            )
			                     .addOp(alp_end_to_end::function::aggr_plus,
			                            Value(&consts.aggregator), //
			                            Buffer(uncompressed)));
			break;
		}
		case encoding::scheme_t::ZSTD: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::zstd,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            Buffer(uncompressed, sizeof(double)) //
			                            )
			                     .addOp(alp_end_to_end::function::aggr_plus,
			                            Value(&consts.aggregator), //
			                            Buffer(uncompressed)));
			break;
		}
		case encoding::scheme_t::UNCOMPRESSED: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::memcpy_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            Buffer(uncompressed, sizeof(double)) //
			                            )
			                     .addOp(alp_end_to_end::function::aggr_plus,
			                            Value(&consts.aggregator), //
			                            Buffer(uncompressed)));
			break;
		}
		case encoding::scheme_t::PDE: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::pde_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            Buffer(uncompressed, sizeof(double)) //
			                            )
			                     .addOp(alp_end_to_end::function::aggr_plus,
			                            Value(&consts.aggregator), //
			                            Buffer(uncompressed)));
			break;
		}
		case encoding::NONE: {
			throw std::runtime_error("NOT SUPPORTED");
		}
		case encoding::CST_BP: {
			throw std::runtime_error("NOT SUPPORTED");
		}
		case encoding::BITPACKED: {
			throw std::runtime_error("NOT SUPPORTED");
		}
		case encoding::ALP_RD: {
			throw std::runtime_error("NOT SUPPORTED");
		}
		}
		break;
	}
	case cfg::SCAN: {
		switch (runtime::cur_q_mtd.scheme.enc_t) {
		case encoding::scheme_t::ALP: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::alp_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            X(scan_bdr, cfg::schema, 3),
			                            Buffer(uncompressed, sizeof(double)))); //

			break;
		}
		case encoding::scheme_t::CHIMP: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::chimp_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            X(scan_bdr, cfg::schema, 3),
			                            Buffer(tmp_1, sizeof(double)),
			                            Buffer(tmp_2, sizeof(double)),
			                            Buffer(uncompressed, sizeof(double)) //
			                            ));
			break;
		}
		case encoding::scheme_t::PATAS: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::patas_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            Buffer(tmp_1, sizeof(double)),       //
			                            Buffer(uncompressed, sizeof(double)) //
			                            ));
			break;
		}
		case encoding::scheme_t::CHIMP128: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::chimp128_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            X(scan_bdr, cfg::schema, 3),
			                            X(scan_bdr, cfg::schema, 4),
			                            Buffer(tmp_1, sizeof(double)),
			                            Buffer(tmp_2, sizeof(double)),
			                            Buffer(tmp_3, sizeof(double)),
			                            Buffer(uncompressed, sizeof(double)) //
			                            ));
			break;
		}
		case encoding::scheme_t::GORILLA: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::gorilla_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            X(scan_bdr, cfg::schema, 2),
			                            Buffer(tmp_1, sizeof(double)),       //
			                            Buffer(uncompressed, sizeof(double)) //
			                            ));
			break;
		}
		case encoding::scheme_t::ZSTD: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::zstd,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            Buffer(uncompressed, sizeof(double)) //
			                            ));
			break;
		}
		case encoding::scheme_t::UNCOMPRESSED: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::memcpy_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            Buffer(uncompressed, sizeof(double)) //
			                            ));
			break;
		}
		case encoding::scheme_t::PDE: {
			FixedAggregation(Expression() //
			                     .addOp(alp_end_to_end::function::pde_func,
			                            X(scan_bdr, cfg::schema, 0),
			                            X(scan_bdr, cfg::schema, 1),
			                            Buffer(uncompressed, sizeof(double)) //
			                            ));
			break;
		}
		case encoding::NONE: {
			throw std::runtime_error("NOT SUPPORTED");
		}
		case encoding::CST_BP: {
			throw std::runtime_error("NOT SUPPORTED");
		}
		case encoding::BITPACKED: {
			throw std::runtime_error("NOT SUPPORTED");
		}
		case encoding::ALP_RD: {
			throw std::runtime_error("NOT SUPPORTED");
		}
		}
		break;
	}
	case cfg::SUM_SIMD: {
		throw std::runtime_error("SUM QUERY TYPE IS NOT SUPPORTED!");
	}
	case cfg::COMPRESSION:
		break;
	}

	res->root_op = popOperator();
	assert(operatorStack.size() == 0);
	return res;
}

runtime::Relation alp_q(runtime::Database& db, size_t t_c, size_t vec_sz) {
	using namespace vectorwise;

	runtime::Relation              result;
	vectorwise::SharedStateManager shared;
	WorkerGroup                    workers(t_c);
	GlobalPool                     pool;
	double                         aggr;
	aggr = 0;

	workers.run([&]() {
		alp_q_builder b(db, shared, vec_sz);
		b.previous = this_worker->allocator.setSource(&pool);
		auto query = b.get_q();
		query->root_op->next();
		aggr += (query->aggregator);

		auto leader = barrier();
		if (leader) {
			result.insert("aggr", make_unique<algebra::Integer>());
			auto& sum = result["aggr"].template typedAccessForChange<double>();
			sum.reset(1);
			auto a = aggr;
			sum.push_back(a);
			result.tup_c = 1;
		}
	});

	return result;
}
