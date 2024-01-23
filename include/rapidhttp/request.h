#pragma once
#include <rapidhttp/layer.h>

#include <cstdint>
#include <string>
#include <utility>
#include <vector>
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
    inline uint32_t strLen() const noexcept { return http_method_string_len(http_method(value_)); }

    static Method from(const char* str) { return get_http_method(str); }

  private:
    int value_;
};

template <class StringT>
class TRequest {
  public:
    typedef StringT string_t;
    using headers_type = std::vector<std::pair<string_t, string_t>>;
    TRequest() = default;
    ~TRequest() = default;
    TRequest(uint32_t major, uint32_t minor_, http_method method_, string_t&& uri,
             headers_type&& header_fields, string_t&& body)
        : major_(major),
          minor_(major),
          method_(method),
          uri_(uri),
          header_fields_(header_fields),
          body_(body) {}

    TRequest(const TRequest& other)
        : major_(other.major_),
          minor_(other.major_),
          method_(other.method_),
          uri_(other.uri_),
          header_fields_(other.header_fields_),
          body_(other.body_) {}

    TRequest(TRequest&& other)
        : major_(other.major_),
          minor_(other.major_),
          method_(other.method_),
          uri_(std::move(other.uri_)),
          header_fields_(std::move(other.header_fields_)),
          body_(std::move(other.body_)) {}

    TRequest& operator=(const TRequest& other) {
        major_ = other.major_;
        minor_ = other.major_, method_ = other.method_;
        uri_ = other.uri_;
        header_fields_ = other.header_fields_;
        body_ = other.body_;
    }

    TRequest& operator=(TRequest&& other) {
        major_ = other.major_;
        minor_ = other.major_, method_ = other.method_;
        uri_ = std::move(other.uri_);
        header_fields_ = std::move(other.header_fields_);
        body_ = std::move(other.body_);
    }

    template <class StringT1>
    TRequest(const TRequest<StringT1>& other)
        : major_(other.major_),
          minor_(other.major_),
          method_(other.method_),
          uri_(other.uri_.c_str()),
          header_fields_(),
          body_(other.body_) {
        for (const auto& h : other.header_fields_) {
            header_fields_.emplace_back(
                std::make_pair(string_t(h.first.c_str()), string_t(h.second.c_str())));
        }
    }

    inline uint32_t major() const noexcept { return major_; }
    inline uint32_t minor() const noexcept { return minor_; }

    inline Method method() const noexcept { return Method(method_); }
    inline string_t const& uri() const noexcept { return uri_; }
    inline string_t const& body() const noexcept { return body_; }

    /// 是否全部初始化完成, Serialize之前会做这个校验
    inline bool IsInitialized() const;

    /// Serialize后的数据长度
    inline size_t ByteSize() const;

    /// 序列化
    inline bool Serialize(char* buf, size_t len);
    inline std::string SerializeAsString();

  private:
    inline bool CheckMethod() const;
    inline bool CheckUri() const;
    // inline bool CheckStatusCode() const;
    // inline bool CheckStatus() const;
    inline bool CheckVersion() const;

  private:
    // 默认版本号: HTTP/1.1
    uint32_t major_{1};
    uint32_t minor_{1};

    //   string_t method_;
    http_method method_{http_method::HTTP_GET};
    string_t uri_;

    std::vector<std::pair<string_t, string_t>> header_fields_;

    string_t body_;

    template <typename T>
    friend class TRequest;
};

template <typename StringT>
inline bool TRequest<StringT>::IsInitialized() const {
    return CheckMethod() && CheckUri() && CheckVersion();
}

template <typename StringT>
inline size_t TRequest<StringT>::ByteSize() const {
    if (!IsInitialized()) return 0;

    size_t bytes = 0;

    // TODO: static array
    // auto method = http_method_str(request_method_);
    // bytes += ::strlen(method);
    bytes += http_method_string_len(http_method(method_));
    // bytes += request_method_.size() + 1; // GET\s
    bytes += uri_.size() + 1;  // /uri\s
    bytes += 10;               // HTTP/1.1CRLF

    for (auto const& kv : header_fields_) {
        bytes += kv.first.size() + 2 + kv.second.size() + 2;
    }
    bytes += 2;
    bytes += body_.size();
    return bytes;
}

template <typename StringT>
inline bool TRequest<StringT>::Serialize(char* buf, size_t len) {
    size_t bytes = ByteSize();
    if (!bytes || len < bytes) return false;
#define _WRITE_STRING(ss)                   \
    do {                                    \
        memcpy(buf, ss.c_str(), ss.size()); \
        buf += ss.size();                   \
    } while (0);

#define _WRITE_C_STR(c_str, length) \
    do {                            \
        memcpy(buf, c_str, length); \
        buf += length;              \
    } while (0);

#define _WRITE_CRLF() \
    *buf++ = '\r';    \
    *buf++ = '\n'

    char* ori = buf;

    // _WRITE_STRING(request_method_);
    _WRITE_C_STR(http_method_str(method_), http_method_string_len(method_));
    *buf++ = ' ';
    _WRITE_STRING(uri_);
    _WRITE_C_STR(" HTTP/", 6);
    *buf++ = major_ + '0';
    *buf++ = '.';
    *buf++ = minor_ + '0';

    _WRITE_CRLF();
    for (auto const& kv : header_fields_) {
        _WRITE_STRING(kv.first);
        *buf++ = ':';
        *buf++ = ' ';
        _WRITE_STRING(kv.second);
        _WRITE_CRLF();
    }
    _WRITE_CRLF();
    _WRITE_STRING(body_);
    size_t length = buf - ori;
    (void)length;
    return true;
#undef _WRITE_CRLF
#undef _WRITE_C_STR
#undef _WRITE_STRING
}
template <typename StringT>
inline std::string TRequest<StringT>::SerializeAsString() {
    std::string s;
    size_t bytes = ByteSize();
    if (!bytes) return "";
    s.resize(bytes);
    if (!Serialize(&s[0], bytes)) return "";
    return s;
}
template <typename StringT>
inline bool TRequest<StringT>::CheckMethod() const {
    //   return !request_method_.empty();
    return method_ >= 0 && method_ < ARRAY_SIZE(method_strings);
}
template <typename StringT>
inline bool TRequest<StringT>::CheckUri() const {
    return !uri_.empty() && uri_[0] == '/';
}
// template <typename StringT>
// inline bool TRequest<StringT>::CheckStatusCode() const {
//     return response_status_code_ >= 100 && response_status_code_ < 1000;
// }
// template <typename StringT>
// inline bool TRequest<StringT>::CheckStatus() const {
//     return !response_status_.empty();
// }
template <typename StringT>
inline bool TRequest<StringT>::CheckVersion() const {
    return major_ >= 0 && major_ <= 9 && minor_ >= 0 && minor_ <= 9;
}

}  // namespace rapidhttp