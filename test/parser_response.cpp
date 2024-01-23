#include <gtest/gtest.h>
#include <rapidhttp/parser.h>
#include <unistd.h>

#include <iostream>
using namespace std;
using namespace rapidhttp;

static std::string c_http_response =
    "HTTP/1.1 200 OK\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "Connection: Keep-Alive\r\n"
    "Content-Length: 3\r\n"
    "\r\nxyz";

static std::string c_http_response_2 =
    "HTTP/1.1 404 Not Found\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "User-Agent: gtest.proxy\r\n"
    "\r\n";

// 错误的协议头
static std::string c_http_response_err_1 =
    "HTTP/1.1200 OK\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "User-Agent: gtest.proxy\r\n"
    "\r\n";

// 错误的协议头
static std::string c_http_response_err_2 =
    "HTTP/1.1 200OK\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "User-Agent: gtest.proxy\r\n"
    "\r\n";

// 一部分协议头, 缺少一个\r\n
static std::string c_http_response_err_3 =
    "HTTP/1.1 200 OK\r\n"
    "Accept: XAccept\r\n"
    "Host: domain.com\r\n"
    "Content-Length: 0\r\n"
    "User-Agent: gtest.proxy\r\n";

template <typename String>
void test_parse_response() {
    TResponseParser<String> parser;
    size_t bytes = parser.PartailParse(c_http_response);
    EXPECT_EQ(bytes, c_http_response.size());
    EXPECT_TRUE(!parser.ParseError());
    EXPECT_TRUE(parser.ParseDone());

    EXPECT_EQ(parser.GetStatus(), "OK");
    EXPECT_EQ(parser.GetStatusCode(), 200);
    EXPECT_EQ(parser.GetMajor(), 1);
    EXPECT_EQ(parser.GetMinor(), 1);
    EXPECT_EQ(parser.GetField("Accept"), "XAccept");
    EXPECT_EQ(parser.GetField("Host"), "domain.com");
    EXPECT_EQ(parser.GetField("Connection"), "Keep-Alive");
    EXPECT_EQ(parser.GetField("User-Agent"), "");
    EXPECT_EQ(parser.GetBody(), "xyz");

    for (int i = 0; i < 10; ++i) {
        size_t bytes = parser.PartailParse(c_http_response.c_str(), c_http_response.size());
        EXPECT_EQ(bytes, c_http_response.size());
        EXPECT_TRUE(!parser.ParseError());
        EXPECT_TRUE(parser.ParseDone());
    }

    EXPECT_EQ(parser.GetStatus(), "OK");
    EXPECT_EQ(parser.GetStatusCode(), 200);
    EXPECT_EQ(parser.GetMajor(), 1);
    EXPECT_EQ(parser.GetMinor(), 1);
    EXPECT_EQ(parser.GetField("Accept"), "XAccept");
    EXPECT_EQ(parser.GetField("Host"), "domain.com");
    EXPECT_EQ(parser.GetField("Connection"), "Keep-Alive");
    EXPECT_EQ(parser.GetField("User-Agent"), "");

    bytes = parser.PartailParse(c_http_response_2);
    EXPECT_EQ(bytes, c_http_response_2.size());
    EXPECT_TRUE(!parser.ParseError());
    EXPECT_TRUE(!parser.ParseDone());
    EXPECT_TRUE(parser.PartailParseEof());
    EXPECT_TRUE(!parser.ParseError());
    EXPECT_TRUE(parser.ParseDone());

    EXPECT_EQ(parser.GetStatus(), "Not Found");
    EXPECT_EQ(parser.GetStatusCode(), 404);
    EXPECT_EQ(parser.GetMajor(), 1);
    EXPECT_EQ(parser.GetMinor(), 1);
    EXPECT_EQ(parser.GetField("Accept"), "XAccept");
    EXPECT_EQ(parser.GetField("Host"), "domain.com");
    EXPECT_EQ(parser.GetField("Connection"), "");
    EXPECT_EQ(parser.GetField("User-Agent"), "gtest.proxy");

    bytes = parser.PartailParse(c_http_response_err_1);
    EXPECT_TRUE(parser.ParseError());

    bytes = parser.PartailParse(c_http_response_err_2);
    EXPECT_TRUE(parser.ParseError());

    // partail parse logic
    cout << "parse partail" << endl;
    bytes = parser.PartailParse(c_http_response_err_3);
    EXPECT_EQ(bytes, c_http_response_err_3.size());
    EXPECT_TRUE(!parser.ParseError());
    EXPECT_TRUE(!parser.ParseDone());

    bytes = parser.PartailParse("\r\n");
    EXPECT_EQ(bytes, 2);
    EXPECT_TRUE(!parser.ParseError());
    EXPECT_TRUE(parser.ParseDone());

    for (size_t pos = 0; pos < c_http_response.size(); ++pos) {
        // cout << "parse response split by " << pos << endl;
        //        cout << "first partail: " << c_http_response.substr(0, pos) << endl << endl;
        std::string fp = c_http_response.substr(0, pos);
        size_t bytes = parser.PartailParse(fp);
        // EXPECT_EQ(bytes, pos);
        // EXPECT_EQ(parser.ParseError().value(), 1);
        EXPECT_FALSE(parser.ParseDone());

        std::string sp = c_http_response.substr(bytes);
        // cout << sp << endl;
        bytes += parser.PartailParse(sp);
        EXPECT_EQ(bytes, c_http_response.size());
        EXPECT_TRUE(!parser.ParseError());
        EXPECT_TRUE(parser.ParseDone());

        EXPECT_EQ(parser.GetStatus(), "OK");
        EXPECT_EQ(parser.GetStatusCode(), 200);
        EXPECT_EQ(parser.GetMajor(), 1);
        EXPECT_EQ(parser.GetMinor(), 1);
        EXPECT_EQ(parser.GetField("Accept"), "XAccept");
        EXPECT_EQ(parser.GetField("Host"), "domain.com");
        EXPECT_EQ(parser.GetField("Connection"), "Keep-Alive");
        EXPECT_EQ(parser.GetField("User-Agent"), "");
    }

    char buf[256] = {};
    auto response = parser.plunderResponse();
    bool b = response.Serialize(buf, sizeof(buf));
    EXPECT_TRUE(b);
    bytes = response.ByteSize();
    EXPECT_EQ(bytes, c_http_response.size());
    EXPECT_EQ(c_http_response, buf);
}
#if 0
void copyto_response() {
    std::string s = c_http_response;

    rapidhttp::HttpDocumentRef parser(rapidhttp::Response);
    size_t bytes = parser.PartailParse(s);
    EXPECT_EQ(bytes, s.size());
    EXPECT_TRUE(!parser.ParseError());

#define _CHECK_DOC(parser)                                  \
    EXPECT_EQ(parser.GetStatus(), "OK");                    \
    EXPECT_EQ(parser.GetStatusCode(), 200);                 \
    EXPECT_EQ(parser.GetMajor(), 1);                        \
    EXPECT_EQ(parser.GetMinor(), 1);                        \
    EXPECT_EQ(parser.GetField("Accept"), "XAccept");        \
    EXPECT_EQ(parser.GetField("Host"), "domain.com");       \
    EXPECT_EQ(parser.GetField("Connection"), "Keep-Alive"); \
    EXPECT_EQ(parser.GetField("User-Agent"), "");           \
    EXPECT_EQ(parser.GetBody(), "xyz")

    _CHECK_DOC(parser);

    rapidhttp::HttpDocumentRef doc2(rapidhttp::Response);
    parser.CopyTo(doc2);
    _CHECK_DOC(doc2);

    rapidhttp::HttpDocument doc3(rapidhttp::Response);
    doc2.CopyTo(doc3);
    _CHECK_DOC(doc3);

    rapidhttp::HttpDocument doc4(rapidhttp::Response);
    doc2.CopyTo(doc4);
    _CHECK_DOC(doc4);
    doc3.CopyTo(doc4);
    _CHECK_DOC(doc4);

    s = "xx";
    _CHECK_DOC(doc3);
    _CHECK_DOC(doc4);
}
#endif

TEST(parser, response) {
    test_parse_response<std::string>();
    // test_parse_response<rapidhttp::HttpDocumentRef>();
    // copyto_response();
}
