#include "common/runtime/Import.hpp"
#include "alp.hpp"
#include "alp_bench.hpp"
#include "benchmarks/alp/config.hpp"
#include "chimp/chimp.hpp"
#include "chimp/chimp128.hpp"
#include "common/runtime/Mmap.hpp"
#include "common/runtime/Types.hpp"
#include "encoding/helper.hpp"
#include "encoding/scheme_pool.hpp"
#include "file_format/file_format.hpp"
#include "gorillas/gorillas.hpp"
#include "patas/patas.hpp"
#include "sys/stat.h"
#include "zstd.h"
#include <filesystem>
#include <fstream>

// ---
#include "btrblocks/scheme/CompressionScheme.hpp"
#include "btrblocks/scheme/SchemePool.hpp"
#include "btrblocks/scheme/double/DoubleBP.hpp"
#include "btrblocks/scheme/double/DynamicDictionary.hpp"
#include "btrblocks/scheme/double/Frequency.hpp"
#include "btrblocks/scheme/double/Pseudodecimal.hpp"
#include "btrblocks/scheme/double/RLE.hpp"
#include "btrblocks/scheme/integer/PBP.hpp"
#include "data.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace btrblocks;

namespace ffor = fastlanes::generated::ffor::fallback::scalar;

using namespace std;

void setupSchemePool() {
	SchemePool::refresh();
	auto& schemes = *SchemePool::available_schemes;

	//	for (auto& scheme : schemes.double_schemes) {
	//		std::cout << ConvertSchemeTypeToString(scheme.first) << std::endl;
	//	}
	//	for (auto& scheme : schemes.integer_schemes) {
	//		std::cout << ConvertSchemeTypeToString(scheme.first) << std::endl;
	//	}

	//	 double: DOUBLE_BP, UNCOMPRESSED,
	for (auto it = schemes.double_schemes.begin(); it != schemes.double_schemes.end();) {
		if (it->first != DoubleSchemeType::DOUBLE_BP       //
		    && it->first != DoubleSchemeType::UNCOMPRESSED //
		) {
			it = schemes.double_schemes.erase(it);
		} else {
			++it;
		}
	}

	//	 int: X_FBP, UNCOMPRESSED,
	for (auto it = schemes.integer_schemes.begin(); it != schemes.integer_schemes.end();) {
		if (it->first != IntegerSchemeType::PFOR            //
		    && it->first != IntegerSchemeType::UNCOMPRESSED //
		                                                    //		    && it->first != IntegerSchemeType::ONE_VALUE //
		) {
			it = schemes.integer_schemes.erase(it);
		} else {
			++it;
		}
	}
}

enum RTType {
	Integer,
	Numeric_12_2,
	Numeric_18_2,
	Date,
	Char_1,
	Char_6,
	Char_7,
	Char_9,
	Char_10,
	Char_11,
	Char_12,
	Char_15,
	Char_18,
	Char_22,
	Char_25,
	Varchar_11,
	Varchar_12,
	Varchar_22,
	Varchar_23,
	Varchar_25,
	Varchar_40,
	Varchar_44,
	Varchar_55,
	Varchar_79,
	Varchar_101,
	Varchar_117,
	Varchar_152,
	Varchar_199,
	Double
};
RTType algebraToRTType(algebra::Type* t) {
	if (dynamic_cast<algebra::Integer*>(t)) {
		return RTType::Integer;
	} else if (auto e = dynamic_cast<algebra::Numeric*>(t)) {
		if (e->size == 12 and e->precision == 2) return Numeric_12_2;
		if (e->size == 18 and e->precision == 2)
			return Numeric_18_2;
		else
			throw runtime_error("Unknown Numeric precision");
	} else if (auto e = dynamic_cast<algebra::Char*>(t)) {
		switch (e->size) {
		case 1:
			return Char_1;
		case 6:
			return Char_6;
		case 7:
			return Char_7;
		case 9:
			return Char_9;
		case 10:
			return Char_10;
		case 11:
			return Char_11;
		case 12:
			return Char_12;
		case 15:
			return Char_15;
		case 18:
			return Char_18;
		case 22:
			return Char_22;
		case 25:
			return Char_25;
		default:
			throw runtime_error("Unknown Char size");
		}
	} else if (auto e = dynamic_cast<algebra::Varchar*>(t)) {
		switch (e->size) {
		case 11:
			return Varchar_11;
		case 12:
			return Varchar_12;
		case 22:
			return Varchar_22;
		case 23:
			return Varchar_23;
		case 25:
			return Varchar_25;
		case 40:
			return Varchar_40;
		case 44:
			return Varchar_44;
		case 55:
			return Varchar_55;
		case 79:
			return Varchar_79;
		case 101:
			return Varchar_101;
		case 117:
			return Varchar_117;
		case 152:
			return Varchar_152;
		case 199:
			return Varchar_199;

		default:
			throw runtime_error("Unknown Varchar size" + std::to_string(e->size));
		}
	} else if (dynamic_cast<algebra::Date*>(t)) {
		return Date;
	} else if (dynamic_cast<algebra::Double*>(t)) {
		return RTType::Double;
	} else {
		throw runtime_error("Unknown type");
	}
}

struct ColumnConfigOwning {
public:
	ColumnConfigOwning(string name, string alias_name, unique_ptr<algebra::Type>&& t, encoding::scheme& scheme)
	    : name(name)
	    , alias_name(alias_name)
	    , type(move(t))
	    , scheme(scheme) {}
	ColumnConfigOwning(ColumnConfigOwning&&) = default;

public:
	std::string                    name;
	std::string                    alias_name;
	std::unique_ptr<algebra::Type> type;
	encoding::scheme&              scheme;
};

