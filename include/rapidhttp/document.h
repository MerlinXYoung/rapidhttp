#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "layer.hpp"
#include "util.h"

namespace rapidhttp {

// Http Header document class.
template <typename StringT>
class TDocument {
    static const StringT empty_string;

  public:
    using string_t = StringT;
    using header_type = std::pair<string_t, string_t>;
    using headers_type = std::vector<header_type>;
    using this_type = TDocument<string_t>;
    inline constexpr TDocument(int type = http_parser_type::HTTP_BOTH)
        : type_(type), major_(1), minor_(1) {}

    TDocument(http_method method, string_t&& uri, headers_type&& header_fields, string_t&& body,
              uint32_t major = 1, uint32_t minor = 1)
        : type_(HTTP_REQUEST),
          major_(major),
          minor_(major),
          method_(method),
          uri_or_status_(uri),
          header_fields_(header_fields),
          body_(body) {}

    TDocument(uint32_t code, string_t&& status, headers_type&& header_fields, string_t&& body,
              uint32_t major = 1, uint32_t minor = 1)
        : type_(HTTP_RESPONSE),
          major_(major),
          minor_(major),
          status_code_(code),
          uri_or_status_(status),
          header_fields_(header_fields),
          body_(body) {}
    TDocument(const TDocument& other)
        : type_(other.type_),
          major_(other.major_),
          minor_(other.major_),
          method_(other.method_),
          uri_or_status_(other.uri_or_status_),
          header_fields_(other.header_fields_),
          body_(other.body_) {}

    TDocument(TDocument&& other)
        : type_(other.type_),
          major_(other.major_),
          minor_(other.major_),
          method_(other.method_),
          uri_or_status_(std::move(other.uri_or_status_)),
          header_fields_(std::move(other.header_fields_)),
          body_(std::move(other.body_)) {}

    TDocument& operator=(const TDocument& other) {
        type_ = other.type_, major_ = other.major_;
        minor_ = other.major_;
        method_ = other.method_;
        uri_or_status_ = other.uri_or_status_;
        header_fields_ = other.header_fields_;
        body_ = other.body_;
        return *this;
    }

    TDocument& operator=(TDocument&& other) {
        type_ = other.type_, major_ = other.major_;
        minor_ = other.major_;
        method_ = other.method_;
        uri_or_status_ = std::move(other.uri_or_status_);
        header_fields_ = std::move(other.header_fields_);
        body_ = std::move(other.body_);
        return *this;
    }

    template <class StringT1>
    TDocument(const TDocument<StringT1>& other)
        : type_(other.type_),
          major_(other.major_),
          minor_(other.major_),
          method_(other.method_),
          uri_or_status_(other.uri_or_status_.data(), other.uri_or_status_.size()),
          header_fields_(),
          body_(other.body_.data(), other.body_.size()) {
        for (const auto& h : other.header_fields_) {
            header_fields_.emplace_back(std::make_pair(string_t(h.first.data(), h.first.size()),
                                                       string_t(h.second.data(), h.second.size())));
        }
    }
    template <class StringT1>
    TDocument& operator=(const TDocument<StringT1>& other) {
        type_ = other.type_, major_ = other.major_;
        minor_ = other.major_, method_ = other.method_;
        uri_or_status_ = string_t(other.uri_or_status_.data(), other.uri_or_status_.size());
        // header_fields_ = other.header_fields_;
        for (const auto& h : other.header_fields_) {
            header_fields_.emplace_back(std::make_pair(string_t(h.first.data(), h.first.size()),
                                                       string_t(h.second.data(), h.second.size())));
        }
        body_ = string_t(other.body_.data(), other.body_.size());
        return *this;
    }
    ~TDocument() = default;

    inline void Reset();

    /// 是否全部初始化完成, Serialize之前会做这个校验
    inline bool IsInitialized() const;

    /// Serialize后的数据长度
    inline size_t ByteSize() const;

    /// 序列化
    inline bool Serialize(char* buf, size_t len);
    inline std::string SerializeAsString();
    /// --------------------------------------------------------

    /// ------------------- fields get/set ---------------------
    inline uint32_t GetMajor() const noexcept { return major_; }
    inline this_type& SetMajor(uint32_t major) {
        major_ = major;
        return *this;
    }
    inline uint32_t GetMinor() const noexcept { return minor_; }
    inline this_type& SetMinor(uint32_t minor) {
        minor_ = minor;
        return *this;
    }
    inline uint32_t GetVersion() const noexcept { return major_ * 10 + minor_; }
    inline this_type& SetVersion(uint32_t version) {
        assert(version < 100);
        major_ = version / 10;
        minor_ = version % 10;
        return *this;
    }
    inline http_method GetMethod() const noexcept { return (http_method)method_; }
    inline this_type& SetMethod(http_method method) {
        method_ = method;
        return *this;
    }

