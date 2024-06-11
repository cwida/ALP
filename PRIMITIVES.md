# ALP Primitives

You can make your own [de]compression API by using ALP primitives. An example of the usage of these can be found in our simple [compression](/include/alp/compressor.hpp) and [decompression](/include/alp/decompressor.hpp) API. The decoding primitives of ALP are auto-vectorized thanks to [FastLanes](https://github.com/cwida/FastLanes). For **benchmarking** purposes, we recommend you use these primitives.

You can use these by including our library in your code: `#include "alp.hpp"` and accessing through the `alp` namespace.

## ALP

### AlpEncode<double|float>::init
```c++
init(double|float* data_column, 
    size_t column_offset, 
    size_t tuples_count, 
    double|float* sample_arr, 
    alp::state& stt)
```
Initializes the algorithm by performing the first-level sampling on the `data_column` buffer and deciding the adequate scheme to use (by default `ALP`). The sampling is performed from the `column_offset` index until `column_offset + ALP_ROUWGROUP_SIZE`. 

### AlpEncode<double|float>::encode
```c++
encode( double|float* input_vector,
        double|float* exceptions,
        uint16_t* exceptions_positions,
        uint16_t* exceptions_count,
        int64_t* encoded_integers, // Encoded integers are always int64
        alp::state& stt)
```
Uses `ALP` to encode the values in `input_vector` into integers using the state (`stt`) `factor` and `exponent`. Encoded values are stored in `encoded_integers` alongside their `exceptions`, their `exceptions_positions` and the `exceptions_count`. Input vector is assumed to point to `ALP_VECTOR_SIZE` (1024) elements. Here, the second-level sampling is performed if necessary. 

### ffor::ffor
```c++
ffor(int64_t|int32_t* in, 
    int64_t|int32_t* out, 
    uint8_t bit_width, 
    int64_t|int32_t* ffor_base)
```
Encode `in` using FFOR (FOR + BP) and writing to `out`. `in` is assumed to point to `ALP_VECTOR_SIZE` (1024) elements. The target `bit_width` and the frame of reference (`ffor_base`) must be given. `alp::analyze_ffor()` primitive can be used to obtain both from an array of integers.


### AlpEncode<double|float>::analyze_ffor
```c++
analyze_ffor(int64_t* input_vector, 
            uint8_t& bit_width, 
            int64_t* ffor_base)
```
Reads values in `input_vector` and set the proper `bit_width` and frame of reference (`ffor_base`) to FOR+bitpack the array. 


### AlpDecode<double|float>::decode
```c++
decode(uint64_t* encoded_integers, 
        uint8_t fac_idx, 
        uint8_t exp_idx, 
        double|float* output)
```
Uses `ALP` to decode the values in `encoded_integers` into `output` using `factor` and `exponent` for the decoding multiplication. The size of the encoded integers array and the output buffer are assumed to be `ALP_VECTOR_SIZE` (1024).

### ffor::unffor
```c++
unffor(int64_t|int32_t* in, 
        int64_t|int32_t* out, 
        uint8_t bit_width, 
        int64_t|int32_t* ffor_base)
```
Decode `in` by reversing the FFOR (FOR + BP) and writing to `out`. `in` is assumed to point to `ALP_VECTOR_SIZE` (1024) elements. The target `bit_width` and the frame of reference (`ffor_base`) must be given. 

### generated::falp::fallback::scalar::falp
```c++
falp(uint64_t* in,
    double* out,
    uint8_t bit_width,
    uint64_t* ffor_base,
    uint8_t factor,
    uint8_t exponent)
```
CURRENTLY ONLY AVAILABLE FOR `double` 

Fused implementation of `decode` and `unffor`. Decode `in` with ALP, reverse the FFOR (FOR + BP) and write to `out`. `in` is assumed to point to `ALP_VECTOR_SIZE` (1024) elements. The target `bit_width`, the frame of reference (`ffor_base`), and the encoding `factor` and `exponent` indexes must be given. 

### AlpDecode<double|float>::patch_exceptions
```c++
patch_exceptions(double|float* output, 
                double|float* exceptions, 
                uint16_t* exceptions_positions, 
                uint16_t* exceptions_count)
```
Patch the exceptions in `output` using their positions and respective count.


## ALP RD
### AlpRD<double|float>::init
```c++
init(double|float* data_column, 
        size_t column_offset, 
        size_t tuples_count, 
        double|float* sample_arr, 
        alp::state& stt)
```
Initializes the algorithm by performing the first-level sampling on the `data_column` buffer. The sampling is performed from the `column_offset` index until `column_offset + ALP_ROUWGROUP_SIZE`. Afterwards, the best position to cut the floating-point values is found and the dictionary to encode the left parts is built and stored in `stt.left_parts_dict`.

### AlpRD<double|float>::encode
```c++
encode(double|float*     input_vector,
        uint16_t*           exceptions,
        uint16_t*           exception_positions,
        uint16_t*           exceptions_count,
        uint64_t|uint32_t*  right_parts,
        uint16_t*           left_parts,
        alp::state&   stt)
```
Uses `ALPRD` to encode the values in `input_vector` into their left and right parts alongside their `exceptions`, their `exceptions_positions` and the `exceptions_count`. Input vector is assumed to point to `ALP_VECTOR_SIZE` (1024) elements. Here, the second-level sampling is performed if necessary. 

### AlpRD<double|float>::decode
```c++
decode(double|float* a_out, 
        uint64_t|uint32_t* unffor_right_arr,  
        uint16_t* unffor_left_arr, 
        uint16_t* exceptions, 
        uint16_t* exceptions_positions, 
        uint16_t* exceptions_count,
        state& stt)
```
Uses `ALP_RD` to decode the values in `unffor_right_arr` and `unffor_left_arr` by glueing them. The size of the encoded integers array and the output buffer are assumed to be `ALP_VECTOR_SIZE` (1024). Exception patching is fused in this function. 

## Using the Primitives

### Rowgroup Level
`init` are primitives that should be called per rowgroup. They set the necessary `state` that other primitives need. All other primitives should be called per vector (1024 values).

### ALP
Encoding is comprised of the `encode`, `analyze_ffor`, and `ffor` primitives. 

Fused decoding is comprised of the `falp` and the `patch_exceptions` primitives. Unfused decoding is comprised of the `unffor`, `decode` and `patch_exceptions` primitives. 


### ALP RD 
Encoding is comprised of `encode` and two calls to `ffor` (for both the left and right parts).

Decoding is comprised of two calls to `unffor` (for both the left and right parts) and the `decode` primitives. 

### Last Vector Encoding
ALP primitives operate on blocks of 1024 values (to easily auto-vectorize). As such, the last vector of a dataset may be incomplete (`vector_size != ALP_VECTOR_SIZE`). A few strategies can be implemented to encode an incomplete vector:   
- Fill the missing values of the vector with the first value or with `0.0`. Pros: Easy to implement and efficient. Cons: Value may be an exception; `0.0` can negatively affect the bitpacking size
- Fill the vector with the first non-exception value after encoding (implemented in our example Compression API). Pros: This will yield the best compression ratio for the last vector. Cons: The vector must be encoded twice.  