struct ColumnConfig {
public:
	ColumnConfig(string name, string alias_name, algebra::Type* t, encoding::scheme& scheme)
	    : name(name)
	    , alias_name(alias_name)
	    , type(t)
	    , scheme(scheme) {}

public:
	std::string       name;
	std::string       alias_name;
	algebra::Type*    type;
	encoding::scheme& scheme;
};

#define COMMA ,
#define EACHTYPE                                                                                                       \
	case Integer:                                                                                                      \
		D(types::Integer)                                                                                              \
	case Double:                                                                                                       \
		D(types::Double)                                                                                               \
	case Numeric_12_2:                                                                                                 \
		D(types::Numeric<12 COMMA 2>)                                                                                  \
	case Numeric_18_2:                                                                                                 \
		D(types::Numeric<18 COMMA 2>)                                                                                  \
	case Date:                                                                                                         \
		D(types::Date)                                                                                                 \
	case Char_1:                                                                                                       \
		D(types::Char<1>)                                                                                              \
	case Char_6:                                                                                                       \
		D(types::Char<6>)                                                                                              \
	case Char_7:                                                                                                       \
		D(types::Char<7>)                                                                                              \
	case Char_9:                                                                                                       \
		D(types::Char<9>)                                                                                              \
	case Char_10:                                                                                                      \
		D(types::Char<10>)                                                                                             \
	case Char_11:                                                                                                      \
		D(types::Char<11>)                                                                                             \
	case Char_12:                                                                                                      \
		D(types::Char<12>)                                                                                             \
	case Char_15:                                                                                                      \
		D(types::Char<15>)                                                                                             \
	case Char_18:                                                                                                      \
		D(types::Char<18>)                                                                                             \
	case Char_22:                                                                                                      \
		D(types::Char<22>)                                                                                             \
	case Char_25:                                                                                                      \
		D(types::Char<25>)                                                                                             \
	case Varchar_11:                                                                                                   \
		D(types::Varchar<11>)                                                                                          \
	case Varchar_12:                                                                                                   \
		D(types::Varchar<12>)                                                                                          \
	case Varchar_22:                                                                                                   \
		D(types::Varchar<22>)                                                                                          \
	case Varchar_23:                                                                                                   \
		D(types::Varchar<23>)                                                                                          \
	case Varchar_25:                                                                                                   \
		D(types::Varchar<25>)                                                                                          \
	case Varchar_40:                                                                                                   \
		D(types::Varchar<40>)                                                                                          \
	case Varchar_44:                                                                                                   \
		D(types::Varchar<44>)                                                                                          \
	case Varchar_55:                                                                                                   \
		D(types::Varchar<55>)                                                                                          \
	case Varchar_79:                                                                                                   \
		D(types::Varchar<79>)                                                                                          \
	case Varchar_101:                                                                                                  \
		D(types::Varchar<101>)                                                                                         \
	case Varchar_117:                                                                                                  \
		D(types::Varchar<117>)                                                                                         \
	case Varchar_152:                                                                                                  \
		D(types::Varchar<152>)                                                                                         \
	case Varchar_199:                                                                                                  \
		D(types::Varchar<199>)

inline void parse(ColumnConfig& c, std::vector<void*>& col, std::string& line, unsigned& begin, unsigned& end) {

	const char* start = line.data() + begin;
	end               = line.find_first_of('|', begin);
	size_t size       = end - begin;

#define D(type)                                                                                                        \
	reinterpret_cast<std::vector<type>&>(col).emplace_back(type::castString(start, size));                             \
	break;

	switch (algebraToRTType(c.type)) { EACHTYPE }
#undef D
	begin = end + 1;
}

void writeBinary(ColumnConfig& col, std::vector<void*>& data, std::string path) {
#define D(type)                                                                                                        \
	{                                                                                                                  \
		auto name = path + "_" + col.name;                                                                             \
		runtime::Vector<type>::write_binary(name.data(), reinterpret_cast<std::vector<type>&>(data));                  \
		break;                                                                                                         \
	}
	switch (algebraToRTType(col.type)) { EACHTYPE }
#undef D
}

void write_bitpacked_binary(ColumnConfig& col, std::vector<void*>& data, std::string path) {
	auto name = path + "_" + col.name;
	runtime::Vector<uint32_t>::write_bitpacked_binary(name.data(), reinterpret_cast<std::vector<uint32_t>&>(data));
}

