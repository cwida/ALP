#pragma once

#include "bit_reader.hpp"
#include "chimp_utils.hpp"
#include "duckdb/duckdb.h"
#include "duckdb/exception.hpp"
#include "duckdb/fast_mem.hpp"
#include "duckdb/likely.hpp"
#include "duckdb/limits.hpp"
#include "flag_buffer.hpp"
#include "leading_zero_buffer.hpp"
#include "output_bit_stream.hpp"

namespace alp_bench {

//===--------------------------------------------------------------------===//
// Compression
//===--------------------------------------------------------------------===//

template <class CHIMP_TYPE, bool EMPTY>
struct ChimpCompressionState {

	ChimpCompressionState()
	    : previous_leading_zeros(NumericLimits<uint8_t>::Maximum()) {
		previous_value = 0;
	}

	inline void SetLeadingZeros(int32_t value = NumericLimits<uint8_t>::Maximum()) {
		this->previous_leading_zeros = value;
	}

	void Flush() { leading_zero_buffer.Flush(); }

	// Reset the state
	void Reset() {
		first = true;
		SetLeadingZeros();
		leading_zero_buffer.Reset();
		flag_buffer.Reset();
		previous_value = 0;
	}

	CHIMP_TYPE BitsWritten() const {
		return output.BitsWritten() + leading_zero_buffer.BitsWritten() + flag_buffer.BitsWritten();
	}

	OutputBitStream<EMPTY>   output; // The stream to write to
	LeadingZeroBuffer<EMPTY> leading_zero_buffer;
	FlagBuffer<EMPTY>        flag_buffer;
	uint8_t                  previous_leading_zeros; //! The leading zeros of the reference value
	CHIMP_TYPE               previous_value = 0;
	bool                     first          = true;
};

template <class CHIMP_TYPE, bool EMPTY>
class ChimpCompression {
public:
	using State = ChimpCompressionState<CHIMP_TYPE, EMPTY>;

	//! The amount of bits needed to store an index between 0-127
	static constexpr uint8_t INDEX_BITS_SIZE = 7;
	static constexpr uint8_t SIGNIFICANT_BITS_SIZE =
	    6; // The amount needed to store the maximum number of significant bits (0-63)
	static constexpr uint8_t BIT_SIZE = sizeof(CHIMP_TYPE) * 8;

	static constexpr uint8_t TRAILING_ZERO_THRESHOLD = SignificantBits<CHIMP_TYPE>::size + INDEX_BITS_SIZE;

	static void Store(CHIMP_TYPE in, State& state) {
		if (state.first) {
			WriteFirst(in, state);
		} else {
			CompressValue(in, state);
		}
	}

	//! Write the content of the bit buffer to the stream
	static void Flush(State& state) {
		if (!EMPTY) { state.output.Flush(); }
	}

	static void WriteFirst(CHIMP_TYPE in, State& state) {
		state.output.template WriteValue<CHIMP_TYPE, BIT_SIZE>(in);
		state.previous_value = in;
		state.first          = false;
	}

	static void CompressValue(CHIMP_TYPE in, State& state) {

		CHIMP_TYPE xor_result;
		uint8_t    previous_index;
		uint32_t   trailing_zeros                  = 0;
		bool       trailing_zeros_exceed_threshold = false;

		xor_result = (CHIMP_TYPE)in ^ state.previous_value;

		// Compress the value
		if (xor_result == 0) {
			state.flag_buffer.Insert(ChimpConstants::Flags::VALUE_IDENTICAL);
			// state.output.template WriteValue<uint8_t, INDEX_BITS_SIZE>(previous_index);
			state.SetLeadingZeros();
		} else {
			// Values are not identical
			auto    leading_zeros_raw = CountZeros<CHIMP_TYPE>::Leading(xor_result);
			uint8_t leading_zeros     = ChimpConstants::Compression::LEADING_ROUND[leading_zeros_raw];

			trailing_zeros                  = CountZeros<CHIMP_TYPE>::Trailing(xor_result);
			trailing_zeros_exceed_threshold = trailing_zeros > 6;

			if (trailing_zeros_exceed_threshold) {
				uint32_t significant_bits = BIT_SIZE - leading_zeros - trailing_zeros;
				state.flag_buffer.Insert(ChimpConstants::Flags::TRAILING_EXCEEDS_THRESHOLD);
				state.leading_zero_buffer.Insert(ChimpConstants::Compression::LEADING_REPRESENTATION[leading_zeros]);
				state.output.template WriteValue<uint8_t, SIGNIFICANT_BITS_SIZE>(significant_bits);
				state.output.template WriteValue<CHIMP_TYPE>(xor_result >> trailing_zeros, significant_bits);
				state.SetLeadingZeros();
			} else if (leading_zeros == state.previous_leading_zeros) {
				state.flag_buffer.Insert(ChimpConstants::Flags::LEADING_ZERO_EQUALITY);
				int32_t significant_bits = BIT_SIZE - leading_zeros;
				state.output.template WriteValue<CHIMP_TYPE>(xor_result, significant_bits);
			} else {
				state.flag_buffer.Insert(ChimpConstants::Flags::LEADING_ZERO_LOAD);
				const int32_t significant_bits = BIT_SIZE - leading_zeros;
				state.leading_zero_buffer.Insert(ChimpConstants::Compression::LEADING_REPRESENTATION[leading_zeros]);
				state.output.template WriteValue<CHIMP_TYPE>(xor_result, significant_bits);
				state.SetLeadingZeros(leading_zeros);
			}
		}
		state.previous_value = in;
	}
};

//===--------------------------------------------------------------------===//
// Decompression
//===--------------------------------------------------------------------===//

template <class CHIMP_TYPE>
struct ChimpDecompressionState {
public:
	ChimpDecompressionState()
	    : reference_value(0)
	    , first(true) {
		ResetZeros();
	}

