//
// Created by Azim Afroozeh on 05/07/2023.
//

#ifndef ALP_HELPER_HPP
#define ALP_HELPER_HPP

#include "cstdint"
#include "iostream"

struct X {
	explicit X(
	    uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint16_t f, uint16_t g, uint16_t h, uint16_t i)
	    : a(a)
	    , b(b)
	    , c(c)
	    , d(d)
	    , e(e)
	    , f(f)
	    , g(g)
	    , h(h)
	    , i(i) {}

	uint64_t a;
	uint64_t b;
	uint64_t c;
	uint64_t d;
	int64_t  e;
	uint16_t f;
	uint16_t g;
	uint16_t h;
	uint16_t i;
};
static_assert(sizeof(X) == cfg::m_sz);

struct alp_m {
	alp_m() {};
	explicit alp_m(uint64_t ffor_arr_byte_c,
	               uint64_t exc_byte_c,
	               uint64_t exc_pos_byte_c,
	               uint64_t bw,
	               int64_t  base,
	               uint16_t fac,
	               uint16_t exp,
	               uint16_t exc_c,
	               uint16_t vec_n)
	    : ffor_arr_byte_c(ffor_arr_byte_c)
	    , exc_byte_c(exc_byte_c)
	    , exc_pos_byte_c(exc_pos_byte_c)
	    , bw(bw)
	    , base(base)
	    , exp(exp)
	    , fac(fac)
	    , exc_c(exc_c)
	    , vec_n(vec_n) {}

	uint64_t ffor_arr_byte_c;
	uint64_t exc_byte_c;
	uint64_t exc_pos_byte_c;
	uint64_t bw;
	int64_t  base;
	uint16_t exp;
	uint16_t fac;
	uint16_t exc_c;
	uint16_t vec_n;
};
static_assert(sizeof(alp_m) == cfg::m_sz);
inline void print_alp(X x) {
	std::cout << "ffor_arr_byte_c :  " << x.a << " | " //
	          << "exc_byte_c:  " << x.b << " | "       //
	          << "exc_pos_byte_c: " << x.c << " | "    //
	          << "bw: " << x.d << " | "                //
	          << "base: " << x.e << " | "              //
	          << "exp: " << x.f << " | "               //
	          << "fac: " << x.g << " | "               //
	          << "exc_c: " << x.h << " | "             //
	          << "vec_n: " << x.i << " | "             //

	          << "\n";
}

struct chimp_m {
	chimp_m() {};
	explicit chimp_m(uint64_t data_arr_byte_c,
	                 uint64_t flags_arr_byte_c,
	                 uint64_t leading_zero_arr_byte_c,
	                 uint64_t d,
	                 uint64_t e,
	                 uint16_t f,
	                 uint16_t g,
	                 uint16_t h,
	                 uint16_t vec_n)
	    : data_arr_byte_c(data_arr_byte_c)
	    , flags_arr_byte_c(flags_arr_byte_c)
	    , leading_zero_arr_byte_c(leading_zero_arr_byte_c)
	    , d(d)
	    , e(e)
	    , f(f)
	    , g(g)
	    , h(h)
	    , vec_n(vec_n) {}

	uint64_t data_arr_byte_c;
	uint64_t flags_arr_byte_c;
	uint64_t leading_zero_arr_byte_c;
	uint64_t d;
	int64_t  e;
	uint16_t f;
	uint16_t g;
	uint16_t h;
	uint16_t vec_n;
};
static_assert(sizeof(chimp_m) == cfg::m_sz);
inline void print_chimp(X x) {
	std::cout << "data_arr_byte_c :  " << x.a << " | "       //
	          << "flags_arr_byte_c:  " << x.b << " | "       //
	          << "leading_zero_arr_byte_c: " << x.c << " | " //
	          << "NOTHING: " << x.d << " | "                 //
	          << "NOTHING: " << x.e << " | "                 //
	          << "NOTHING: " << x.f << " | "                 //
	          << "NOTHING: " << x.g << " | "                 //
	          << "NOTHING: " << x.h << " | "                 //
	          << "vec_n: " << x.i << " | "                   //

	          << "\n";
}

