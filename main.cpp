#include "httpclient.h"
#include <fstream>
#include <sstream>
#define PRINT_LOG [](const std::string &strLogMsg) { std::cout << strLogMsg << std::endl; }

using namespace std;
int main(int argc, char const *argv[])
{

    CppHTTPClient::HeadersMap RequestHeaders;
    CppHTTPClient::HttpResponse ServerResponse;

    CppHTTPClient pRESTClient(PRINT_LOG);
    pRESTClient.InitSession();

    ifstream in1("test.file");
    istreambuf_iterator<char> begin1(in1);
    istreambuf_iterator<char> end1;
    string strPostData(begin1, end1);

    ifstream in2("test.file.ip");
    istreambuf_iterator<char> begin2(in2);
    istreambuf_iterator<char> end2;
    string ipPort(begin2, end2);

    RequestHeaders.emplace("Content-Type", "application/json");
    if (pRESTClient.Post(ipPort, RequestHeaders, strPostData, ServerResponse))
    {
        // Print whole Response
        cout << CppHTTPClient::ParseHttpResponse(ServerResponse) << endl;
        // Print Body
        cout << ServerResponse.strBody << endl;
    }
    pRESTClient.CleanupSession();
    return 0;
}
