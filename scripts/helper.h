namespace detail {
constexpr size_t constLength(const char* str) { return (*str == 0) ? 0 : constLength(str + 1) + 1; }
}  // namespace detail
inline int http_method_str_len(http_method m) {
    static constexpr uint8_t method_string_lens[] = {
#define XX(num, name, string) (uint8_t) detail::constLength(#string),
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
