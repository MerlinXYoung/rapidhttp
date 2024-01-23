#pragma once
#include <rapidhttp/layer.h>
#include <rapidhttp/util.h>

#include <cstdint>
#include <utility>
#include <vector>

namespace rapidhttp {

template <class StringT>
class TResponse {
    typedef StringT string_t;
    using headers_type = std::vector<std::pair<string_t, string_t>>;
    TResponse() = default;
    ~TResponse() = default;
    TResponse(uint32_t major, uint32_t minor, uint32_t code, string_t &&status,
              headers_type &&header_fields, string_t &&body)
        : major_(major),
          minor_(major),
          status_code_(code),
          status_(status),
          header_fields_(header_fields),
          body_(body) {}
    TResponse(const TResponse &other)
        : major_(other.major_),
          minor_(other.major_),
          status_code_(other.status_code_),
          status_(other.status_),
          header_fields_(other.header_fields_),
          body_(other.body_) {}

    TResponse(TResponse &&other)
        : major_(other.major_),
          minor_(other.major_),
          status_code_(other.status_code_),
          status_(std::move(other.status_)),
          header_fields_(std::move(other.header_fields_)),
          body_(std::move(other.body_)) {}

    TResponse &operator=(const TResponse &other) {
        major_ = other.major_;
        minor_ = other.major_, status_code_ = other.status_code_;
        status_ = other.status_;
        header_fields_ = other.header_fields_;
        body_ = other.body_;
    }

    TResponse &operator=(TResponse &&other) {
        major_ = other.major_;
        minor_ = other.major_, status_code_ = other.status_code_;
        status_ = std::move(other.status_);
        header_fields_ = std::move(other.header_fields_);
        body_ = std::move(other.body_);
    }

    template <class StringT1>
    TResponse(const TResponse<StringT1> &other)
        : major_(other.major_),
          minor_(other.major_),
          status_code_(other.status_code_),
          status_(other.status_.c_str()),
          header_fields_(),
          body_(other.body_) {
        for (const auto &h : other.header_fields_) {
            header_fields_.emplace_back(
                std::make_pair(string_t(h.first.c_str()), string_t(h.second.c_str())));
        }
    }

    inline uint32_t major() const noexcept { return major_; }
    inline uint32_t minor() const noexcept { return minor_; }

    inline uint32_t status_code() const noexcept { return status_code_; }
    inline string_t const &status() const noexcept { return status_; }

    inline string_t const &body() const noexcept { return body_; }

    /// 是否全部初始化完成, Serialize之前会做这个校验
    inline bool IsInitialized() const;

    /// Serialize后的数据长度
    inline size_t ByteSize() const;

    /// 序列化
    inline bool Serialize(char *buf, size_t len);
    inline std::string SerializeAsString();

  private:
    // inline bool CheckMethod() const;
    // inline bool CheckUri() const;
    inline bool CheckStatusCode() const;
    inline bool CheckStatus() const;
    inline bool CheckVersion() const;

  private:
    // 默认版本号: HTTP/1.1
    uint32_t major_{1};
    uint32_t minor_{1};

    uint32_t status_code_{0};
    string_t status_;

    std::vector<std::pair<string_t, string_t>> header_fields_;

    string_t body_;

    template <typename T>
    friend class TResponse;
};

template <typename StringT>
inline bool TResponse<StringT>::IsInitialized() const {
    return CheckVersion() && CheckStatusCode() && CheckStatus();
}

template <typename StringT>
inline size_t TResponse<StringT>::ByteSize() const {
    if (!IsInitialized()) return 0;

    size_t bytes = 0;

    bytes += 9;                                   // HTTP/1.1\s
    bytes += UIntegerByteSize(status_code_) + 1;  // 200\s
    bytes += status_.size() + 2;                  // okCRLF

    for (auto const &kv : header_fields_) {
        bytes += kv.first.size() + 2 + kv.second.size() + 2;
    }
    bytes += 2;
    bytes += body_.size();
    return bytes;
}

template <typename StringT>
inline bool TResponse<StringT>::Serialize(char *buf, size_t len) {
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

    char *ori = buf;

    _WRITE_C_STR("HTTP/", 5);
    *buf++ = major_ + '0';
    *buf++ = '.';
    *buf++ = minor_ + '0';
    *buf++ = ' ';
    *buf++ = (status_code_ / 100) + '0';
    *buf++ = (status_code_ % 100) / 10 + '0';
    *buf++ = (status_code_ % 10) + '0';
    *buf++ = ' ';
    _WRITE_STRING(status_);

    _WRITE_CRLF();
    for (auto const &kv : header_fields_) {
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
inline std::string TResponse<StringT>::SerializeAsString() {
    std::string s;
    size_t bytes = ByteSize();
    if (!bytes) return "";
    s.resize(bytes);
    if (!Serialize(&s[0], bytes)) return "";
    return s;
}
// template <typename StringT>
// inline bool TResponse<StringT>::CheckMethod() const {
//     //   return !request_method_.empty();
//     return request_method_ == -1;
// }
// template <typename StringT>
// inline bool TResponse<StringT>::CheckUri() const {
//     return !request_uri_.empty() && request_uri_[0] == '/';
// }
template <typename StringT>
inline bool TResponse<StringT>::CheckStatusCode() const {
    return status_code_ >= 100 && status_code_ < 1000;
}
template <typename StringT>
inline bool TResponse<StringT>::CheckStatus() const {
    return !status_.empty();
}
template <typename StringT>
inline bool TResponse<StringT>::CheckVersion() const {
    return major_ >= 0 && major_ <= 9 && minor_ >= 0 && minor_ <= 9;
}
}  // namespace rapidhttp