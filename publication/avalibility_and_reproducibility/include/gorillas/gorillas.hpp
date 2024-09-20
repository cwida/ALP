#pragma once
#include "chimp/bit_reader.hpp"
#include "chimp/flag_buffer.hpp"
#include "chimp/leading_zero_buffer.hpp"
#include "chimp/output_bit_stream.hpp"
#include "duckdb/duckdb.h"
#include "duckdb/exception.hpp"
#include "duckdb/fast_mem.hpp"
#include "duckdb/likely.hpp"
#include "duckdb/limits.hpp"
#include "gorillas/gorillas_utils.hpp"
#include <iostream>

namespace alp_bench {

template <class CHIMP_TYPE, bool EMPTY>
struct GorillasCompressionState {

	GorillasCompressionState()
	    : previous_leading_zeros(NumericLimits<uint8_t>::Maximum())
	    , previous_trailing_zeros(0) {
		previous_value = 0;
	}

	inline void SetLeadingZeros(int8_t value = NumericLimits<uint8_t>::Maximum()) {
		this->previous_leading_zeros = value;
	}

	inline void SetTrailingZeros(int8_t value = 0) { this->previous_trailing_zeros = value; }

	void Flush() {
		//
	}

	// Reset the state
	void Reset() {
		first = true;
		SetLeadingZeros();
		SetTrailingZeros();
		flag_buffer.Reset();
		previous_value = 0;
	}

	CHIMP_TYPE BitsWritten() const { return output.BitsWritten() + flag_buffer.BitsWritten(); }

	OutputBitStream<EMPTY> output; // The stream to write to
	FlagBuffer<EMPTY>      flag_buffer;
	uint8_t                previous_leading_zeros; //! The leading zeros of the reference value
	uint8_t                previous_trailing_zeros;
	CHIMP_TYPE             previous_value = 0;
	bool                   first          = true;
};

template <class CHIMP_TYPE, bool EMPTY>
class GorillasCompression {
public:
	using State = GorillasCompressionState<CHIMP_TYPE, EMPTY>;

	//! The amount of bits needed to store an index between 0-127
	static constexpr uint8_t SIGNIFICANT_BITS_SIZE =
	    6; // The amount needed to store the maximum number of significant bits (0-63)
	static constexpr uint8_t LEADING_ZEROS_BITS_SIZE = 5;
	static constexpr uint8_t BIT_SIZE                = sizeof(CHIMP_TYPE) * 8;

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
		xor_result = (CHIMP_TYPE)in ^ state.previous_value;

		// Compress the value
		if (xor_result == 0) {
			state.flag_buffer.Insert(GorillasConstants::Flags::VALUE_IDENTICAL);
		} else { // Values are not identical

			uint8_t leading_zeros = CountZeros<CHIMP_TYPE>::Leading(xor_result);
			if (leading_zeros >= 32) { // To prevent overflow
				leading_zeros = 31;
			}

			uint8_t trailing_zeros = CountZeros<CHIMP_TYPE>::Trailing(xor_result);

			if (leading_zeros >= state.previous_leading_zeros && trailing_zeros >= state.previous_trailing_zeros) {
				state.flag_buffer.Insert(alp_bench::GorillasConstants::Flags::LEADING_HIGHER_OR_EQUAL);
				uint32_t significant_bits = BIT_SIZE - state.previous_leading_zeros - state.previous_trailing_zeros;
				state.output.template WriteValue<CHIMP_TYPE>(xor_result >> state.previous_trailing_zeros,
				                                             significant_bits);
			} else {
				state.flag_buffer.Insert(alp_bench::GorillasConstants::Flags::LEADING_LOWER);
				uint32_t significant_bits = BIT_SIZE - leading_zeros - trailing_zeros;

				state.output.template WriteValue<uint8_t, LEADING_ZEROS_BITS_SIZE>(leading_zeros);

				state.output.template WriteValue<uint8_t, SIGNIFICANT_BITS_SIZE>(significant_bits - 1);
				state.output.template WriteValue<CHIMP_TYPE>(xor_result >> trailing_zeros, significant_bits);
				state.SetLeadingZeros(leading_zeros);
				state.SetTrailingZeros(trailing_zeros);
			}
		}
		state.previous_value = in;
	}
};

//===--------------------------------------------------------------------===//
// Decompression
//===--------------------------------------------------------------------===//

template <class CHIMP_TYPE>
struct GorillasDecompressionState {
public:
	GorillasDecompressionState()
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
struct GorillasDecompression {
public:
	using DecompressState = GorillasDecompressionState<CHIMP_TYPE>;

	static constexpr uint8_t BIT_SIZE                = sizeof(CHIMP_TYPE) * 8;
	static constexpr uint8_t SIGNIFICANT_BITS_SIZE   = 6;
	static constexpr uint8_t LEADING_ZEROS_BITS_SIZE = 5;

	static inline CHIMP_TYPE Load(GorillasConstants::Flags flag, DecompressState& state) {
		if (DUCKDB_UNLIKELY(state.first)) {
			return LoadFirst(state);
		} else {
			return DecompressValue(flag, state);
		}
	}

	static inline CHIMP_TYPE LoadFirst(DecompressState& state) {
		CHIMP_TYPE result     = state.input.template ReadValue<CHIMP_TYPE, sizeof(CHIMP_TYPE) * 8>();
		state.first           = false;
		state.reference_value = result;
		return result;
	}

	static inline CHIMP_TYPE DecompressValue(GorillasConstants::Flags flag, DecompressState& state) {
		CHIMP_TYPE result;
		switch (flag) {
		case GorillasConstants::Flags::VALUE_IDENTICAL: {
			//! Value is identical to previous value
			result = state.reference_value;
			break;
		}
		case GorillasConstants::Flags::LEADING_LOWER: {
			// state.leading_zeros = leading_zeros[leading_zero_index++]; // comment/uncomment here
			state.leading_zeros = state.input.template ReadValue<uint8_t>(LEADING_ZEROS_BITS_SIZE);

			auto significant_bits = state.input.template ReadValue<uint8_t>(SIGNIFICANT_BITS_SIZE) + 1;
			state.trailing_zeros  = BIT_SIZE - significant_bits - state.leading_zeros;
			result = state.input.template ReadValue<CHIMP_TYPE>(BIT_SIZE - state.leading_zeros - state.trailing_zeros);
			result <<= state.trailing_zeros;
			result ^= state.reference_value;
			break;
		}
		case GorillasConstants::Flags::LEADING_HIGHER_OR_EQUAL: {
			result = state.input.template ReadValue<CHIMP_TYPE>(BIT_SIZE - state.leading_zeros - state.trailing_zeros);
			result <<= state.trailing_zeros;
			result ^= state.reference_value;
			break;
		}
		default:
			// std::cout << "Gorillas compression flag with value not recognized ";
			// std::cout << flag;
			break;
			// throw InternalException("Gorillas compression flag with value %d not recognized", flag);
		}
		state.reference_value = result;
		return result;
	}
};

} // namespace alp_bench