	void Reset() {
		ResetZeros();
		reference_value = 0;
		first           = true;
	}

	inline void ResetZeros() {
		leading_zeros  = NumericLimits<uint8_t>::Maximum();
		trailing_zeros = 0;
	}

	inline void SetLeadingZeros(uint8_t value) { leading_zeros = value; }

	inline void SetTrailingZeros(uint8_t value) {
		D_ASSERT(value <= sizeof(CHIMP_TYPE) * 8);
		trailing_zeros = value;
	}

	uint8_t LeadingZeros() const { return leading_zeros; }
	uint8_t TrailingZeros() const { return trailing_zeros; }

	BitReader  input;
	uint8_t    leading_zeros;
	uint8_t    trailing_zeros;
	CHIMP_TYPE reference_value = 0;

	bool first;
};

template <class CHIMP_TYPE>
struct ChimpDecompression {
public:
	using DecompressState = ChimpDecompressionState<CHIMP_TYPE>;

	static constexpr uint8_t INDEX_BITS_SIZE       = 7;
	static constexpr uint8_t BIT_SIZE              = sizeof(CHIMP_TYPE) * 8;
	static constexpr uint8_t SIGNIFICANT_BITS_SIZE = 6;

	static inline CHIMP_TYPE
	Load(ChimpConstants::Flags flag, uint8_t leading_zeros[], uint32_t& leading_zero_index, DecompressState& state) {
		if (DUCKDB_UNLIKELY(state.first)) {
			return LoadFirst(state);
		} else {
			return DecompressValue(flag, leading_zeros, leading_zero_index, state);
		}
	}

	static inline CHIMP_TYPE LoadFirst(DecompressState& state) {
		CHIMP_TYPE result     = state.input.template ReadValue<CHIMP_TYPE, sizeof(CHIMP_TYPE) * 8>();
		state.first           = false;
		state.reference_value = result;
		return result;
	}

	static inline CHIMP_TYPE DecompressValue(ChimpConstants::Flags flag,
	                                         uint8_t               leading_zeros[],
	                                         uint32_t&             leading_zero_index,
	                                         DecompressState&      state) {
		CHIMP_TYPE result;
		switch (flag) {
		case ChimpConstants::Flags::VALUE_IDENTICAL: {
			//! Value is identical to previous value
			result = state.reference_value;
			break;
		}
		case ChimpConstants::Flags::TRAILING_EXCEEDS_THRESHOLD: {
			state.leading_zeros   = leading_zeros[leading_zero_index++];
			auto significant_bits = state.input.template ReadValue<uint8_t>(SIGNIFICANT_BITS_SIZE);
			state.trailing_zeros  = BIT_SIZE - significant_bits - state.leading_zeros;
			result                = state.input.template ReadValue<CHIMP_TYPE>(significant_bits);
			result <<= state.trailing_zeros;
			result ^= state.reference_value;
			break;
		}
		case ChimpConstants::Flags::LEADING_ZERO_EQUALITY: {
			result = state.input.template ReadValue<CHIMP_TYPE>(BIT_SIZE - state.leading_zeros);
			result ^= state.reference_value;
			break;
		}
		case ChimpConstants::Flags::LEADING_ZERO_LOAD: {
			state.leading_zeros = leading_zeros[leading_zero_index++];
			D_ASSERT(state.leading_zeros <= BIT_SIZE);
			result = state.input.template ReadValue<CHIMP_TYPE>(BIT_SIZE - state.leading_zeros);
			result ^= state.reference_value;
			break;
		}
		default:
			// std::cout << "Chimp compression flag with value not recognized ";
			// std::cout << flag;
			break;
			// throw InternalException("Chimp compression flag with value %d not recognized", flag);
		}
		state.reference_value = result;
		return result;
	}
};

} // namespace alp_bench
