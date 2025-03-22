#pragma once
#include <stdio.h>

#include <algorithm>

#include "parser.h"
#include "util.h"

namespace rapidhttp {

template <typename StringT>
inline TParser<StringT>::TParser(http_parser_type type) : doc_(type) {
    Reset();
#if USE_PICO
#else
    memset(&settings_, 0, sizeof(settings_));
    settings_.on_headers_complete = sOnHeadersComplete;
    settings_.on_message_complete = sOnMessageComplete;
    settings_.on_url = sOnUrl;
    settings_.on_status = sOnStatus;
    settings_.on_header_field = sOnHeaderField;
    settings_.on_header_value = sOnHeaderValue;
    settings_.on_body = sOnBody;
#endif
}

/// ------------------- parse/generate ---------------------
/// 流式解析
// @buf_ref: 外部传入的缓冲区首地址, 再调用Storage前必须保证缓冲区有效且不变.
// @len: 缓冲区长度
// @returns：解析完成返回error_code=0, 解析一半返回error_code=1,
// 解析失败返回其他错误码.
template <typename StringT>
inline size_t TParser<StringT>::PartailParse(std::string const& buf) {
    return PartailParse(buf.c_str(), buf.size());
}

#if USE_PICO
#else
template <typename StringT>
inline size_t TParser<StringT>::PartailParse(const char *buf_ref, size_t len) {
    if (ParseDone() || ParseError()) Reset();

    size_t parsed = http_parser_execute(&parser_, &settings_, buf_ref, len);
    if (parser_.http_errno) {
        // TODO: support pause
        ec_ = MakeParseErrorCode(parser_.http_errno);
    }
    return parsed;
}
template <typename StringT>
inline bool TParser<StringT>::PartailParseEof() {
    if (ParseDone() || ParseError()) return false;

    PartailParse("", 0);
    return ParseDone();
}
template <typename StringT>
inline bool TParser<StringT>::ParseDone() const noexcept {
    return parse_done_;
}

template <typename StringT>
inline int TParser<StringT>::sOnHeadersComplete(http_parser *parser) {
    return ((TParser *)parser->data)->OnHeadersComplete(parser);
}
template <typename StringT>
inline int TParser<StringT>::sOnMessageComplete(http_parser *parser) {
    return ((TParser *)parser->data)->OnMessageComplete(parser);
}
template <typename StringT>
inline int TParser<StringT>::sOnUrl(http_parser *parser, const char *at, size_t length) {
    return ((TParser *)parser->data)->OnUrl(parser, at, length);
}
template <typename StringT>
inline int TParser<StringT>::sOnStatus(http_parser *parser, const char *at, size_t length) {
    return ((TParser *)parser->data)->OnStatus(parser, at, length);
}
template <typename StringT>
inline int TParser<StringT>::sOnHeaderField(http_parser *parser, const char *at, size_t length) {
    return ((TParser *)parser->data)->OnHeaderField(parser, at, length);
}
template <typename StringT>
inline int TParser<StringT>::sOnHeaderValue(http_parser *parser, const char *at, size_t length) {
    return ((TParser *)parser->data)->OnHeaderValue(parser, at, length);
}
template <typename StringT>
inline int TParser<StringT>::sOnBody(http_parser *parser, const char *at, size_t length) {
    return ((TParser *)parser->data)->OnBody(parser, at, length);
}

template <typename StringT>
inline int TParser<StringT>::OnHeadersComplete(http_parser *parser) {
    if (IsRequest())
        // request_method_ = http_method_str((http_method)parser->method);
        // request_method_ = (http_method)parser->method;
        doc_.SetMethod((http_method)parser->method);
    else
        // response_status_code_ = parser->status_code;
        doc_.SetStatusCode(parser->status_code);
    doc_.SetMajor(parser->http_major);
    doc_.SetMinor(parser->http_minor);
    if (kv_state_ == 1) {
        // doc_.SetField(std::move(callback_header_key_cache_),
        //               std::move(callback_header_value_cache_));
        doc_.header_fields_.emplace_back(std::move(callback_header_key_cache_),
                                         std::move(callback_header_value_cache_));
        kv_state_ = 0;
    }
    return 0;
}
template <typename StringT>
inline int TParser<StringT>::OnMessageComplete(http_parser *parser) {
    parse_done_ = true;
    return 0;
}
template <typename StringT>
inline int TParser<StringT>::OnUrl(http_parser *parser, const char *at, size_t length) {
    doc_.uri_or_status_.append(at, length);
    return 0;
}
template <typename StringT>
inline int TParser<StringT>::OnStatus(http_parser *parser, const char *at, size_t length) {
    doc_.uri_or_status_.append(at, length);
    return 0;
}
template <typename StringT>
inline int TParser<StringT>::OnHeaderField(http_parser *parser, const char *at, size_t length) {
    if (kv_state_ == 1) {
        // doc_.SetField(std::move(callback_header_key_cache_),
        //               std::move(callback_header_value_cache_));
        doc_.header_fields_.emplace_back(std::move(callback_header_key_cache_),
                                         std::move(callback_header_value_cache_));
        kv_state_ = 0;
    }

    callback_header_key_cache_.append(at, length);
    return 0;
}
template <typename StringT>
inline int TParser<StringT>::OnHeaderValue(http_parser *parser, const char *at, size_t length) {
    kv_state_ = 1;
    callback_header_value_cache_.append(at, length);
    return 0;
}
template <typename StringT>
inline int TParser<StringT>::OnBody(http_parser *parser, const char *at, size_t length) {
    doc_.body_.append(at, length);
    return 0;
}
#endif

template <typename StringT>
inline void TParser<StringT>::Reset() {
#if USE_PICO
#else
    http_parser_init(&parser_, IsRequest() ? HTTP_REQUEST : HTTP_RESPONSE);
    parser_.data = this;
#endif
    doc_.Reset();
    parse_done_ = false;
    ec_ = std::error_code();
    kv_state_ = 0;
    callback_header_key_cache_.clear();
    callback_header_value_cache_.clear();
    // major_ = 1;
    // minor_ = 1;
    // //   request_method_.clear();
    // request_method_ = http_method(0);
    // request_uri_.clear();
    // response_status_code_ = 0;
    // response_status_.clear();
    // header_fields_.clear();
    // body_.clear();
}

// 返回解析错误码
template <typename StringT>
inline std::error_code TParser<StringT>::ParseError() const noexcept {
    return ec_;
}

template <typename StringT>
inline typename TParser<StringT>::request_t&& TParser<StringT>::StealRequest() {
    // return request_t(request_method_, std::move(request_uri_), std::move(header_fields_),
    //                  std::move(body_), major_, minor_);
    return (request_t&&)std::move(doc_);
}
template <typename StringT>
inline typename TParser<StringT>::response_t&& TParser<StringT>::StealResponse() {
    // return response_t(response_status_code_, std::move(response_status_),
    // std::move(header_fields_),
    //                   std::move(body_), major_, minor_);
    return (response_t&&)std::move(doc_);
}
template <typename StringT>
template <typename OStringT>
inline TRequest<OStringT>&& TParser<StringT>::StealRequest() {
    // return TRequest<OStringT>(request_method_, std::move(request_uri_),
    // std::move(header_fields_),
    //                           std::move(body_), major_, minor_);
    return (request_t&&)std::move(doc_);
}
template <typename StringT>
template <typename OStringT>
inline TResponse<OStringT>&& TParser<StringT>::StealResponse() {
    // return TResponse<OStringT>(response_status_code_, std::move(response_status_),
    //                            std::move(header_fields_), std::move(body_), major_, minor_);
    return (response_t&&)std::move(doc_);
}
/// --------------------------------------------------------

typedef TParser<std::string> Parser;
typedef TParser<StringRef> RefParser;

}  // namespace rapidhttp