int64_t write_double(ColumnConfig& col, double* dbl_arr) {
	std::vector<std::string> path_vec = ff::file::get_col_paths(col.alias_name, col.scheme);
	double                   end {0};
	double                   compression_time {0};
	int64_t                  start {0};
	int64_t                  result {0};

	switch (col.scheme.enc_t) {
	case encoding::NONE: {
		runtime::Vector<double>::write_binary(path_vec[0].c_str(), dbl_arr, cfg::t_c);
		break;
	}
	case encoding::CST_BP: {
		throw std::runtime_error("NOT SUPPORTED");
	}
	case encoding::BITPACKED: {
		throw std::runtime_error("NOT SUPPORTED");
	}
	case encoding::CHIMP: {
		uint64_t*                                         uint64_p;
		alp_bench::ChimpCompressionState<uint64_t, false> state;
		uint8_t*                                          data_arr;
		uint8_t*                                          flags_arr;
		uint8_t*                                          leading_zero_arr;
		uint8_t                                           leading_zero_block_count;

		// TODO
		data_arr         = new uint8_t[cfg::t_c * 8];
		flags_arr        = new uint8_t[cfg::t_c * 8];
		leading_zero_arr = new uint8_t[cfg::t_c * 8];

		// Init Encoding

		/* Encode */

		size_t ttl_data_arr_byte_c {0};
		size_t ttl_flags_arr_byte_c {0};
		size_t ttl_leading_zero_arr_byte_c {0};

		std::vector<chimp_m> m_vec;

		auto* data_arr_als         = data_arr;
		auto* flags_arr_als        = flags_arr;
		auto* leading_zero_arr_als = leading_zero_arr;

		start = benchmark::cycleclock::Now();
		;
		for (size_t vec_n {0}; vec_n < cfg::vec_c; ++vec_n) {
			state.Reset();
			state.output.SetStream(data_arr_als);
			state.leading_zero_buffer.SetBuffer(leading_zero_arr_als);
			state.flag_buffer.SetBuffer(flags_arr_als);

			uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
			for (size_t i {0}; i < 1024; ++i) {
				alp_bench::ChimpCompression<uint64_t, false>::Store(uint64_p[i], state);
			}
			state.Flush();
			state.output.Flush();

			size_t cur_data_arr_byte_c(state.output.BytesWritten());
			size_t cur_flags_arr_byte_c(state.flag_buffer.BytesUsed());
			size_t cur_leading_zero_arr_byte_c(state.leading_zero_buffer.BlockCount() * 4);

			chimp_m x(ttl_data_arr_byte_c,
			          ttl_flags_arr_byte_c,
			          ttl_leading_zero_arr_byte_c,
			          state.leading_zero_buffer.BlockCount(),
			          0,
			          0,
			          0,
			          0,
			          vec_n);
			m_vec.push_back(x);

			ttl_data_arr_byte_c += cur_data_arr_byte_c;
			ttl_flags_arr_byte_c += cur_flags_arr_byte_c;
			ttl_leading_zero_arr_byte_c += cur_leading_zero_arr_byte_c;

			data_arr_als += cur_data_arr_byte_c;
			flags_arr_als += cur_flags_arr_byte_c;
			leading_zero_arr_als += cur_leading_zero_arr_byte_c;

			dbl_arr = dbl_arr + 1024;
		}
		end = benchmark::cycleclock::Now();

		runtime::Vector<uint8_t>::write_binary(
		    path_vec[0].c_str(), reinterpret_cast<uint8_t*>(m_vec.data()), sizeof(chimp_m) * m_vec.size());
		runtime::Vector<uint8_t>::write_binary(path_vec[1].c_str(), data_arr, ttl_data_arr_byte_c);
		runtime::Vector<uint8_t>::write_binary(path_vec[2].c_str(), flags_arr, ttl_flags_arr_byte_c);
		runtime::Vector<uint8_t>::write_binary(path_vec[3].c_str(), leading_zero_arr, ttl_leading_zero_arr_byte_c);

		break;
	}
	case encoding::CHIMP128: {
		alp_bench::Chimp128CompressionState<uint64_t, false> com_stt;
		uint64_t*                                            uint64_p;
		std::vector<chimp128_m>                              m_vec;

		// TODO
		uint8_t*  data_arr         = new uint8_t[cfg::t_c * 8];
		uint8_t*  flags_arr        = new uint8_t[cfg::t_c];
		uint8_t*  leading_zero_arr = new uint8_t[cfg::t_c];
		uint16_t* packed_data_arr  = new uint16_t[cfg::t_c];

		size_t ttl_data_arr_byte_c {0};
		size_t ttl_flags_arr_byte_c {0};
		size_t ttl_leading_zero_arr_byte_c {0};
		size_t ttl_packed_data_arr_byte_c {0};

		auto* data_arr_als         = data_arr;
		auto* flags_arr_als        = flags_arr;
		auto* leading_zero_arr_als = leading_zero_arr;
		auto* packed_data_arr_als  = reinterpret_cast<uint8_t*>(packed_data_arr);

		start = benchmark::cycleclock::Now();
		for (size_t vec_n {0}; vec_n < cfg::vec_c; ++vec_n) {
			// Init Encoding
			com_stt.Reset();
			com_stt.output.SetStream(data_arr_als);
			com_stt.leading_zero_buffer.SetBuffer(leading_zero_arr_als);
			com_stt.flag_buffer.SetBuffer(flags_arr_als);
			com_stt.packed_data_buffer.SetBuffer(reinterpret_cast<uint16_t*>(packed_data_arr_als));

			/* Encode */
			uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
			for (size_t i {0}; i < 1024; ++i) {
				alp_bench::Chimp128Compression<uint64_t, false>::Store(uint64_p[i], com_stt);
			}
			com_stt.Flush();
			com_stt.output.Flush();

			size_t cur_data_arr_byte_c(com_stt.output.BytesWritten());
			size_t cur_flags_arr_byte_c(com_stt.flag_buffer.BytesUsed());
			size_t cur_leading_zero_arr_byte_c(com_stt.leading_zero_buffer.BlockCount() * 4);
			size_t cur_packed_data_arr_byte_c(com_stt.packed_data_buffer.index * 2);

			chimp128_m x(ttl_data_arr_byte_c,
			             ttl_flags_arr_byte_c,
			             ttl_leading_zero_arr_byte_c,
			             ttl_packed_data_arr_byte_c,
			             com_stt.leading_zero_buffer.BlockCount(),
			             0,
			             0,
			             0,
			             vec_n);
			m_vec.push_back(x);

			ttl_data_arr_byte_c += cur_data_arr_byte_c;
			ttl_flags_arr_byte_c += cur_flags_arr_byte_c;
			ttl_leading_zero_arr_byte_c += cur_leading_zero_arr_byte_c;
			ttl_packed_data_arr_byte_c += cur_packed_data_arr_byte_c;

			data_arr_als += cur_data_arr_byte_c;
			flags_arr_als += cur_flags_arr_byte_c;
			leading_zero_arr_als += cur_leading_zero_arr_byte_c;
			packed_data_arr_als += cur_packed_data_arr_byte_c;

			dbl_arr = dbl_arr + 1024;
		}
		end = benchmark::cycleclock::Now();

		runtime::Vector<uint8_t>::write_binary(
		    path_vec[0].c_str(), reinterpret_cast<uint8_t*>(m_vec.data()), sizeof(chimp128_m) * m_vec.size());
		runtime::Vector<uint8_t>::write_binary(path_vec[1].c_str(), data_arr, ttl_data_arr_byte_c);
		runtime::Vector<uint8_t>::write_binary(path_vec[2].c_str(), flags_arr, ttl_flags_arr_byte_c);
		runtime::Vector<uint8_t>::write_binary(path_vec[3].c_str(), leading_zero_arr, ttl_leading_zero_arr_byte_c);
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[4].c_str(), reinterpret_cast<uint8_t*>(packed_data_arr), ttl_packed_data_arr_byte_c);

		delete[] data_arr;
		delete[] flags_arr;
		delete[] leading_zero_arr;
		delete[] packed_data_arr;

		break;
	}
	case encoding::PDE: {
		// Init Encoding
		uint8_t*             enc_arr = new uint8_t[cfg::t_c * 8];
		std::vector<chimp_m> m_vec;
		doubles::Decimal     pd;

		size_t ttl_enc_arr_byte_c {0};
		auto*  enc_arr_als = enc_arr;

		start = benchmark::cycleclock::Now();
		for (size_t vec_n {0}; vec_n < cfg::vec_c; ++vec_n) {
			size_t cur_enc_arr_byte_c {0};
			/* Init Encoding */
			size_t cascade = 2;
			size_t size    = 1024;

			DoubleStats stats(dbl_arr, nullptr, size);
			stats = DoubleStats::generateStats(dbl_arr, nullptr, size);

			/* Encode */
			cur_enc_arr_byte_c = pd.compress(dbl_arr, nullptr, enc_arr_als, stats, cascade);

			chimp_m x(ttl_enc_arr_byte_c, cur_enc_arr_byte_c, stats.tuple_count, 0, 0, 0, 0, 0, vec_n);
			m_vec.push_back(x);

			//			std::cout << __FUNCTION__ << " : ZSTD ";
			//			print_zstd(*reinterpret_cast<X*>(&x));

			ttl_enc_arr_byte_c += cur_enc_arr_byte_c;
			enc_arr_als += cur_enc_arr_byte_c;

			dbl_arr = dbl_arr + 1024;
		}
		end = benchmark::cycleclock::Now();

		runtime::Vector<uint8_t>::write_binary(
		    path_vec[0].c_str(), reinterpret_cast<uint8_t*>(m_vec.data()), sizeof(chimp_m) * m_vec.size());
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[1].c_str(), reinterpret_cast<uint8_t*>(enc_arr), ttl_enc_arr_byte_c);

		break;
	}
	case encoding::PATAS: {
		uint64_t*                                                uint64_p;
		alp_bench::patas::PatasCompressionState<uint64_t, false> patas_state;
		uint8_t*                                                 data_arr;
		uint16_t*                                                packed_metadata;

		// TODO
		data_arr        = new uint8_t[cfg::t_c * 8];
		packed_metadata = new uint16_t[cfg::t_c];

		// Init Encoding

		/*
		 * Encode
		 */

		size_t ttl_data_arr_byte_c {0};
		size_t ttl_packed_metadata_byte_c {0};

		std::vector<chimp_m> m_vec;

		auto* data_arr_als        = data_arr;
		auto* packed_metadata_als = reinterpret_cast<uint8_t*>(packed_metadata);

		start = benchmark::cycleclock::Now();
		for (size_t vec_n {0}; vec_n < cfg::vec_c; ++vec_n) {
			// Init Encoding
			patas_state.Reset();
			patas_state.SetOutputBuffer(data_arr_als);
			patas_state.packed_data_buffer.SetBuffer(reinterpret_cast<uint16_t*>(packed_metadata_als));

			/* Encode */
			uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
			for (size_t i {0}; i < 1024; ++i) {
				alp_bench::patas::PatasCompression<uint64_t, false>::Store(uint64_p[i], patas_state);
			}

			size_t cur_data_arr_byte_c(patas_state.byte_writer.BytesWritten());
			size_t cur_packed_metadata_als_c(patas_state.packed_data_buffer.index * 2);

			chimp_m x(ttl_data_arr_byte_c, ttl_packed_metadata_byte_c, 0, 0, 0, 0, 0, 0, vec_n);
			m_vec.push_back(x);

			//			std::cout << __FUNCTION__ << " : PATAS ";
			//			print_patas(*reinterpret_cast<X*>(&x));

			ttl_data_arr_byte_c += cur_data_arr_byte_c;
			ttl_packed_metadata_byte_c += cur_packed_metadata_als_c;

			data_arr_als += cur_data_arr_byte_c;
			packed_metadata_als += cur_packed_metadata_als_c;

			dbl_arr = dbl_arr + 1024;
		}
		end = benchmark::cycleclock::Now();

		runtime::Vector<uint8_t>::write_binary(
		    path_vec[0].c_str(), reinterpret_cast<uint8_t*>(m_vec.data()), sizeof(chimp_m) * m_vec.size());
		runtime::Vector<uint8_t>::write_binary(path_vec[1].c_str(), data_arr, ttl_data_arr_byte_c);
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[2].c_str(), reinterpret_cast<uint8_t*>(packed_metadata), ttl_packed_metadata_byte_c);

		break;
	}
	case encoding::ALP: {
		double*   exc_arr;
		uint16_t* pos_arr;
		uint16_t* exc_c_arr;
		int64_t*  ffor_arr;
		int64_t*  base_arr;
		int64_t*  dig_arr;
		double*   smp_arr;
		uint8_t   bw;
		// vector (if 1 -> no consecutive runs inside vector)

		exc_arr   = new double[cfg::t_c];
		pos_arr   = new uint16_t[cfg::t_c];
		ffor_arr  = new int64_t[cfg::t_c];
		dig_arr   = new int64_t[1024];
		exc_c_arr = new uint16_t[1];
		base_arr  = new int64_t[1];
		smp_arr   = new double[1024];

		size_t ttl_ffor_arr_byte_c {0};
		size_t ttl_exc_byte_c {0};
		size_t ttl_exc_pos_byte_c {0};

		std::vector<alp_m> m_vec;

		size_t tup_c {100 * 1024};
		size_t ofs {0};

		//		alp_debug::print_arr<double>(dbl_arr, 5, std::cout);
		alp::state<double> stt;

		// Init
		alp::encoder<double>::init(dbl_arr, ofs, tup_c, smp_arr, stt);

		//
		auto* ffor_arr_als = reinterpret_cast<uint8_t*>(ffor_arr);
		auto* exc_arr_als  = reinterpret_cast<uint8_t*>(exc_arr);
		auto* pos_arr_als  = reinterpret_cast<uint8_t*>(pos_arr);

		start = benchmark::cycleclock::Now();
		for (size_t vec_n {0}; vec_n < cfg::vec_c; ++vec_n) {
			alp::encoder<double>::encode(dbl_arr,
			                             reinterpret_cast<double*>(exc_arr_als),
			                             reinterpret_cast<uint16_t*>(pos_arr_als),
			                             exc_c_arr,
			                             dig_arr,
			                             stt);

			alp::encoder<double>::analyze_ffor(dig_arr, bw, base_arr);

			ffor::ffor(dig_arr, reinterpret_cast<int64_t*>(ffor_arr_als), bw, base_arr);

			size_t cur_ffor_arr_byte_c(bw * 128);
			size_t cur_exc_byte_c(*exc_c_arr * 8);
			size_t cur_exc_pos_byte_c(*exc_c_arr * 2);

			alp_m x(ttl_ffor_arr_byte_c,
			        ttl_exc_byte_c,
			        ttl_exc_pos_byte_c,
			        bw,
			        *base_arr,
			        stt.fac,
			        stt.exp,
			        *exc_c_arr,
			        vec_n);

			m_vec.push_back(x);

			ttl_ffor_arr_byte_c += cur_ffor_arr_byte_c;
			ttl_exc_byte_c += cur_exc_byte_c;
			ttl_exc_pos_byte_c += cur_exc_pos_byte_c;

			//			std::cout << __FUNCTION__ << " : ALP ";
			//			print_alp(*reinterpret_cast<X*>(&x));
			//
			//			if (vec_n > 258) exit(0);

			//			alp_debug::print_arr<double>(dbl_arr, 1, std::cout);

			ffor_arr_als += cur_ffor_arr_byte_c;
			exc_arr_als += cur_exc_byte_c;
			pos_arr_als += cur_exc_pos_byte_c;

			dbl_arr += 1024;
		}
		end = benchmark::cycleclock::Now();

		//
		//		std::cout << ttl_ffor_arr_byte_c << std::endl;
		//		std::cout << ttl_exc_byte_c << std::endl;
		//		std::cout << ttl_exc_pos_byte_c << std::endl;

		runtime::Vector<uint8_t>::write_binary(
		    path_vec[0].c_str(), reinterpret_cast<uint8_t*>(m_vec.data()), sizeof(X) * m_vec.size());
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[1].c_str(), reinterpret_cast<uint8_t*>(ffor_arr), ttl_ffor_arr_byte_c);
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[2].c_str(), reinterpret_cast<uint8_t*>(exc_arr), ttl_exc_byte_c);
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[3].c_str(), reinterpret_cast<uint8_t*>(pos_arr), ttl_exc_pos_byte_c);

		break;
	}
	case encoding::ZSTD: {
		// Init Encoding

		uint8_t*             enc_arr = new uint8_t[cfg::t_c * 8];
		std::vector<chimp_m> m_vec;

		size_t ttl_enc_arr_byte_c {0};
		auto*  enc_arr_als = enc_arr;

		start = benchmark::cycleclock::Now();
		for (size_t vec_n {0}; vec_n < cfg::vec_c; ++vec_n) {
			auto   rg_n = vec_n / 128;
			auto*  rg_p = dbl_arr + (rg_n * cfg::morsel_c);
			size_t cur_enc_arr_byte_c {0};

			if (vec_n % 128 == 0) {
				/**/
				cur_enc_arr_byte_c =
				    ZSTD_compress(enc_arr_als, cfg::morsel_c * 8, rg_p, cfg::morsel_c * 8, 3); // Level 3
			}
			// Init Encoding

			chimp_m x(ttl_enc_arr_byte_c, cur_enc_arr_byte_c, 0, 0, 0, 0, 0, 0, vec_n);
			m_vec.push_back(x);

			//			std::cout << __FUNCTION__ << " : ZSTD ";
			//			print_zstd(*reinterpret_cast<X*>(&x));

			ttl_enc_arr_byte_c += cur_enc_arr_byte_c;
			enc_arr_als += cur_enc_arr_byte_c;
		}
		end = benchmark::cycleclock::Now();

		runtime::Vector<uint8_t>::write_binary(
		    path_vec[0].c_str(), reinterpret_cast<uint8_t*>(m_vec.data()), sizeof(chimp_m) * m_vec.size());
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[1].c_str(), reinterpret_cast<uint8_t*>(enc_arr), ttl_enc_arr_byte_c);

		break;
	}
	case encoding::ALP_RD: {
		throw std::runtime_error("NOT SUPPORTED!");
	}
	case encoding::GORILLA: {
		uint64_t*                                            uint64_p;
		alp_bench::GorillasCompressionState<uint64_t, false> state;
		uint8_t*                                             data_arr;
		uint8_t*                                             flags_arr;

		// TODO
		data_arr  = new uint8_t[cfg::t_c * 8];
		flags_arr = new uint8_t[cfg::t_c * 8];

		size_t ttl_data_arr_byte_c {0};
		size_t ttl_flag_arr_byte_c {0};

		std::vector<gorilla_m> m_vec;

		auto* data_arr_als  = data_arr;
		auto* flags_arr_als = flags_arr;

		start = benchmark::cycleclock::Now();
		for (size_t vec_n {0}; vec_n < cfg::vec_c; ++vec_n) {
			// Init Encoding
			state.Reset();
			state.output.SetStream(data_arr_als);
			state.flag_buffer.SetBuffer(flags_arr_als);

			/* Encode */
			uint64_p = reinterpret_cast<uint64_t*>(dbl_arr);
			for (size_t i {0}; i < 1024; ++i) {
				alp_bench::GorillasCompression<uint64_t, false>::Store(uint64_p[i], state);
			}

			state.Flush();
			state.output.Flush();

			size_t cur_data_arr_byte_c(state.output.BytesWritten());
			size_t cur_flag_arr_byte_c(state.flag_buffer.BytesUsed());

			gorilla_m x(ttl_data_arr_byte_c, ttl_flag_arr_byte_c, 0, 0, 0, 0, 0, 0, vec_n);
			m_vec.push_back(x);

			ttl_data_arr_byte_c += cur_data_arr_byte_c;
			ttl_flag_arr_byte_c += cur_flag_arr_byte_c;

			data_arr_als += cur_data_arr_byte_c;
			flags_arr_als += cur_flag_arr_byte_c;

			dbl_arr = dbl_arr + 1024;
		}
		end = benchmark::cycleclock::Now();

		runtime::Vector<uint8_t>::write_binary(
		    path_vec[0].c_str(), reinterpret_cast<uint8_t*>(m_vec.data()), sizeof(gorilla_m) * m_vec.size());
		runtime::Vector<uint8_t>::write_binary(path_vec[1].c_str(), data_arr, ttl_data_arr_byte_c);
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[2].c_str(), reinterpret_cast<uint8_t*>(flags_arr), ttl_flag_arr_byte_c);

		break;
	}
	case encoding::UNCOMPRESSED: {
		// Init Encoding
		std::vector<uncompressed_m> m_vec;

		size_t ttl_enc_arr_byte_c {0};

		start = benchmark::cycleclock::Now();
		for (size_t vec_n {0}; vec_n < cfg::vec_c; ++vec_n) {
			size_t cur_enc_arr_byte_c {1024 * 8};

			uncompressed_m x(ttl_enc_arr_byte_c, cur_enc_arr_byte_c, 0, 0, 0, 0, 0, 0, vec_n);
			m_vec.push_back(x);

			ttl_enc_arr_byte_c += cur_enc_arr_byte_c;
		}
		end = benchmark::cycleclock::Now();

		runtime::Vector<uint8_t>::write_binary(
		    path_vec[0].c_str(), reinterpret_cast<uint8_t*>(m_vec.data()), sizeof(uncompressed_m) * m_vec.size());
		runtime::Vector<uint8_t>::write_binary(
		    path_vec[1].c_str(), reinterpret_cast<uint8_t*>(dbl_arr), ttl_enc_arr_byte_c);

		break;
	}
	}

	result = end - start;
	return result;
}

