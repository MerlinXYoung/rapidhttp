#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "doc.h"
namespace rapidhttp {

struct Method {
    enum {
#define XX(num, name, string) name = num,
        HTTP_METHOD_MAP(XX)
#undef XX
    };
    inline constexpr Method(int m) : value_(m){};
    inline bool isDelete() const noexcept { return DELETE == value_; }
    inline bool isGet() const noexcept { return GET == value_; }
    inline bool isHead() const noexcept { return HEAD == value_; }

    inline bool isPost() const noexcept { return POST == value_; }
    inline bool isPut() const noexcept { return PUT == value_; }
    inline bool isConnect() const noexcept { return CONNECT == value_; }
    inline bool isOptions() const noexcept { return OPTIONS == value_; }
    inline bool isTrace() const noexcept { return TRACE == value_; }

    inline operator int() const noexcept { return value_; }

    inline const char* toCStr() const noexcept { return http_method_str(http_method(value_)); }
    inline uint32_t strLen() const noexcept { return http_method_str_len(http_method(value_)); }

    static Method from(const char* str) { return get_http_method(str); }

  private:
    int value_;
};

template <class StringT>
struct TRequest : public TDocument<StringT> {
    template <class>
    friend class TParser;
    using base_type = TDocument<StringT>;
    using string_t = typename base_type::string_t;
    using header_type = typename base_type::header_type;
    using headers_type = typename base_type::headers_type;
    using this_type = TRequest<string_t>;
    TRequest() : base_type(HTTP_REQUEST) {}
    using base_type::base_type;

    inline Method GetMethod() const noexcept { return Method((int)base_type::method_); }
};

}  // namespace rapidhttp