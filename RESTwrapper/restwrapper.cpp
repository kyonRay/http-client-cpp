#include <string>
#include <unordered_map>
#include "restwrapper.h"
#include "document.h"     // rapidjson's DOM-style API
#include "prettywriter.h" // for stringify JSON
#include "stringbuffer.h"
#include "writer.h"
#include "error/en.h"
#define PRINT_LOG [](const std::string &strLogMsg) { std::cout << strLogMsg << std::endl; }

inline bool checkJSONCorrectness(const std::string &str)
{
    rapidjson::Document d;
    bool flag = true;
    if (d.Parse(str.c_str()).HasParseError())
    {
        fprintf(stderr, "JSON Parse Error(offset %u): %s\n",
                (unsigned)d.GetErrorOffset(),
                GetParseError_En(d.GetParseError()));
        flag = false;
    }
    return flag;
}

void ParseJSON2HeadersMap(const std::string &str, CppHTTPClient::HeadersMap &headermap)
{
    if (str.length() == 0)
        return;
    rapidjson::Document d;
    if (d.Parse(str.c_str()).HasParseError())
    {
        fprintf(stderr, "JSON Parse Error(offset %u): %s\n",
                (unsigned)d.GetErrorOffset(),
                GetParseError_En(d.GetParseError()));
        return;
    }
    else if (!d.HasMember("Header"))
    {
        cout << "JSON Parse Error: JSON string dose not have member called \"Header\"." << endl;
    }
    else
    {
        for (rapidjson::Value::MemberIterator iter = d.MemberBegin(); iter != d.MemberEnd(); iter++)
        {
            const char *key = iter->name.GetString();
            const rapidjson::Value &val = iter->value;
            headermap.emplace(string(key), string(val.GetString()));
        }
    }
    return;
}

const std::string PostWrapper(const std::string &strUrl,
                              const std::string &extraHeadersJSON,
                              const std::string &strPostDataJSON)
{
    CppHTTPClient::HeadersMap RequestHeaders;
    CppHTTPClient::HttpResponse ServerResponse;
    if (extraHeadersJSON != "")
        ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders);
    std::string re = "";
    CppHTTPClient pRESTClient(PRINT_LOG);

    pRESTClient.InitSession();
    RequestHeaders.emplace("Content-Type", "application/json");
    if (!checkJSONCorrectness(strPostDataJSON))
    {
        cout << "PostWrapper: strPostDataJSON JSON Parse Error" << endl;
    }
    else
    {
        if (pRESTClient.Post(strUrl, RequestHeaders, strPostDataJSON, ServerResponse))
        {
            re = CppHTTPClient::ParseHttpResponse(ServerResponse);
        }
        else
        {
            cout << "PostWrapper: POST Progress Error." << endl;
        }
    }
    pRESTClient.CleanupSession();
    return re;
}

const std::string GetWrapper(const std::string &strUrl,
                             const std::string &extraHeadersJSON)
{
    CppHTTPClient::HeadersMap RequestHeaders;
    CppHTTPClient::HttpResponse ServerResponse;
    ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders);
    std::string re = "";
    CppHTTPClient pRESTClient(PRINT_LOG);

    pRESTClient.InitSession();

    if (pRESTClient.Get(strUrl, RequestHeaders, ServerResponse))
    {
        re = CppHTTPClient::ParseHttpResponse(ServerResponse);
    }
    else
    {
        cout << "GetWrapper: GET Progress Error." << endl;
    }
    pRESTClient.CleanupSession();
    return re;
}

const std::string HeadWrapper(const std::string &strUrl,
                              const std::string &extraHeadersJSON)
{
    CppHTTPClient::HeadersMap RequestHeaders;
    CppHTTPClient::HttpResponse ServerResponse;
    ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders);
    std::string re = "";
    CppHTTPClient pRESTClient(PRINT_LOG);

    pRESTClient.InitSession();

    if (pRESTClient.Head(strUrl, RequestHeaders, ServerResponse))
    {
        re = CppHTTPClient::ParseHttpResponse(ServerResponse);
    }
    else
    {
        cout << "HeadWrapper: HEAD Progress Error." << endl;
    }
    pRESTClient.CleanupSession();

    return re;
}

const std::string DelWrapper(const std::string &strUrl,
                             const std::string &extraHeadersJSON)
{
    CppHTTPClient::HeadersMap RequestHeaders;
    CppHTTPClient::HttpResponse ServerResponse;
    ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders);
    std::string re = "";
    CppHTTPClient pRESTClient(PRINT_LOG);

    pRESTClient.InitSession();

    if (pRESTClient.Del(strUrl, RequestHeaders, ServerResponse))
    {
        re = CppHTTPClient::ParseHttpResponse(ServerResponse);
    }
    else
    {
        cout << "DelWrapper: DEL Progress Error." << endl;
    }
    pRESTClient.CleanupSession();

    return re;
}

const std::string PutWrapper(const std::string &strUrl,
                             const std::string &extraHeadersJSON,
                             const std::string &strPutDataJSON)
{
    CppHTTPClient::HeadersMap RequestHeaders;
    CppHTTPClient::HttpResponse ServerResponse;
    ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders);
    std::string re = "";
    CppHTTPClient pRESTClient(PRINT_LOG);

    pRESTClient.InitSession();
    RequestHeaders.emplace("Content-Type", "application/json");
    if (!checkJSONCorrectness(strPutDataJSON))
    {
        cout << "PutWrapper: strPostDataJSON JSON Parse Error" << endl;
    }
    else
    {
        if (pRESTClient.Put(strUrl, RequestHeaders, strPutDataJSON, ServerResponse))
        {
            re = CppHTTPClient::ParseHttpResponse(ServerResponse);
        }
        else
        {
            cout << "PutWrapper: PUT Progress Error." << endl;
        }
    }
    pRESTClient.CleanupSession();
    return re;
}