size_t read_double(runtime::Relation& r, ColumnConfig& col) {

	std::vector<std::string> path_vec = ff::file::get_col_paths(col.alias_name, col.scheme);
	size_t                   size {0};

	switch (col.scheme.enc_t) {
	case encoding::NONE: {
		auto& attr = r[col.name];
		auto& data = attr.typedAccessForChange<double>();

		data.read_aligned_binary(path_vec[0]);
		break;
	}
	case encoding::ALP: {
		auto& attr   = r[col.name];
		auto& data_0 = attr.access_0();
		auto& data_1 = attr.access_1();
		auto& data_2 = attr.access_2();
		auto& data_3 = attr.access_3();

		data_0.read_aligned_binary(path_vec[0]);
		data_1.read_aligned_binary(path_vec[1]);
		data_2.read_aligned_binary(path_vec[2]);
		data_3.read_aligned_binary(path_vec[3]);
		break;
	}
	case encoding::CST_BP: {
		throw std::runtime_error("NOT SUPPORTED!");
	}
	case encoding::BITPACKED: {
		throw std::runtime_error("NOT SUPPORTED!");
	}
	case encoding::CHIMP: {
		auto& attr   = r[col.name];
		auto& data_0 = attr.access_0();
		auto& data_1 = attr.access_1();
		auto& data_2 = attr.access_2();
		auto& data_3 = attr.access_3();

		data_0.read_aligned_binary(path_vec[0]);
		data_1.read_aligned_binary(path_vec[1]);
		data_2.read_aligned_binary(path_vec[2]);
		data_3.read_aligned_binary(path_vec[3]);
		break;
	}
	case encoding::CHIMP128: {
		auto& attr   = r[col.name];
		auto& data_0 = attr.access_0();
		auto& data_1 = attr.access_1();
		auto& data_2 = attr.access_2();
		auto& data_3 = attr.access_3();
		auto& data_4 = attr.access_4();

		data_0.read_aligned_binary(path_vec[0]);
		data_1.read_aligned_binary(path_vec[1]);
		data_2.read_aligned_binary(path_vec[2]);
		data_3.read_aligned_binary(path_vec[3]);
		data_4.read_aligned_binary(path_vec[4]);
		break;
	}
	case encoding::PDE: {
		auto& attr   = r[col.name];
		auto& data_0 = attr.access_0();
		auto& data_1 = attr.access_1();

		data_0.read_aligned_binary(path_vec[0]);
		data_1.read_aligned_binary(path_vec[1]);
		break;
	}
	case encoding::PATAS: {
		auto& attr   = r[col.name];
		auto& data_0 = attr.access_0();
		auto& data_1 = attr.access_1();
		auto& data_2 = attr.access_2();

		data_0.read_aligned_binary(path_vec[0]);
		data_1.read_aligned_binary(path_vec[1]);
		data_2.read_aligned_binary(path_vec[2]);
		break;
	}
	case encoding::ZSTD: {
		auto& attr   = r[col.name];
		auto& data_0 = attr.access_0();
		auto& data_1 = attr.access_1();

		data_0.read_aligned_binary(path_vec[0]);
		data_1.read_aligned_binary(path_vec[1]);
		break;
	}
	case encoding::ALP_RD: {
		throw runtime_error(" NOT SUPPORTED!");
	}
	case encoding::GORILLA: {
		auto& attr   = r[col.name];
		auto& data_0 = attr.access_0();
		auto& data_1 = attr.access_1();
		auto& data_2 = attr.access_2();

		data_0.read_aligned_binary(path_vec[0]);
		data_1.read_aligned_binary(path_vec[1]);
		data_2.read_aligned_binary(path_vec[2]);
		break;
	}
	case encoding::UNCOMPRESSED: {
		auto& attr   = r[col.name];
		auto& data_0 = attr.access_0();
		auto& data_1 = attr.access_1();

		data_0.read_aligned_binary(path_vec[0]);
		data_1.read_aligned_binary(path_vec[1]);
		break;
	}
	}

	return cfg::t_c;
}

