# HTTP REST Client

HTTP REST Client提供了简化的访问RESTful Service的方法。使用者通过Client发起HTTP请求，并最终将RESTful Service执行的结果返回。

## 部署REST Client

### 1. 环境要求

需要基础的C++编译环境以及CMake、Makefile等C++项目构建工具。此外，项目源码依赖于`libcurl`代码库，在类Unix环境下使用包管理工具下载`curl`工具即可：

```shell
# Debian APT
$ apt-get install curl
# Red Hat YUM
$ yum install curl
# MacOSX Homebrew
$ brew install curl
```

| 类型         | **物料**                                                     | **版本**                     | **说明**                 |
| ------------ | ------------------------------------------------------------ | ---------------------------- | ------------------------ |
| **系统要求** | CentOS/Ubuntu/MacOSX                                         | 7.2/ 16.04/10.14             | 编译部署REST Client      |
| **代码依赖** | libcurl                                                      | 7.4+                         | Curl库，用于发起HTTP请求 |
| **构建依赖** | GNU gcc/Apple clang                                          | gcc 5.0.0+/Apple clang 10.0+ | 编译C++ 14标准源码       |
| **构建依赖** | CMake                                                        | 3.5.0+                       | 项目自动化构建工具       |
| **构建依赖** | GNU Makefile                                                 | 3.8+                         | 项目自动化构建工具       |

### 2. 编译源代码与快速使用举例

首先从GitHub中下载REST Client源码，并进行编译以生成静态库文件：

```shell
$ git clone https://github.com/kyonRay/http-client-cpp.git
$ cd http-client-cpp
$ mkdir -p build && cd build
$ cmake ..
$ make
```

