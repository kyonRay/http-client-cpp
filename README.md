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

1. 使用 `CppHTTPClient` 类进行HTTP请求

```c++
#include "httpclient.h"

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

2. 使用封装的REST方式进行HTTP请求

```c++
#include "restwrapper.h"

std::string strPostData = "{\"DATA\":\"TEST\"}";
cout << PostWrapper("http://192.168.0.0:8800", "", strPostData) << endl;
```

输出：

```JSON
{
    "Status-Code":200,
    "Headers":[{
        "Content-Type":"application/json;charset=UTF-8",
        "Transfer-Encoding":"chunked",
        ...
        }],
    "Body":{
        ...
    }
}
```

## 编译安装

```shell
mkdir build
cd build
cmake ..
make
```