size_t readBinary(runtime::Relation& r, ColumnConfig& col, std::string path) {
#define D(rt_type)                                                                                                     \
	{                                                                                                                  \
		auto  name = path + "_" + col.name;                                                                            \
		auto& attr = r[col.name];                                                                                      \
		/* attr.name = col.name;   */                                                                                  \
		/*attr.type = col.type;  */                                                                                    \
		auto& data = attr.typedAccessForChange<rt_type>();                                                             \
		data.readBinary(name.data());                                                                                  \
		return data.size();                                                                                            \
	}
	switch (algebraToRTType(col.type)) { EACHTYPE default : throw runtime_error("Unknown type"); }
#undef D
}

size_t read_bitpacked_binary(runtime::Relation& r, ColumnConfig& col, std::string path) {

	auto  name = path + "_" + col.name;
	auto& attr = r[col.name]; /* attr.name = col.name;   */ /*attr.type = col.type;  */
	auto& data = attr.typedAccessForChange<int32_t>();
	data.readBinary(name.data());
	// TODO
	return data.size() * 32 / cfg::bw;
}

double parse_columns(runtime::Relation& r, std::vector<ColumnConfigOwning>& cols) {
	double                    compression_time {0};
	std::vector<ColumnConfig> cols_c;
	setupSchemePool();

	bool all_columns_mmaped = true;

	for (auto& col : cols) {
		//		std::cout << col.name << std::endl;
		//		std::cout << col.alias_name << std::endl;

		cols_c.emplace_back(col.name, col.alias_name, col.type.get(), col.scheme);
		r.insert(col.name, std::move(col.type));
	}

	if (!mkdir((cfg::cached_dir).c_str(), 0777)) {
		throw runtime_error("Could not create dir 'cached': " + cfg::cached_dir);
	}

	//	std::cout << "current_path : \t\t" << std::filesystem::current_path() << std::endl;

	// FLS_CHANGE
	for (auto& col : cols_c) {
		if (!ff::file::contains_col(col.alias_name, col.scheme)) { all_columns_mmaped = false; }
	}

	if (!all_columns_mmaped) {
		std::vector<std::vector<void*>> attributes;
		attributes.assign(cols_c.size(), {});

		std::string relation_file_name;
		switch (cfg::input_t) {
		case cfg::input_t::BINARY: {
			relation_file_name = cfg::data_ext_dir + cols[0].alias_name + ".bin";

			//			std::cout << __FUNCTION__ << " : " << relation_file_name << std::endl;
			//
			//			std::cout << __FUNCTION__ << " : READING BINARY!" << std::endl;

			runtime::Vector<double> data_runvec = runtime::Vector<double>(relation_file_name.c_str());

			//			std::cout << __FUNCTION__ << " : COMPRESSING STARTED!" << std::endl;
			//
			//			std::cout << std::fixed << __FUNCTION__
			//			          << " : RESULT SHOULD BE : " << experiment::sum(data_runvec.data(), cfg::t_c) <<
			// std::endl;
			size_t count = 0;
			for (auto& col : cols_c) {
				compression_time = write_double(col, data_runvec.data());
				count++;
			}
			break;
		}
		case cfg::input_t::CSV: {
			relation_file_name = cfg::data_dir + cols[0].alias_name + ".csv";
			ifstream relation_file(relation_file_name); // TODO
			if (!relation_file.is_open()) { throw runtime_error("tbl file not found: " + relation_file_name); }
			std::cout << __FUNCTION__ << " : " << relation_file_name << std::endl;

			std::cout << __FUNCTION__ << " : PARSING STARTED!" << std::endl;

			string   line;
			unsigned begin = 0, end;
			uint64_t count = 0;
			while (getline(relation_file, line)) {
				count++;
				unsigned i = 0;
				for (auto& col : cols_c) {
					parse(col, attributes[i++], line, begin, end);
				}
				begin = 0;
			}

			std::cout << __FUNCTION__ << " : COMPRESSING STARTED!" << std::endl;
			count = 0;
			for (auto& col : cols_c) {
				write_double(col, reinterpret_cast<double*>(attributes[count].data()));
				count++;
			}
			break;
		}
		}
	}

	size_t ttl_size = ff::file::get_compressed_size(cols_c[0].alias_name, cols_c[0].scheme);
	size_t mtd_size = ff::file::get_mtd_size(cols_c[0].alias_name, cols_c[0].scheme);

	//	std::cout << __FUNCTION__ << " : SIZE :  " << ttl_size << " bytes." << std::endl;
	//	std::cout << __FUNCTION__ << " : SIZE :  " << double(ttl_size * 8) / cfg::t_c << " bits per tuple." <<
	// std::endl; 	std::cout << __FUNCTION__ << " : MTD_SIZE :  " << mtd_size << " bytes." << std::endl; 	std::cout <<
	//__FUNCTION__ << " : MTD_SIZE :  " << double(mtd_size * 8) / cfg::t_c << " bits per tuple."
	//	          << std::endl;
	//	std::cout << ff::file::get_details(cols_c[0].alias_name, cols_c[0].scheme);

	// load mmaped files
	size_t size  = 0;
	size_t diffs = 0;
	for (auto& col : cols_c) {
		auto old_size = size;

		size = read_double(r, col);

		diffs += (old_size != size);
	}

	if (diffs > 1) { throw runtime_error("Columns of " + cols[0].name + " differ in size."); }
	r.tup_c = size;

	return compression_time;
	//	std::cout << __FUNCTION__ << " : FINISHED!" << std::endl;
}

