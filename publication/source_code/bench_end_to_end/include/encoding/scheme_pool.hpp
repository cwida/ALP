#ifndef ENCODINGS_H
#define ENCODINGS_H

#include "scheme.hpp"
#include <vector>

namespace encoding {

inline std::vector<scheme> scheme_vec {
    alp_scheme,
    patas_scheme,
    chimp128_scheme,
    chimp_scheme, /**/
    gorilla_scheme,
    zstd_scheme,
    uncompressed_scheme,
    pde_scheme,
};

} // namespace encoding
#endif // ENCODINGS_H
