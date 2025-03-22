#pragma once

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

#include "cmake_config.h"
#include "constants.h"
#include "error_code.h"
#include "layer.hpp"
#include "request.h"
#include "response.h"
#include "stringref.h"

namespace rapidhttp {

// enum ParserType {
//     Request,
//     Response,
// };

// Http Header document class.
template <typename StringT>
class TParser {
  public:
    using string_t = StringT;
    using document_type = TDocument<string_t>;
    using request_t = TRequest<string_t>;
    using response_t = TResponse<string_t>;

    explicit TParser(http_parser_type type);
    TParser(TParser const &other) = delete;
    TParser(TParser &&other) = delete;
    TParser &operator=(TParser const &other) = delete;
    TParser &operator=(TParser &&other) = delete;

    /// ------------------- parse/generate ---------------------
    /// 流式解析
    // @buf_ref: 外部传入的缓冲区首地址
    // @len: 缓冲区长度
    // @returns：返回已成功解析到的数据长度
    inline size_t PartailParse(const char *buf_ref, size_t len);
    inline size_t PartailParse(std::string const &buf);

    /// 解析eof
    // 解析Response时, 断开链接时要调用这个接口, 因为有些response协议需要读取到
    // 网络链接断开为止.
    inline bool PartailParseEof();

    /// 是否解析成功
    inline bool ParseDone() const noexcept;

    /// 重置解析流状态
    // 同时清除解析流状态和已解析成功的数据状态
    inline void Reset();

    /// 返回解析错误码
    inline std::error_code ParseError() const noexcept;

    inline const document_type &GetDoc() const noexcept { return doc_; }
    inline document_type &&StealDoc() { return std::move(doc_); }

    request_t &&StealRequest();
    response_t &&StealResponse();

    template <typename OStringT>
    TRequest<OStringT> &&StealRequest();
    template <typename OStringT>
    TResponse<OStringT> &&StealResponse();

    inline bool IsRequest() const noexcept { return doc_.IsRequest(); }
    inline bool IsResponse() const noexcept { return doc_.IsResponse(); }

  private:
    inline bool CheckMethod() const noexcept;
    inline bool CheckUri() const noexcept;
    inline bool CheckStatusCode() const noexcept;
    inline bool CheckStatus() const noexcept;
    inline bool CheckVersion() const noexcept;

#if USE_PICO
#else
    // http-parser
  private:
    static inline int sOnHeadersComplete(http_parser *parser);
    static inline int sOnMessageComplete(http_parser *parser);
    static inline int sOnUrl(http_parser *parser, const char *at, size_t length);
    static inline int sOnStatus(http_parser *parser, const char *at, size_t length);
    static inline int sOnHeaderField(http_parser *parser, const char *at, size_t length);
    static inline int sOnHeaderValue(http_parser *parser, const char *at, size_t length);
    static inline int sOnBody(http_parser *parser, const char *at, size_t length);

    inline int OnHeadersComplete(http_parser *parser);
    inline int OnMessageComplete(http_parser *parser);
    inline int OnUrl(http_parser *parser, const char *at, size_t length);
    inline int OnStatus(http_parser *parser, const char *at, size_t length);
    inline int OnHeaderField(http_parser *parser, const char *at, size_t length);
    inline int OnHeaderValue(http_parser *parser, const char *at, size_t length);
    inline int OnBody(http_parser *parser, const char *at, size_t length);
#endif

  private:
    document_type doc_;
    // ParserType type_;  // 类型

    bool parse_done_{false};
    std::error_code ec_;  // 解析错状态

#if USE_PICO
#else
    struct http_parser parser_;
    struct http_parser_settings settings_;
#endif

    int kv_state_{0};
    string_t callback_header_key_cache_;
    string_t callback_header_value_cache_;

    template <typename T>
    friend class TParser;
};

template <class StringT>
struct TRequestParser : public TParser<StringT> {
    using base_type = TParser<StringT>;
    inline TRequestParser() : base_type(HTTP_REQUEST) {}
};
template <class StringT>
struct TResponseParser : public TParser<StringT> {
    using base_type = TParser<StringT>;
    inline TResponseParser() : base_type(HTTP_RESPONSE) {}
};

using RequestParser = TRequestParser<std::string>;
using ResponseParser = TResponseParser<std::string>;

}  // namespace rapidhttp

#include "parser.hpp"