std::vector<ColumnConfigOwning> configX(std::initializer_list<ColumnConfigOwning>&& l) {
	std::vector<ColumnConfigOwning> v;

	for (auto& e : l) {
		v.emplace_back(e.name, e.alias_name, std::move(const_cast<unique_ptr<Type>&>(e.type)), e.scheme);
	}

	return v;
}

namespace runtime {
void importTPCH(std::string dir, Database& db) {
	// NO SUPPORT FOR TPCH
}

void importSSB(std::string dir, Database& db) {
	// NO SUPPORT FOR SSB
}

double import_alp(alp_bench::Column& col, Database& db, encoding::scheme& scheme) {
	auto& rel    = db[cfg::tbl_name];
	rel.name     = col.name;
	auto columns = configX({{cfg::schema, col.name, make_unique<algebra::Double>(), scheme}});

	return parse_columns(rel, columns);
}

} // namespace runtime

namespace experiment {

double sum(double* in, size_t c) {
	double result {0};

	for (size_t i {0}; i < c; ++i) {
		result += in[i];
	}

	return result;
}

void expand_binary_x_times(alp_bench::Column& col, size_t x) {
	/**/

	std::string              col_file_path          = cfg::data_dir + col.name + ".bin";
	std::string              col_extended_file_path = cfg::data_ext_dir + col.name + +".bin";
	runtime::Vector<uint8_t> data_runvec            = runtime::Vector<uint8_t>(col_file_path.c_str());

	//	std::cout << __FUNCTION__ << " : Extension of " << col_file_path << " has been started!" << std::endl;

	auto* double_arr = new double[1024 * 1024 * 1024];
	for (size_t i {0}; i < x; ++i) {
		std::memcpy(double_arr + (i * cfg::morsel_c), data_runvec.data(), cfg::morsel_c * sizeof(double));
	}

	runtime::Vector<uint8_t>::write_binary(
	    col_extended_file_path.c_str(), reinterpret_cast<uint8_t*>(double_arr), cfg::t_c * sizeof(double));

	//	std::cout << __FUNCTION__ << " : " << col_file_path << " has been extended " << x << " times." << std::endl;
}

bool is_expanded(alp_bench::Column& col) {
	/**/
	bool file_exist;
	bool has_correct_size;

	uint64_t sz;

	std::string file_path = cfg::data_ext_dir + col.name + +".bin";

	file_exist = std::filesystem::exists(file_path);
	//	std::cout << std::boolalpha << __FUNCTION__ << " | " << file_path << " | exists | " << file_exist << std::endl;
	if (!file_exist) { return false; }

	sz               = std::filesystem::file_size(file_path);
	has_correct_size = (sz == 1024ULL * 1024 * 1024 * 8);
	//	std::cout << std::boolalpha << __FUNCTION__ << " | " << file_path << " | has correct size | " <<
	// has_correct_size
	//	          << std::endl;

	return has_correct_size;
}

static void remove_file(std::string_view file_path) {
	/**/
	std::filesystem::remove(file_path);

	//	std::cout << __FUNCTION__ << " : " << file_path << " has been removed successfully." << std::endl;
}

void remove_binary_file(alp_bench::Column& col) {
	/**/
	std::string col_extended_file_path = cfg::data_ext_dir + col.name + +".bin";
	remove_file(col_extended_file_path);
}

void clean_compressed_data(alp_bench::Column& col, encoding::scheme& scheme) {
	/**/
	std::vector<std::string> path_vec = ff::file::get_col_paths(col.name, scheme);
	for (auto& file_path : path_vec) {
		remove_file(file_path);
	}
}

} // namespace experiment