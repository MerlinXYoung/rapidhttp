#include <rapidhttp/rapidhttp.h>
#include <unistd.h>

#include <iostream>
using namespace std;

void parse() {
    static std::string c_http_request =
        "POST /uri/abc HTTP/1.1\r\n"
        "Accept: XAccept\r\n"
        "Host: domain.com\r\n"
        "User-Agent: gtest.proxy\r\n"
        "Content-Length: 3\r\n"
        "\r\nabc";

    // 1.定义一个Document对象
    rapidhttp::Parser parser(rapidhttp::HTTP_REQUEST);

    // 2.调用PartailParse解析数据流, 接口参数是
    // std::string
    // 或 (const char*, size_t)
    parser.PartailParse(c_http_request);

    // 3.判断解析是否出错
    if (parser.ParseError()) {
        // 打印错误描述信息
        cout << "parse error:" << parser.ParseError().message() << endl;
        return;
    }

    // 4.判断解析是否完成
    if (parser.ParseDone()) {
        cout << "parse not done." << endl;
    }
}

void partail_parse() {
    static std::string c_http_request =
        "POST /uri/abc HTTP/1.1\r\n"
        "Accept: XAccept\r\n"
        "Host: domain.com\r\n"
        "User-Agent: gtest.proxy\r\n"
        "Content-Length: 3\r\n"
        "\r\nabc";

    // 1.定义一个Document对象
    rapidhttp::Parser parser(rapidhttp::HTTP_REQUEST);

    // 2.调用PartailParse解析数据流的一部分
    int pos = 27;
    size_t bytes = parser.PartailParse(c_http_request.c_str(), pos);
    (void)bytes;
    // 3.判断解析是否出错
    if (parser.ParseError()) {
        // 打印错误描述信息
        cout << "parse error:" << parser.ParseError().message() << endl;
        return;
    }

    // 4.判断解析是否完成
    if (!parser.ParseDone()) {
        // 未完成, 继续解析剩余部分
        bytes += parser.PartailParse(c_http_request.c_str() + pos, c_http_request.size() - pos);
    }

    // 4.判断解析是否完成
    if (parser.ParseDone()) {
        cout << "parse not done." << endl;
    }
}

/// 一种更加快速的解析方法: HttpDocumentRef
void fast_parse() {
    std::string c_http_request =
        "POST /uri/abc HTTP/1.1\r\n"
        "Accept: XAccept\r\n"
        "Host: domain.com\r\n"
        "User-Agent: gtest.proxy\r\n"
        "Content-Length: 3\r\n"
        "\r\nabc";

    // 1.定义一个HttpDocumentRef对象
    rapidhttp::RefParser parser(rapidhttp::HTTP_REQUEST);

    // 2.调用PartailParse解析数据流, 接口参数是
    // std::string
    // 或 (const char*, size_t)
    parser.PartailParse(c_http_request);

    // 3.判断解析是否出错
    if (parser.ParseError()) {
        // 打印错误描述信息
        cout << "parse error:" << parser.ParseError().message() << endl;
        return;
    }

    // 4.判断解析是否完成
    if (parser.ParseDone()) {
        cout << "parse not done." << endl;
    }

    // 5.使用HttpDocumentRef时, 不保存数据内容, 只是对数据流的引用.
    // 所以当数据流失效前, 如果要保存解析结果, 需要Copy到HttpDocument对象中.
    // rapidhttp::HttpDocument storage_doc(rapidhttp::Request);
    // parser.CopyTo(storage_doc);
    rapidhttp::Document storage_doc(parser.StealDoc());

    // 6.让缓冲区失效
    c_http_request = "";

    // 7.此时doc已经不能用了, storage_doc还有效
    cout << "Valid DOM: " << storage_doc.GetBody() << endl;
}

int main() {
    parse();
    partail_parse();
    fast_parse();
    return 0;
}
