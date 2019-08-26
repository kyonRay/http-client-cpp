#include "gtest/gtest.h" // Google Test Framework


#include <mutex>
#include <thread>

#include "stringbuffer.h"
#include "writer.h"
#include "document.h"     // rapidjson's DOM-style API
#include "prettywriter.h" // for stringify JSON
#include "httpclient.h"

#define PRINT_LOG [](const std::string &strLogMsg) { std::cout << strLogMsg << std::endl; }

std::mutex g_mtxConsoleMutex;

namespace
{
// Fixture for HTTP tests
class HTTPTest : public ::testing::Test
{
protected:
   std::unique_ptr<CppHTTPClient> m_pHTTPClient;

   HTTPTest() : m_pHTTPClient(nullptr)
   {
   }

   virtual ~HTTPTest() {}

   virtual void SetUp()
   {
      m_pHTTPClient.reset(new CppHTTPClient(PRINT_LOG));

      /* to enable HTTPS, use the proper scheme in the URI 
       * or use SetHTTPS(true) */
      m_pHTTPClient->InitSession();
   }

   virtual void TearDown()
   {
      if (m_pHTTPClient.get() != nullptr)
      {
         m_pHTTPClient->CleanupSession();
         m_pHTTPClient.reset();
      }
   }
};

// Fixture for REST tests
class RestClientTest : public ::testing::Test
{
protected:
   std::unique_ptr<CppHTTPClient> m_pRESTClient;
   CppHTTPClient::HeadersMap m_mapHeader;
   CppHTTPClient::HttpResponse m_Response;

   RestClientTest() : m_pRESTClient(nullptr)
   {
      m_mapHeader.emplace("User-Agent", CLIENT_USERAGENT);
   }

   virtual ~RestClientTest() {}

   virtual void SetUp()
   {
      m_pRESTClient.reset(new CppHTTPClient(PRINT_LOG));

      m_pRESTClient->InitSession();
   }

