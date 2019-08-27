#ifndef RESTWRAPPER_H
#define RESTWRAPPER_H
#include "httpclient.h"
#include "document.h"     // rapidjson's DOM-style API
#include "prettywriter.h" // for stringify JSON
#include "stringbuffer.h"
#include "writer.h"
#include "error/en.h"

#include <string>
#include <unordered_map>

using namespace std;

const std::string PostWrapper(const std::string &strUrl,
                              const std::string &extraHeadersJSON,
                              const std::string &strPostDataJSOn);

const std::string GetWrapper(const std::string &strUrl,
                             const std::string &extraHeadersJSON);

const std::string HeadWrapper(const std::string &strUrl,
                              const std::string &extraHeadersJSON);

const std::string DelWrapper(const std::string &strUrl,
                             const std::string &extraHeadersJSON);

const std::string PutWrapper(const std::string &strUrl,
                             const std::string &Headers,
                             const std::string &strPutDataJSON);
#endif