struct patas_m {
	patas_m() {};
	explicit patas_m(uint64_t data_arr_byte_c,
	                 uint64_t packed_data_arr_byte_c,
	                 uint64_t c,
	                 uint64_t d,
	                 uint64_t e,
	                 uint16_t f,
	                 uint16_t g,
	                 uint16_t h,
	                 uint16_t i)
	    : data_arr_byte_c(data_arr_byte_c)
	    , packed_data_arr_byte_c(packed_data_arr_byte_c)
	    , c(c)
	    , d(d)
	    , e(e)
	    , f(f)
	    , g(g)
	    , h(h)
	    , i(i) {}

	uint64_t data_arr_byte_c;
	uint64_t packed_data_arr_byte_c;
	uint64_t c;
	uint64_t d;
	int64_t  e;
	uint16_t f;
	uint16_t g;
	uint16_t h;
	uint16_t i;
};
inline void print_patas(X x) {
	std::cout << "data_arr_byte_c :  " << x.a << " | "       //
	          << "packed_metadata_byte_c:  " << x.b << " | " //
	          << "NOTHING: " << x.c << " | "                 //
	          << "NOTHING: " << x.d << " | "                 //
	          << "NOTHING: " << x.e << " | "                 //
	          << "NOTHING: " << x.f << " | "                 //
	          << "NOTHING: " << x.g << " | "                 //
	          << "NOTHING: " << x.h << " | "                 //
	          << "vec_n: " << x.i << " | "                   //

	          << "\n";
}
static_assert(sizeof(patas_m) == cfg::m_sz);

struct gorilla_m {
	gorilla_m() {};
	explicit gorilla_m(uint64_t data_arr_byte_c,
	                   uint64_t flag_buffer_byte_c,
	                   uint64_t c,
	                   uint64_t d,
	                   uint64_t e,
	                   uint16_t f,
	                   uint16_t g,
	                   uint16_t h,
	                   uint16_t vec_n)
	    : data_arr_byte_c(data_arr_byte_c)
	    , flag_buffer_byte_c(flag_buffer_byte_c)
	    , c(c)
	    , d(d)
	    , e(e)
	    , f(f)
	    , g(g)
	    , h(h)
	    , vec_n(vec_n) {}

	uint64_t data_arr_byte_c;
	uint64_t flag_buffer_byte_c;
	uint64_t c;
	uint64_t d;
	int64_t  e;
	uint16_t f;
	uint16_t g;
	uint16_t h;
	uint16_t vec_n;
};
inline void print_gorilla(X x) {
	std::cout << "data_arr_byte_c :  " << x.a << " | "   //
	          << "flag_buffer_byte_c:  " << x.b << " | " //
	          << "NOTHING: " << x.c << " | "             //
	          << "NOTHING: " << x.d << " | "             //
	          << "NOTHING: " << x.e << " | "             //
	          << "NOTHING: " << x.f << " | "             //
	          << "NOTHING: " << x.g << " | "             //
	          << "NOTHING: " << x.h << " | "             //
	          << "vec_n: " << x.i << " | "               //

	          << "\n";
}
static_assert(sizeof(gorilla_m) == cfg::m_sz);

struct chimp128_m {
	chimp128_m() {};
	explicit chimp128_m(uint64_t data_arr_byte_c,
	                    uint64_t flags_arr_byte_c,
	                    uint64_t leading_zero_arr_byte_c,
	                    uint64_t packed_data_arr_byte_c,
	                    int64_t  e,
	                    uint16_t f,
	                    uint16_t g,
	                    uint16_t h,
	                    uint16_t vec_n)
	    : data_arr_byte_c(data_arr_byte_c)
	    , flags_arr_byte_c(flags_arr_byte_c)
	    , leading_zero_arr_byte_c(leading_zero_arr_byte_c)
	    , packed_data_arr_byte_c(packed_data_arr_byte_c)
	    , e(e)
	    , f(f)
	    , g(g)
	    , h(h)
	    , vec_n(vec_n) {}

