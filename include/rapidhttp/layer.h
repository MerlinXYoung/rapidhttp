#pragma once
#include <rapidhttp/layer.hpp>

namespace rapidhttp {

inline int http_method_string_len(http_method m) {
    static constexpr uint8_t method_string_lens[] = {
#define XX(num, name, string) (uint8_t)::strlen(#string),
        HTTP_METHOD_MAP(XX)
#undef XX
    };
    return ELEM_AT(method_string_lens, m, 9 /*<unkonwn>*/);
}

inline http_method get_http_method(const char* s) {
    for (uint32_t i = 0; i < ARRAY_SIZE(method_strings); ++i) {
        if (::strcmp(s, method_strings[i]) == 0) return http_method(i);
    }
    return http_method(0);
}

}  // namespace rapidhttp
