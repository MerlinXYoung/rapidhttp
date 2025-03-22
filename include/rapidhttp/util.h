#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "error_code.h"

namespace rapidhttp {

inline size_t UIntegerByteSize(uint32_t i) noexcept {
    if (i < 10)
        return 1;
    else if (i < 100)
        return 2;
    else if (i < 1000)
        return 3;
    else if (i < 10000)
        return 4;
    else if (i < 100000)
        return 5;
    else if (i < 1000000)
        return 6;
    else if (i < 10000000)
        return 7;
    else if (i < 100000000)
        return 8;
    else if (i < 1000000000)
        return 9;
    else
        return 10;
}

inline const char* SkipSpaces(const char* pos, const char* last) noexcept {
    for (; pos < last && *pos == ' '; ++pos);
    return pos;
}

inline const char* FindSpaces(const char* pos, const char* last) noexcept {
    for (; pos < last && *pos != ' '; ++pos);
    if (pos == last) return nullptr;
    return *pos == ' ' ? pos : nullptr;
}
inline const char* FindCRLF(const char* pos, const char* last, std::error_code& ec) noexcept {
    for (; pos < last - 1; ++pos) {
        if (*pos == '\r') {
            if (*(pos + 1) == '\n') {
                return pos;
            } else {
                ec = MakeErrorCode(eErrorCode::parse_error);
                return nullptr;
            }
        } else if (*pos == '\n') {
            ec = MakeErrorCode(eErrorCode::parse_error);
            return nullptr;
        }
    }

    return nullptr;
}

}  // namespace rapidhttp