	uint64_t data_arr_byte_c;
	uint64_t flags_arr_byte_c;
	uint64_t leading_zero_arr_byte_c;
	uint64_t packed_data_arr_byte_c;
	int64_t  e;
	uint16_t f;
	uint16_t g;
	uint16_t h;
	uint16_t vec_n;
};
static_assert(sizeof(chimp128_m) == cfg::m_sz);
inline void print_chimp128(X x) {
	std::cout << "data_arr_byte_c :  " << x.a << " | "       //
	          << "flags_arr_byte_c:  " << x.b << " | "       //
	          << "leading_zero_arr_byte_c: " << x.c << " | " //
	          << "packed_data_arr_byte: " << x.d << " | "    //
	          << "NOTHING: " << x.e << " | "                 //
	          << "NOTHING: " << x.f << " | "                 //
	          << "NOTHING: " << x.g << " | "                 //
	          << "NOTHING: " << x.h << " | "                 //
	          << "vec_n: " << x.i << " | "                   //

	          << "\n";
}

struct zstd_m {
	zstd_m() {};
	explicit zstd_m(uint64_t enc_arr_byte_c,
	                uint64_t enc_size,
	                uint64_t c,
	                uint64_t d,
	                int64_t  e,
	                uint16_t f,
	                uint16_t g,
	                uint16_t h,
	                uint16_t vec_n)
	    : enc_arr_byte_c(enc_arr_byte_c)
	    , enc_size(enc_size)
	    , c(c)
	    , d(d)
	    , e(e)
	    , f(f)
	    , g(g)
	    , h(h)
	    , vec_n(vec_n) {}

	uint64_t enc_arr_byte_c;
	uint64_t enc_size;
	uint64_t c;
	uint64_t d;
	int64_t  e;
	uint16_t f;
	uint16_t g;
	uint16_t h;
	uint16_t vec_n;
};
static_assert(sizeof(zstd_m) == cfg::m_sz);
inline void print_zstd(X x) {
	std::cout << "enc_arr_byte_c :  " << x.a << " | " //
	          << "enc_size:  " << x.b << " | "        //
	          << "NOTHING: " << x.c << " | "          //
	          << "NOTHING: " << x.d << " | "          //
	          << "NOTHING: " << x.e << " | "          //
	          << "NOTHING: " << x.f << " | "          //
	          << "NOTHING: " << x.g << " | "          //
	          << "NOTHING: " << x.h << " | "          //
	          << "vec_n: " << x.i << " | "            //

	          << "\n";
}

struct uncompressed_m {
	uncompressed_m() {};
	explicit uncompressed_m(uint64_t enc_arr_byte_c,
	                      uint64_t b,
	                      uint64_t c,
	                      uint64_t d,
	                      int64_t  e,
	                      uint16_t f,
	                      uint16_t g,
	                      uint16_t h,
	                      uint16_t vec_n)
	    : enc_arr_byte_c(enc_arr_byte_c)
	    , b(b)
	    , c(c)
	    , d(d)
	    , e(e)
	    , f(f)
	    , g(g)
	    , h(h)
	    , vec_n(vec_n) {}

	uint64_t enc_arr_byte_c;
	uint64_t b;
	uint64_t c;
	uint64_t d;
	int64_t  e;
	uint16_t f;
	uint16_t g;
	uint16_t h;
	uint16_t vec_n;
};
static_assert(sizeof(zstd_m) == cfg::m_sz);
inline void print_uncompressed(X x) {
	std::cout << "enc_arr_byte_c :  " << x.a << " | " //
	          << "NOTHING:  " << x.b << " | "         //
	          << "NOTHING: " << x.c << " | "          //
	          << "NOTHING: " << x.d << " | "          //
	          << "NOTHING: " << x.e << " | "          //
	          << "NOTHING: " << x.f << " | "          //
	          << "NOTHING: " << x.g << " | "          //
	          << "NOTHING: " << x.h << " | "          //
	          << "vec_n: " << x.i << " | "            //

	          << "\n";
}
#endif // ALP_HELPER_HPP