    inline const char* GetMethodCStr() const noexcept {
        return http_method_str((http_method)method_);
    }

    inline string_t const& GetUri() const noexcept { return uri_or_status_; }
    inline this_type& SetUrl(const char* uri) {
        uri_or_status_ = uri;
        return *this;
    }
    template <class OStringT>
    inline this_type& SetUrl(const OStringT& uri) {
        uri_or_status_ = uri;
        return *this;
    }
    inline this_type& SetUrl(const string_t& uri) {
        uri_or_status_ = uri;
        return *this;
    }
    inline this_type& SetUrl(string_t&& uri) {
        uri_or_status_ = uri;
        return *this;
    }
    inline uint16_t GetStatusCode() const noexcept { return status_code_; }
    inline this_type& SetStatusCode(uint16_t code) {
        status_code_ = code;
        return *this;
    }
    inline this_type& SetStatus(uint16_t code, const char* status = nullptr) {
        status_code_ = code;
        uri_or_status_ = status ? status : http_status_str(code);
        return *this;
    }
    inline string_t const& GetStatus() const noexcept { return uri_or_status_; }
    inline this_type& SetStatus(const char* status) {
        uri_or_status_ = status;
        return *this;
    }
    template <class OStringT>
    inline this_type& SetStatus(const OStringT& status) {
        uri_or_status_ = status;
        return *this;
    }
    inline this_type& SetStatus(const string_t& status) {
        uri_or_status_ = status;
        return *this;
    }
    inline this_type& SetStatus(string_t&& status) {
        uri_or_status_ = status;
        return *this;
    }

    inline string_t const& GetBody() const noexcept { return body_; }
    inline this_type& SetBody(const char* body) {
        body_ = body;
        return *this;
    }
    template <class OStringT>
    inline this_type& SetBody(const OStringT& body) {
        body_ = body;
        return *this;
    }
    inline this_type& SetBody(const string_t& body) {
        body_ = body;
        return *this;
    }
    inline this_type& SetBody(string_t&& body) {
        body_ = body;
        return *this;
    }

    inline headers_type const& GetFields() const noexcept { return header_fields_; }
    inline string_t const* FindField(const char* key) const noexcept {
        for (const auto& h : header_fields_)
            if (h.first == key) return &h.second;
        return nullptr;
    }

    template <class OStringT>
    inline string_t const* FindField(const OStringT& key) const noexcept {
        for (const auto& h : header_fields_)
            if (h.first == key) return &h.second;
        return nullptr;
    }

    inline string_t const* FindField(const string_t& key) const noexcept {
        for (const auto& h : header_fields_)
            if (h.first == key) return &h.second;
        return nullptr;
    }

    inline string_t const& GetField(const char* key) const noexcept {
        for (const auto& h : header_fields_)
            if (h.first == key) return h.second;
        return empty_string;
    }

    template <class OStringT>
    inline string_t const& GetField(const OStringT& key) const noexcept {
        for (const auto& h : header_fields_)
            if (h.first == key) return h.second;
        return empty_string;
    }

    inline string_t const& GetField(const string_t& key) const noexcept {
        for (const auto& h : header_fields_)
            if (h.first == key) return h.second;
        return empty_string;
    }

    inline this_type& SetField(const header_type& h) {
        header_fields_.push_back(h);
        return *this;
    }

    inline this_type& SetField(header_type&& h) {
        header_fields_.emplace_back(h);
        return *this;
    }

    inline this_type& SetField(const string_t& key, const string_t& value) {
        return SetField(std::make_pair(key, value));
    }

    inline this_type& SetField(string_t&& key, string_t&& value) {
        return SetField(std::make_pair(std::move(key), std::move(value)));
    }

    inline this_type& SetField(const char* key, const char* value) {
        return SetField(string_t(key), string_t(value));
    }

    template <class OStringT1, class OStringT2>
    inline this_type& SetField(const OStringT1& key, const OStringT2& value) {
        return SetField(key.c_str(), value.c_str());
    }

  protected:
    /// --------------------------------------------------------

  protected:
    inline bool IsRequest() const noexcept { return type_ == HTTP_REQUEST; }
    inline bool IsResponse() const noexcept { return type_ == HTTP_RESPONSE; }

