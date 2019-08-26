#include "restwrapper.h"
#include <fstream>
#include <sstream>
#define PRINT_LOG [](const std::string &strLogMsg) { std::cout << strLogMsg << std::endl; }

using namespace std;
int main(int argc, char const *argv[])
{
    ifstream in1("test.file");
    istreambuf_iterator<char> begin1(in1);
    istreambuf_iterator<char> end1;
    string strPostData(begin1, end1);

    ifstream in2("test.file.ip");
    istreambuf_iterator<char> begin2(in2);
    istreambuf_iterator<char> end2;
    string ipPort(begin2, end2);
    ipPort.pop_back();
    
    // ipPort : http://{{ip}}:{{port}}
    cout << PostWrapper(ipPort, "", strPostData) << endl;
    return 0;
}
