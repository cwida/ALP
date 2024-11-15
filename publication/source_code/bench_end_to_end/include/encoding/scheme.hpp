#ifndef ENCODING_HPP
#define ENCODING_HPP

#include <string>
#include <cstdint>

namespace encoding {

enum scheme_t : uint8_t {
	NONE         = 0, //
	UNCOMPRESSED = 1,
	CST_BP,           //
	BITPACKED,
	CHIMP,
	CHIMP128,
	PDE,
	PATAS,
	ALP,
	ALP_RD,
	ZSTD,
	GORILLA,
};

struct scheme {
	std::string name;
	std::string extension;
	std::string prefix;
	uint64_t    c;
	scheme_t    enc_t;
};

inline scheme none_scheme {"NONE", ".none", "none", 1, NONE};          // to be compatible with tectorwise
inline scheme uncompressed_scheme {
    "uncompressed", ".uncompressed", "uncompressed", 2, UNCOMPRESSED}; // to be compatible with tectorwise
inline scheme alp_scheme {"ALP", ".alp", "alp", 4, ALP};
inline scheme pde_scheme {"PDE", ".pde", "pde", 2, PDE};
inline scheme patas_scheme {"PATAS", ".patas", "patas", 3, PATAS};
inline scheme gorilla_scheme {"GORILLA", ".gorilla", "gorilla", 3, GORILLA};
inline scheme alp_rd_scheme {"ALP_RD", ".alp_rd", "alp_rd", 0, ALP_RD};
inline scheme zstd_scheme {"ZSTD", ".zstd", "zstd", 2, ZSTD};
inline scheme bitpacked_scheme {"BITPACKED", "bp", ".bp", 2, BITPACKED};
inline scheme cst_bp_scheme {"CST_BP", ".cbp", "cbp", 1, CST_BP};
inline scheme chimp_scheme {"CHIMP", ".chimp", "chimp", 4, CHIMP};
inline scheme chimp128_scheme {"CHIMP128", ".chimp_128", "chimp_128", 5, CHIMP128};

} // namespace encoding

#endif // ENCODING_HPP