编译之后会在`./build/lib/`目录下有`libcpprestclient.a` 静态库文件。`libcpprestclient.a` 开放了REST形式的HTTP请求的方法，接口使用方法在[使用REST Client](##使用REST Client) 章节中可见。

此外，在`./build/bin/` 目录下有简单的`POST`方法测试二进制文件`main`，具体实现如下：

```c++
#include "restwrapper.h"
#include <fstream>
#include <sstream>

using namespace std;
int main(int argc, char const *argv[])
{
  	// read file to strPostData
    ifstream in1("test.file");
    istreambuf_iterator<char> begin1(in1);
    istreambuf_iterator<char> end1;
    string strPostData(begin1, end1);
		
  	// read file to ipPort
    ifstream in2("test.file.ip");
    istreambuf_iterator<char> begin2(in2);
    istreambuf_iterator<char> end2;
    string ipPort(begin2, end2);

    // ipPort : http://{{ip}}:{{port}}
    cout << PostWrapper(ipPort, "", strPostData) << endl;
    return 0;
}

```

在二进制文件`main`所在目录下，创建`test.file` 文件：

```json
{
    "somethingJSON": {
        "DATA":"DATA"
    }
}
```

与`test.file.ip`文件（此处的host和port应该设置为HTTP RESTful Service 真实的服务端口）：

```http
http://192.168.0.1:20191/
```

## 使用REST Client

在上一章的第2小节中，举例了`POST`方法的使用实现，当外部程序需要调用 `REST Client` 的时候，只需要引用头文件包`./include/` 中的头文件，并在链接时将 `./build/lib/libcpprestclient.a` 静态库链接进二进制文件即可进行使用。

### REST Client API

#### 1. 总体介绍

具体地，每个API的参数都大致相同：

```c++
const std::string functionName(const std::string &strUrl,
                               const std::string &extraHeaderJSON,
                               const std::string $strDataJSON   // optional
                              )
```

- 输入参数：

  - strUrl：输入为访问的URL，具体形式例如：http://192.168.0.0:20191/ ，该接口暂不支持 HTTPS协议；

  - extraHeaderJSON：输入为额外的HTTP Header设置，具体形式为JSON格式，例如：

    ```json
    {
      "Header":{
        "Transfer-Encoding":"chunked"
      }
    }
    ```

    需要注意的是，在本接口中默认已设置Header：`"Content-Type":"application/json;charset=UTF-8"` ，无需额外加入。若无额外Header需要设置，置为 `""` 即可。

  - strDataJSON：输入为需要传输的实体数据，表现形式为JSON格式，例如：

    ```json
    {
        "somethingJSON": {
            "DATA":"DATA"
        }
    }
    ```

- 输出结果：

  输出结果为JSON格式，有`Status-Code`、`Header`、`Body` 三个字段，形式如：

  ```json
  {
    "Status-Code":200,
    "Header":{
        "Transfer-Encoding":"chunked"
    },
    "Body":"{\"DATA\":\"DATA\"}"
  }
  
  ```

  值得注意的是，Body字段对应的值是**字符串**。

#### 2. POST方法

输入参数：

| 参数名          | 参数类型            | 参数说明           |
| --------------- | ------------------- | ------------------ |
| strUrl          | const std::string & | URL                |
| extraHeaderJSON | const std::string & | 请求额外Header设定 |
| strPostDataJSON | const std::string & | POST实体数据       |

返回字段说明：

| 字段名      | 字段说明                       |
| ----------- | ------------------------------ |
| Status-Code | HTTP返回状态码                 |
| Header      | HTTP返回报文头部               |
| Body        | HTTP返回报文实体，类型为字符串 |

返回示例：

```json
{
  "Status-Code":200,
  "Header":{
    "Content-Type":"application/json;charset=UTF-8",
    "...":"..."
  },
  "Body":"{\"XXX\":\"YYY\"}"
}

```

接口使用示例：

```c++
string url = "http://192.168.0.0:20191/";
string extraHeader = "";
string strPostDataJSON = "{ \"DATA\":\"DATA\" }";

string result = PostWrapper(url, extraHeader, strPostDataJSON);

```

#### 3. GET方法

输入参数：

| 参数名          | 参数类型            | 参数说明           |
| --------------- | ------------------- | ------------------ |
| strUrl          | const std::string & | URL                |
| extraHeaderJSON | const std::string & | 请求额外Header设定 |

返回字段说明：

| 字段名      | 字段说明                       |
| ----------- | ------------------------------ |
| Status-Code | HTTP返回状态码                 |
| Header      | HTTP返回报文头部               |
| Body        | HTTP返回报文实体，类型为字符串 |

返回示例：

```json
{
  "Status-Code":200,
  "Header":{
    "Content-Type":"application/json;charset=UTF-8",
    "...":"..."
  },
  "Body":"{\"XXX\":\"YYY\"}"
}

```

接口使用示例：

```c++
string url = "http://192.168.0.0:20191/";
string extraHeader = "";

string result = GetWrapper(url, extraHeader);

```

#### 4. HEAD方法

输入参数：

| 参数名          | 参数类型            | 参数说明           |
| --------------- | ------------------- | ------------------ |
| strUrl          | const std::string & | URL                |
| extraHeaderJSON | const std::string & | 请求额外Header设定 |

返回字段说明：

| 字段名      | 字段说明                       |
| ----------- | ------------------------------ |
| Status-Code | HTTP返回状态码                 |
| Header      | HTTP返回报文头部               |
| Body        | HTTP返回报文实体，类型为字符串 |

返回示例：

```json
{
  "Status-Code":200,
  "Header":{
    "Content-Type":"application/json;charset=UTF-8",
    "...":"..."
  },
  "Body":"{\"XXX\":\"YYY\"}"
}

```

接口使用示例：

```c++
string url = "http://192.168.0.0:20191/";
string extraHeader = "";

string result = HeadWrapper(url, extraHeader);

```

#### 5. DELETE方法

输入参数：

| 参数名          | 参数类型            | 参数说明           |
| --------------- | ------------------- | ------------------ |
| strUrl          | const std::string & | URL                |
| extraHeaderJSON | const std::string & | 请求额外Header设定 |

返回字段说明：

| 字段名      | 字段说明                       |
| ----------- | ------------------------------ |
| Status-Code | HTTP返回状态码                 |
| Header      | HTTP返回报文头部               |
| Body        | HTTP返回报文实体，类型为字符串 |

返回示例：

```json
{
  "Status-Code":200,
  "Header":{
    "Content-Type":"application/json;charset=UTF-8",
    "...":"..."
  },
  "Body":"{\"XXX\":\"YYY\"}"
}

```

接口使用示例：

```c++
string url = "http://192.168.0.0:20191/";
string extraHeader = "";

string result = DelWrapper(url, extraHeader);

```

#### 6. PUT方法（目前只支持JSON String）

输入参数：

| 参数名          | 参数类型            | 参数说明           |
| --------------- | ------------------- | ------------------ |
| strUrl          | const std::string & | URL                |
| extraHeaderJSON | const std::string & | 请求额外Header设定 |
| strPutDataJSON  | const std::string & | PUT实体数据        |

返回字段说明：

| 字段名      | 字段说明                       |
| ----------- | ------------------------------ |
| Status-Code | HTTP返回状态码                 |
| Header      | HTTP返回报文头部               |
| Body        | HTTP返回报文实体，类型为字符串 |

返回示例：

```json
{
  "Status-Code":200,
  "Header":{
    "Content-Type":"application/json;charset=UTF-8",
    "...":"..."
  },
  "Body":"{\"XXX\":\"YYY\"}"
}

```

接口使用示例：

```c++
string url = "http://192.168.0.0:20191/";
string extraHeader = "";
string strPutDataJSON = "{\"DATA\":\"DATA\"}"

string result = PutWrapper(url, extraHeader, strPutDataJSON);

```

## 代码结构

```shell
http-client-cpp
├── CMakeLists.txt
├── README.md
├── example					 # an example
│   └── main.cpp
├── include					 # head files
│   ├── httpclient.h
│   ├── rapidjson
│   └── restwrapper.h
└── src								# source code
    ├── CMakeLists.txt
    ├── httpclient.cpp
    └── restwrapper.cpp


```
