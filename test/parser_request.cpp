#include <gtest/gtest.h>
#include <rapidhttp/parser.h>
#include <unistd.h>

#include <iostream>

#include "rapidhttp/request.h"
#include "rapidhttp/stringref.h"

using namespace std;
using namespace rapidhttp;

static std::string c_http_request =
    "GET /uri/abc HTTP/1.1\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "Connection: Keep-Alive\r\n"
    "\r\n";

static std::string c_http_request_2 =
    "POST /uri/abc HTTP/1.1\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "User-Agent: gtest.proxy\r\n"
    "Content-Length: 3\r\n"
    "\r\nabc";

// 错误的协议头
static std::string c_http_request_err_1 =
    "POST/uri/abc HTTP/1.1\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "User-Agent: gtest.proxy\r\n"
    "\r\n";

// 兼容HTTP0.9的协议头
static std::string c_http_request_http_0_9 =
    "POST /uri/abcHTTP/1.1\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "User-Agent: gtest.proxy\r\n"
    "\r\n";

// 一部分协议头, 缺少一个\r\n
static std::string c_http_request_err_3 =
    "POST /uri/abc HTTP/1.1\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "User-Agent: gtest.proxy\r\n";

template <typename String>
static void test_parse_request() {
    TRequestParser<String> parser;
    TRequest<String> request;
    size_t bytes = parser.PartailParse(c_http_request);
    EXPECT_EQ(bytes, c_http_request.size());
    EXPECT_TRUE(!parser.ParseError());

    EXPECT_STREQ(parser.GetMethodCStr(), "GET");
    EXPECT_EQ(parser.GetUri(), "/uri/abc");
    EXPECT_EQ(parser.GetMajor(), 1);
    EXPECT_EQ(parser.GetMinor(), 1);
    EXPECT_EQ(parser.GetField("Accept"), "XAccept");
    EXPECT_EQ(parser.GetField("Host"), "domain.com");
    EXPECT_EQ(parser.GetField("Connection"), "Keep-Alive");
    EXPECT_EQ(parser.GetField("User-Agent"), "");

    for (int i = 0; i < 10; ++i) {
        size_t bytes = parser.PartailParse(c_http_request.c_str(), c_http_request.size());
        EXPECT_EQ(bytes, c_http_request.size());
        EXPECT_TRUE(!parser.ParseError());
    }

    EXPECT_STREQ(parser.GetMethodCStr(), "GET");
    EXPECT_EQ(parser.GetUri(), "/uri/abc");
    EXPECT_EQ(parser.GetMajor(), 1);
    EXPECT_EQ(parser.GetMinor(), 1);
    EXPECT_EQ(parser.GetField("Accept"), "XAccept");
    EXPECT_EQ(parser.GetField("Host"), "domain.com");
    EXPECT_EQ(parser.GetField("Connection"), "Keep-Alive");
    EXPECT_EQ(parser.GetField("User-Agent"), "");

    bytes = parser.PartailParse(c_http_request_2);
    EXPECT_EQ(bytes, c_http_request_2.size());
    EXPECT_TRUE(!parser.ParseError());
    EXPECT_STREQ(parser.GetMethodCStr(), "POST");
    EXPECT_EQ(parser.GetUri(), "/uri/abc");
    EXPECT_EQ(parser.GetMajor(), 1);
    EXPECT_EQ(parser.GetMinor(), 1);
    EXPECT_EQ(parser.GetField("Accept"), "XAccept");
    EXPECT_EQ(parser.GetField("Host"), "domain.com");
    EXPECT_EQ(parser.GetField("Connection"), "");
    EXPECT_EQ(parser.GetField("User-Agent"), "gtest.proxy");
    EXPECT_EQ(parser.GetBody(), "abc");

    bytes = parser.PartailParse(c_http_request_err_1);
    EXPECT_TRUE(parser.ParseError());

    bytes = parser.PartailParse(c_http_request_http_0_9);
    EXPECT_FALSE(parser.ParseError());
    EXPECT_TRUE(parser.ParseDone());
    EXPECT_EQ(parser.GetMajor(), 0);
    EXPECT_EQ(parser.GetMinor(), 9);

    // partail parse logic
    cout << "parse partail" << endl;
    bytes = parser.PartailParse(c_http_request_err_3);
    EXPECT_EQ(bytes, c_http_request_err_3.size());
    EXPECT_FALSE(parser.ParseError());
    EXPECT_FALSE(parser.ParseDone());

    bytes = parser.PartailParse("\r\n");
    EXPECT_EQ(bytes, 2);
    EXPECT_FALSE(parser.ParseError());

    for (size_t pos = 0; pos < c_http_request.size(); ++pos) {
        //        cout << "parse split by " << pos << endl;
        std::string fp = c_http_request.substr(0, pos);
        size_t bytes = parser.PartailParse(fp);
        //        EXPECT_EQ(bytes, pos);
        //        EXPECT_EQ(parser.ParseError().value(), 1);
        EXPECT_FALSE(parser.ParseDone());

        std::string sp = c_http_request.substr(bytes);
        bytes += parser.PartailParse(sp);
        EXPECT_EQ(bytes, c_http_request.size());
        EXPECT_TRUE(!parser.ParseError());
        EXPECT_TRUE(parser.ParseDone());

        EXPECT_STREQ(parser.GetMethodCStr(), "GET");
        EXPECT_EQ(parser.GetUri(), "/uri/abc");
        EXPECT_EQ(parser.GetMajor(), 1);
        EXPECT_EQ(parser.GetMinor(), 1);
        EXPECT_EQ(parser.GetField("Accept"), "XAccept");
        EXPECT_EQ(parser.GetField("Host"), "domain.com");
        EXPECT_EQ(parser.GetField("Connection"), "Keep-Alive");
        EXPECT_EQ(parser.GetField("User-Agent"), "");
    }
    // for stringref
    parser.PartailParse(c_http_request);

    char buf[256] = {};
    request = parser.plunderRequest();
    bool b = request.Serialize(buf, sizeof(buf));
    EXPECT_TRUE(b);
    bytes = request.ByteSize();
    EXPECT_EQ(bytes, c_http_request.size());
    EXPECT_EQ(c_http_request, buf);

    bytes = parser.PartailParse(c_http_request_2);
    EXPECT_EQ(bytes, c_http_request_2.size());
    EXPECT_TRUE(parser.ParseDone());
    request = parser.plunderRequest();
    b = request.Serialize(buf, sizeof(buf));
    EXPECT_TRUE(b);
    bytes = request.ByteSize();
    EXPECT_EQ(bytes, c_http_request_2.size());
    EXPECT_EQ(c_http_request_2, buf);
}
#if 1
static void copyto_request() {
    std::string s = c_http_request_2;

    rapidhttp::TRequestParser<std::string> parser;
    size_t bytes = parser.PartailParse(s);
    EXPECT_EQ(bytes, s.size());
    EXPECT_TRUE(!parser.ParseError());

#define _CHECK_DOC(parser)                                   \
    EXPECT_STREQ(parser.GetMethodCStr(), "POST");            \
    EXPECT_EQ(parser.GetUri(), "/uri/abc");                  \
    EXPECT_EQ(parser.GetMajor(), 1);                         \
    EXPECT_EQ(parser.GetMinor(), 1);                         \
    EXPECT_EQ(parser.GetField("Accept"), "XAccept");         \
    EXPECT_EQ(parser.GetField("Host"), "domain.com");        \
    EXPECT_EQ(parser.GetField("Connection"), "");            \
    EXPECT_EQ(parser.GetField("User-Agent"), "gtest.proxy"); \
    EXPECT_EQ(parser.GetBody(), "abc")

    _CHECK_DOC(parser);

    auto request = parser.plunderRequest();

    TRequest<StringRef> requestRef(request);

    EXPECT_EQ(request.SerializeAsString(), requestRef.SerializeAsString());

    TRequest<std::string> tmp = requestRef;
    EXPECT_EQ(tmp.SerializeAsString(), requestRef.SerializeAsString());
}
#endif

TEST(parser, request) {
    test_parse_request<std::string>();
    test_parse_request<StringRef>();
    copyto_request();
}
