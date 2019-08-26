# http-client-cpp

一个使用C++实现的简单http客户端

## 依赖

项目依赖

- [libcurl](http://curl.haxx.se/libcurl/)：一个开源且易于使用的客户端URL传输库，支持绝大部分协议传输，且跨平台。

测试依赖

- [googletest](https://github.com/google/googletest) ：Google测试和模拟框架；
- [rapidjson](https://github.com/Tencent/rapidjson) ：高效的C++ JSON解析／生成器，提供SAX及DOM风格API；

构建依赖

- CMake
- GNU GCC/Clang （支持C++14标准）

## 使用举例

```c++
CppHTTPClient::HeadersMap RequestHeaders;
CppHTTPClient::HttpResponse ServerResponse;

CppHTTPClient pRESTClient(PRINT_LOG);
pRESTClient.InitSession();

std::string strPostData = "{\"DATA\":\"TEST\"}";

RequestHeaders.emplace("Content-Type", "application/json");
if (pRESTClient.Post("http://196.128.0.0:8800", RequestHeaders, strPostData, ServerResponse))
{

    cout << ServerResponse.iCode << endl;
    cout << ServerResponse.strBody << endl;
}
pRESTClient.CleanupSession();
```

## 编译安装

```shell
mkdir build
cd build
cmake ..
make
```