   virtual void TearDown()
   {
      m_pRESTClient->CleanupSession();
      m_pRESTClient.reset();
   }
};

// Unit tests

// Tests without a fixture (testing setters/getters and init./cleanup session)
TEST(HTTPClient, TestSession)
{
   CppHTTPClient HTTPClient(PRINT_LOG);

   EXPECT_TRUE(HTTPClient.GetURL().empty());

   EXPECT_TRUE(HTTPClient.GetSSLCertFile().empty());
   EXPECT_TRUE(HTTPClient.GetSSLKeyFile().empty());
   EXPECT_TRUE(HTTPClient.GetSSLKeyPwd().empty());

   EXPECT_FALSE(HTTPClient.GetNoSignal());
   EXPECT_FALSE(HTTPClient.GetHTTPS());

   EXPECT_EQ(0, HTTPClient.GetTimeout());

   EXPECT_TRUE(HTTPClient.GetCurlPointer() == nullptr);

   EXPECT_EQ(CppHTTPClient::SettingsFlag::ALL_FLAGS, HTTPClient.GetSettingsFlags());

   /* after init. a session */
   ASSERT_TRUE(HTTPClient.InitSession(true, CppHTTPClient::ENABLE_LOG));

   // check that the parameters provided to InitSession
   EXPECT_EQ(CppHTTPClient::ENABLE_LOG, HTTPClient.GetSettingsFlags());
   EXPECT_TRUE(HTTPClient.GetHTTPS());

   EXPECT_TRUE(HTTPClient.GetCurlPointer() != nullptr);

   HTTPClient.SetSSLCertFile("file.cert");
   HTTPClient.SetSSLKeyFile("key.key");
   HTTPClient.SetSSLKeyPassword("passphrase");
   HTTPClient.SetTimeout(10);
   HTTPClient.SetHTTPS(false);
   HTTPClient.SetNoSignal(true);

   EXPECT_FALSE(HTTPClient.GetHTTPS());
   EXPECT_TRUE(HTTPClient.GetNoSignal());
   EXPECT_STREQ("file.cert", HTTPClient.GetSSLCertFile().c_str());
   EXPECT_STREQ("key.key", HTTPClient.GetSSLKeyFile().c_str());
   EXPECT_STREQ("passphrase", HTTPClient.GetSSLKeyPwd().c_str());
   EXPECT_EQ(10, HTTPClient.GetTimeout());

   EXPECT_TRUE(HTTPClient.CleanupSession());
}

TEST(HTTPClient, TestDoubleInitializingSession)
{
   CppHTTPClient HTTPClient(PRINT_LOG);

   ASSERT_TRUE(HTTPClient.InitSession());
   EXPECT_FALSE(HTTPClient.InitSession());
   EXPECT_TRUE(HTTPClient.CleanupSession());
}

TEST(HTTPClient, TestDoubleCleanUp)
{
   CppHTTPClient HTTPClient(PRINT_LOG);

   ASSERT_TRUE(HTTPClient.InitSession());
   EXPECT_TRUE(HTTPClient.CleanupSession());
   EXPECT_FALSE(HTTPClient.CleanupSession());
}

TEST(HTTPClient, TestCleanUpWithoutInit)
{
   CppHTTPClient HTTPClient(PRINT_LOG);

   ASSERT_FALSE(HTTPClient.CleanupSession());
}

TEST(HTTPClient, TestMultithreading)
{
   const char *arrDataArray[3] = {"Thread 1", "Thread 2", "Thread 3"};
   unsigned uInitialCount = CppHTTPClient::GetCurlSessionCount();

   auto ThreadFunction = [](const char *pszThreadName) {
      CppHTTPClient HTTPClient([](const std::string &strMsg) { std::cout << strMsg << std::endl; });
      if (pszThreadName != nullptr)
      {
         g_mtxConsoleMutex.lock();
         std::cout << pszThreadName << std::endl;
         g_mtxConsoleMutex.unlock();
      }
   };

   std::thread FirstThread(ThreadFunction, arrDataArray[0]);
   std::thread SecondThread(ThreadFunction, arrDataArray[1]);
   std::thread ThirdThread(ThreadFunction, arrDataArray[2]);

   // synchronize threads
   FirstThread.join();  // pauses until first finishes
   SecondThread.join(); // pauses until second finishes
   ThirdThread.join();  // pauses until third finishes

   ASSERT_EQ(uInitialCount, CppHTTPClient::GetCurlSessionCount());
}

/* REST tests */

// HEAD Tests
// check return code
TEST_F(RestClientTest, TestRestClientHeadCode)
{
   EXPECT_TRUE(m_pRESTClient->Head("http://httpbin.org/get", m_mapHeader, m_Response));
   EXPECT_EQ(200, m_Response.iCode);
   EXPECT_TRUE(m_Response.strBody.empty());
   EXPECT_FALSE(m_Response.mapHeaders.empty());
}

// GET Tests
TEST_F(RestClientTest, TestRestClientGETCode)
{
   EXPECT_TRUE(m_pRESTClient->Get("http://httpbin.org/get", m_mapHeader, m_Response));
   EXPECT_EQ(200, m_Response.iCode);
}
TEST_F(RestClientTest, TestRestClientGETBodyCode)
{
   ASSERT_TRUE(m_pRESTClient->Get("http://httpbin.org/get", m_mapHeader, m_Response));

   rapidjson::Document document;
   std::vector<char> Resp(m_Response.strBody.c_str(),
                          m_Response.strBody.c_str() + m_Response.strBody.size() + 1);
   ASSERT_FALSE(document.ParseInsitu(&Resp[0]).HasParseError());

   rapidjson::Value::MemberIterator itTokenUrl = document.FindMember("url");
   ASSERT_TRUE(itTokenUrl != document.MemberEnd());
   ASSERT_TRUE(itTokenUrl->value.IsString());
   EXPECT_STREQ("https://httpbin.org/get", itTokenUrl->value.GetString());

   rapidjson::Value::MemberIterator itTokenHeaders = document.FindMember("headers");
   ASSERT_TRUE(itTokenHeaders != document.MemberEnd());
   ASSERT_TRUE(itTokenHeaders->value.IsObject());
   rapidjson::Value::MemberIterator itTokenAgent = itTokenHeaders->value.FindMember("User-Agent");
   ASSERT_TRUE(itTokenAgent != itTokenHeaders->value.MemberEnd());
   EXPECT_STREQ(CLIENT_USERAGENT, itTokenAgent->value.GetString());
}

// check for failure
TEST_F(RestClientTest, TestRestClientGETFailureCode)
{
   std::string strInvalidUrl = "http://nonexistent";

   EXPECT_FALSE(m_pRESTClient->Get(strInvalidUrl, m_mapHeader, m_Response));
   EXPECT_TRUE(m_Response.strBody.empty());
   EXPECT_EQ(-1, m_Response.iCode);
}

TEST_F(RestClientTest, TestRestClientGETHeaders)
{
   ASSERT_TRUE(m_pRESTClient->Get("http://httpbin.org/get", m_mapHeader, m_Response));
   ASSERT_TRUE(m_Response.mapHeaders.find("Connection") != m_Response.mapHeaders.end());
   EXPECT_EQ("keep-alive", m_Response.mapHeaders["Connection"]);
}

TEST_F(RestClientTest, TestRestClientAuth)
{
   ASSERT_TRUE(m_pRESTClient->Get("http://foo:bar@httpbin.org/basic-auth/foo/bar", m_mapHeader, m_Response));
   ASSERT_EQ(200, m_Response.iCode);

   rapidjson::Document document;
   std::vector<char> Resp(m_Response.strBody.c_str(),
                          m_Response.strBody.c_str() + m_Response.strBody.size() + 1);
   ASSERT_FALSE(document.ParseInsitu(&Resp[0]).HasParseError());

   rapidjson::Value::MemberIterator itTokenUser = document.FindMember("user");
   ASSERT_TRUE(itTokenUser != document.MemberEnd());
   ASSERT_TRUE(itTokenUser->value.IsString());
   EXPECT_STREQ("foo", itTokenUser->value.GetString());

   rapidjson::Value::MemberIterator itTokenAuth = document.FindMember("authenticated");
   ASSERT_TRUE(itTokenAuth != document.MemberEnd());
   ASSERT_TRUE(itTokenAuth->value.IsBool());
   EXPECT_EQ(true, itTokenAuth->value.GetBool());

   ASSERT_TRUE(m_pRESTClient->Get("http://httpbin.org/basic-auth/foo/bar", m_mapHeader, m_Response));
   ASSERT_EQ(401, m_Response.iCode);
}

// POST Tests
// check return code
TEST_F(RestClientTest, TestRestClientPOSTCode)
{
   std::string strPostData = "data";
   m_mapHeader.emplace("Content-Type", "text/text");

   EXPECT_TRUE(m_pRESTClient->Post("http://httpbin.org/post", m_mapHeader, strPostData, m_Response));
   EXPECT_EQ(200, m_Response.iCode);
}
TEST_F(RestClientTest, TestRestClientPOSTBody)
{
   m_mapHeader.emplace("Content-Type", "text/text");

   ASSERT_TRUE(m_pRESTClient->Post("http://httpbin.org/post", m_mapHeader, "data", m_Response));

   rapidjson::Document document;
   std::vector<char> Resp(m_Response.strBody.c_str(),
                          m_Response.strBody.c_str() + m_Response.strBody.size() + 1);
   ASSERT_FALSE(document.ParseInsitu(&Resp[0]).HasParseError());

   rapidjson::Value::MemberIterator itTokenUrl = document.FindMember("url");
   ASSERT_TRUE(itTokenUrl != document.MemberEnd());
   ASSERT_TRUE(itTokenUrl->value.IsString());
   EXPECT_STREQ("https://httpbin.org/post", itTokenUrl->value.GetString());

   rapidjson::Value::MemberIterator itTokenHeaders = document.FindMember("headers");
   ASSERT_TRUE(itTokenHeaders != document.MemberEnd());
   ASSERT_TRUE(itTokenHeaders->value.IsObject());
   rapidjson::Value::MemberIterator itTokenAgent = itTokenHeaders->value.FindMember("User-Agent");
   ASSERT_TRUE(itTokenAgent != itTokenHeaders->value.MemberEnd());
   EXPECT_STREQ(CLIENT_USERAGENT, itTokenAgent->value.GetString());
}
// check for failure
TEST_F(RestClientTest, TestRestClientPOSTFailureCode)
{
   std::string strInvalidUrl = "http://nonexistent";
   m_mapHeader.emplace("Content-Type", "text/text");

   EXPECT_FALSE(m_pRESTClient->Post(strInvalidUrl, m_mapHeader, "data", m_Response));
   EXPECT_EQ(-1, m_Response.iCode);
}
TEST_F(RestClientTest, TestRestClientPOSTHeaders)
{
   m_mapHeader.emplace("Content-Type", "text/text");

   ASSERT_TRUE(m_pRESTClient->Post("http://httpbin.org/post", m_mapHeader, "data", m_Response));
   ASSERT_TRUE(m_Response.mapHeaders.find("Connection") != m_Response.mapHeaders.end());
   EXPECT_EQ("keep-alive", m_Response.mapHeaders["Connection"]);
}

// PUT Tests
// check return code
TEST_F(RestClientTest, TestRestClientPUTString)
{
   std::string strPutData = "data";
   m_mapHeader.emplace("Content-Type", "text/text");

   EXPECT_TRUE(m_pRESTClient->Put("http://httpbin.org/put", m_mapHeader, strPutData, m_Response));
   EXPECT_EQ(200, m_Response.iCode);

   rapidjson::Document document;
   std::vector<char> Resp(m_Response.strBody.c_str(),
                          m_Response.strBody.c_str() + m_Response.strBody.size() + 1);
   ASSERT_FALSE(document.ParseInsitu(&Resp[0]).HasParseError());

   rapidjson::Value::MemberIterator itTokenUrl = document.FindMember("url");
   ASSERT_TRUE(itTokenUrl != document.MemberEnd());
   ASSERT_TRUE(itTokenUrl->value.IsString());
   EXPECT_STREQ("https://httpbin.org/put", itTokenUrl->value.GetString());

   rapidjson::Value::MemberIterator itTokenHeaders = document.FindMember("headers");
   ASSERT_TRUE(itTokenHeaders != document.MemberEnd());
   ASSERT_TRUE(itTokenHeaders->value.IsObject());
   rapidjson::Value::MemberIterator itTokenAgent = itTokenHeaders->value.FindMember("User-Agent");
   ASSERT_TRUE(itTokenAgent != itTokenHeaders->value.MemberEnd());
   EXPECT_STREQ(CLIENT_USERAGENT, itTokenAgent->value.GetString());
}

TEST_F(RestClientTest, TestRestClientPUTBuffer)
{
   CppHTTPClient::ByteBuffer vecBuffer; // std::vector<char>
   vecBuffer.reserve(4);
   vecBuffer.push_back('d');
   vecBuffer.push_back('a');
   vecBuffer.push_back('t');
   vecBuffer.push_back('a');

   m_mapHeader.emplace("Content-Type", "text/text");

   ASSERT_TRUE(m_pRESTClient->Put("http://httpbin.org/put", m_mapHeader, vecBuffer, m_Response));
   ASSERT_EQ(200, m_Response.iCode);

   rapidjson::Document document;
   std::vector<char> Resp(m_Response.strBody.c_str(),
                          m_Response.strBody.c_str() + m_Response.strBody.size() + 1);
   ASSERT_FALSE(document.ParseInsitu(&Resp[0]).HasParseError());

   rapidjson::Value::MemberIterator itTokenUrl = document.FindMember("url");
   ASSERT_TRUE(itTokenUrl != document.MemberEnd());
   ASSERT_TRUE(itTokenUrl->value.IsString());
   EXPECT_STREQ("https://httpbin.org/put", itTokenUrl->value.GetString());

   rapidjson::Value::MemberIterator itTokenHeaders = document.FindMember("headers");
   ASSERT_TRUE(itTokenHeaders != document.MemberEnd());
   ASSERT_TRUE(itTokenHeaders->value.IsObject());
   rapidjson::Value::MemberIterator itTokenAgent = itTokenHeaders->value.FindMember("User-Agent");
   ASSERT_TRUE(itTokenAgent != itTokenHeaders->value.MemberEnd());
   EXPECT_STREQ(CLIENT_USERAGENT, itTokenAgent->value.GetString());
}

// check for failure
TEST_F(RestClientTest, TestRestClientPUTFailureCode)
{
   std::string strInvalidUrl = "http://nonexistent";
   m_mapHeader.emplace("Content-Type", "text/text");

   EXPECT_FALSE(m_pRESTClient->Put(strInvalidUrl, m_mapHeader, "data", m_Response));
   EXPECT_EQ(-1, m_Response.iCode);
}
TEST_F(RestClientTest, TestRestClientPUTHeaders)
{
   m_mapHeader.emplace("Content-Type", "text/text");

   ASSERT_TRUE(m_pRESTClient->Put("http://httpbin.org/put", m_mapHeader, "data", m_Response));
   ASSERT_TRUE(m_Response.mapHeaders.find("Connection") != m_Response.mapHeaders.end());
   EXPECT_EQ("keep-alive", m_Response.mapHeaders["Connection"]);
}

// DELETE Tests
// check return code
TEST_F(RestClientTest, TestRestClientDeleteCode)
{
   EXPECT_TRUE(m_pRESTClient->Del("http://httpbin.org/delete", m_mapHeader, m_Response));
   EXPECT_EQ(200, m_Response.iCode);
}
TEST_F(RestClientTest, TestRestClientDeleteBody)
{
   ASSERT_TRUE(m_pRESTClient->Del("http://httpbin.org/delete", m_mapHeader, m_Response));

   rapidjson::Document document;
   std::vector<char> Resp(m_Response.strBody.c_str(),
                          m_Response.strBody.c_str() + m_Response.strBody.size() + 1);
   ASSERT_FALSE(document.ParseInsitu(&Resp[0]).HasParseError());

   rapidjson::Value::MemberIterator itTokenUrl = document.FindMember("url");
   ASSERT_TRUE(itTokenUrl != document.MemberEnd());
   ASSERT_TRUE(itTokenUrl->value.IsString());
   EXPECT_STREQ("https://httpbin.org/delete", itTokenUrl->value.GetString());

   rapidjson::Value::MemberIterator itTokenHeaders = document.FindMember("headers");
   ASSERT_TRUE(itTokenHeaders != document.MemberEnd());
   ASSERT_TRUE(itTokenHeaders->value.IsObject());
   rapidjson::Value::MemberIterator itTokenAgent = itTokenHeaders->value.FindMember("User-Agent");
   ASSERT_TRUE(itTokenAgent != itTokenHeaders->value.MemberEnd());
   EXPECT_STREQ(CLIENT_USERAGENT, itTokenAgent->value.GetString());
}

// check for failure
TEST_F(RestClientTest, TestRestClientDeleteFailureCode)
{
   EXPECT_FALSE(m_pRESTClient->Del("http://nonexistent", m_mapHeader, m_Response));
   EXPECT_EQ(-1, m_Response.iCode);
}
TEST_F(RestClientTest, TestRestClientDeleteHeaders)
{
   ASSERT_TRUE(m_pRESTClient->Del("http://httpbin.org/delete", m_mapHeader, m_Response));
   ASSERT_TRUE(m_Response.mapHeaders.find("Connection") != m_Response.mapHeaders.end());
   EXPECT_STREQ("keep-alive", m_Response.mapHeaders["Connection"].c_str());
}

} // namespace

int main(int argc, char **argv)
{
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