    inline bool CheckMethod() const noexcept;
    inline bool CheckUri() const noexcept;
    inline bool CheckStatusCode() const noexcept;
    inline bool CheckStatus() const noexcept;
    inline bool CheckVersion() const noexcept;

  private:
    uint8_t type_{HTTP_BOTH};
    // 默认版本号: HTTP/1.1
    uint8_t major_ : 4;
    uint8_t minor_ : 4;
    union {
        uint16_t method_{(uint16_t)-1};
        uint16_t status_code_;
    };
    string_t uri_or_status_;

    headers_type header_fields_;

    string_t body_;

    template <typename>
    friend class TParser;
    template <typename>
    friend class TDocument;
};
template <typename StringT>
inline void TDocument<StringT>::Reset() {
    major_ = 1;
    minor_ = 1;
    //   request_method_.clear();
    status_code_ = -1;
    uri_or_status_.clear();
    header_fields_.clear();
    body_.clear();
}
template <typename StringT>
inline bool TDocument<StringT>::CheckMethod() const noexcept {
    // return !request_method_.empty();
    return method_ >= 0 && method_ < ARRAY_SIZE(method_strings);
}
template <typename StringT>
inline bool TDocument<StringT>::CheckUri() const noexcept {
    return !uri_or_status_.empty() && uri_or_status_[0] == '/';
}
template <typename StringT>
inline bool TDocument<StringT>::CheckStatusCode() const noexcept {
    return status_code_ >= 100 && status_code_ < 1000;
}
template <typename StringT>
inline bool TDocument<StringT>::CheckStatus() const noexcept {
    return !uri_or_status_.empty();
}
template <typename StringT>
inline bool TDocument<StringT>::CheckVersion() const noexcept {
    return major_ < 10 && minor_ < 10;
}

template <typename StringT>
inline bool TDocument<StringT>::IsInitialized() const {
    if (IsRequest())
        return CheckMethod() && CheckUri() && CheckVersion();
    else
        return CheckVersion() && CheckStatusCode() && CheckStatus();
}

template <typename StringT>
inline size_t TDocument<StringT>::ByteSize() const {
    if (!IsInitialized()) return 0;

    size_t bytes = 0;
    if (IsRequest()) {
        // bytes += request_method_.size() + 1;  // GET\s
        bytes += http_method_str_len(http_method(method_)) + 1;
        bytes += GetUri().size() + 1;  // /uri\s
        bytes += 10;                   // HTTP/1.1CRLF
    } else {
        bytes += 9;                                      // HTTP/1.1\s
        bytes += UIntegerByteSize(GetStatusCode()) + 1;  // 200\s
        bytes += GetStatus().size() + 2;                 // okCRLF
    }
    for (auto const& kv : header_fields_) {
        bytes += kv.first.size() + 2 + kv.second.size() + 2;
    }
    bytes += 2;
    bytes += body_.size();
    return bytes;
}

template <typename StringT>
inline bool TDocument<StringT>::Serialize(char* buf, size_t len) {
    size_t bytes = ByteSize();
    if (!bytes || len < bytes) return false;
#define _WRITE_STRING(ss)                  \
    do {                                   \
        memcpy(buf, ss.data(), ss.size()); \
        buf += ss.size();                  \
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
    if (IsRequest()) {
        // _WRITE_STRING(request_method_);
        _WRITE_C_STR(http_method_str(GetMethod()), http_method_str_len(GetMethod()));
        *buf++ = ' ';
        _WRITE_STRING(GetUri());
        _WRITE_C_STR(" HTTP/", 6);
        *buf++ = major_ + '0';
        *buf++ = '.';
        *buf++ = minor_ + '0';
    } else {
        _WRITE_C_STR("HTTP/", 5);
        *buf++ = major_ + '0';
        *buf++ = '.';
        *buf++ = minor_ + '0';
        *buf++ = ' ';
        *buf++ = (GetStatusCode() / 100) + '0';
        *buf++ = (GetStatusCode() % 100) / 10 + '0';
        *buf++ = (GetStatusCode() % 10) + '0';
        *buf++ = ' ';
        _WRITE_STRING(GetStatus());
    }
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
inline std::string TDocument<StringT>::SerializeAsString() {
    std::string s;
    size_t bytes = ByteSize();
    if (!bytes) return "";
    s.resize(bytes);
    if (!Serialize(&s[0], bytes)) return "";
    return s;
}
template <typename StringT>
const StringT TDocument<StringT>::empty_string;

using Document = TDocument<std::string>;
// using RefDocument = TDocument<StringRef>;
}  // namespace rapidhttp
