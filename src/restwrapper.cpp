#include "restwrapper.h"

#define PRINT_LOG [](const std::string &strLogMsg) { std::cout << strLogMsg << std::endl; }

/// Resonse parse to JSON
const std::string ParseHttpResponse(const CppHTTPClient::HttpResponse &re)
{
    rapidjson::Document document;
    rapidjson::Document::AllocatorType &alloc = document.GetAllocator();
    rapidjson::Value root(rapidjson::kObjectType);
    rapidjson::Value rootMap(rapidjson::kObjectType);
    rapidjson::Value key(rapidjson::kStringType);
    rapidjson::Value valueInt(rapidjson::kNumberType);
    rapidjson::Value valueString(rapidjson::kStringType);

    key.SetString("Status-Code", alloc);
    valueInt.SetInt(re.iCode);
    root.AddMember(key, valueInt, alloc);

    for (CppHTTPClient::HeadersMap::const_iterator it = re.mapHeaders.cbegin(); it != re.mapHeaders.cend(); it++)
    {
        key.SetString(it->first.c_str(), alloc);
        valueString.SetString(it->second.c_str(), alloc);
        rootMap.AddMember(key, valueString, alloc);
    }
    key.SetString("Header", alloc);
    root.AddMember(key, rootMap, alloc);
    key.SetString("Body", alloc);
    valueString.SetString(re.strBody.c_str(), alloc);
    root.AddMember(key, valueString, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    root.Accept(writer);
    return buffer.GetString();
}

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

bool ParseJSON2HeadersMap(const std::string &str, CppHTTPClient::HeadersMap &headermap)
{
    if (str.length() == 0)
        return false;
    rapidjson::Document d;
    if (d.Parse(str.c_str()).HasParseError())
    {
        fprintf(stderr, "JSON Parse Error(offset %u): %s\n",
                (unsigned)d.GetErrorOffset(),
                GetParseError_En(d.GetParseError()));
        return false;
    }
    else if (!d.HasMember("Header"))
    {
        cout << "JSON string: " << str << endl;
        cout << "JSON Parse Error: JSON string dose not have member called \"Header\"." << endl;
        return false;
    }
    else
    {
        rapidjson::Value &header = d["Header"];
        for (rapidjson::Value::MemberIterator iter = header.MemberBegin(); iter != header.MemberEnd(); iter++)
        {
            const char *key = iter->name.GetString();
            const rapidjson::Value &val = iter->value;
            headermap.emplace(string(key), string(val.GetString()));
        }
    }
    return true;
}

const std::string PostWrapper(const std::string &strUrl,
                              const std::string &extraHeadersJSON,
                              const std::string &strPostDataJSON)
{
    CppHTTPClient::HeadersMap RequestHeaders;
    CppHTTPClient::HttpResponse ServerResponse;
    if (!extraHeadersJSON.empty() && extraHeadersJSON.length() != 0)
        if (!ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders))
            return "";
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
            re = ParseHttpResponse(ServerResponse);
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
    if (!extraHeadersJSON.empty() && extraHeadersJSON.length() != 0)
        if (!ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders))
            return "";
    std::string re = "";
    CppHTTPClient pRESTClient(PRINT_LOG);

    pRESTClient.InitSession();

    if (pRESTClient.Get(strUrl, RequestHeaders, ServerResponse))
    {
        re = ParseHttpResponse(ServerResponse);
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
    if (!extraHeadersJSON.empty() && extraHeadersJSON.length() != 0)
        if (!ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders))
            return "";
    std::string re = "";
    CppHTTPClient pRESTClient(PRINT_LOG);

    pRESTClient.InitSession();

    if (pRESTClient.Head(strUrl, RequestHeaders, ServerResponse))
    {
        re = ParseHttpResponse(ServerResponse);
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
    if (!extraHeadersJSON.empty() && extraHeadersJSON.length() != 0)
        if (!ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders))
            return "";
    std::string re = "";
    CppHTTPClient pRESTClient(PRINT_LOG);

    pRESTClient.InitSession();

    if (pRESTClient.Del(strUrl, RequestHeaders, ServerResponse))
    {
        re = ParseHttpResponse(ServerResponse);
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
    if (!extraHeadersJSON.empty() && extraHeadersJSON.length() != 0)
        if (!ParseJSON2HeadersMap(extraHeadersJSON, RequestHeaders))
            return "";
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
            re = ParseHttpResponse(ServerResponse);
        }
        else
        {
            cout << "PutWrapper: PUT Progress Error." << endl;
        }
    }
    pRESTClient.CleanupSession();
    return re;
}
