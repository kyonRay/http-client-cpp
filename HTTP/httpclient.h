#pragma once

#define CLIENT_USERAGENT "CppHTTPClient-agent/0.1"

#include <algorithm>
#include <cstring>
#include <vector>
#include <curl/curl.h>
#include <functional>
#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <cstdarg>

class CppHTTPClient
{
public:
   // Public definitions
   typedef std::function<int(void*, double, double, double, double)> ProgressFnCallback;
   typedef std::function<void(const std::string&)>                   LogFnCallback;
   typedef std::unordered_map<std::string, std::string>              HeadersMap;
   typedef std::vector<char> ByteBuffer;

   // HTTP response data
   struct HttpResponse
   {
      HttpResponse() : iCode(0) {}
      int iCode; // HTTP response code
      HeadersMap mapHeaders; // HTTP response headers fields
      std::string strBody; // HTTP response body
   };

   enum SettingsFlag
   {
      NO_FLAGS = 0x00,
      ENABLE_LOG = 0x01,
      VERIFY_PEER = 0x02,
      VERIFY_HOST = 0x04,
      ALL_FLAGS = 0xFF
   };

   /* Please provide your logger thread-safe routine, otherwise, you can turn off
   * error log messages printing by not using the flag ALL_FLAGS or ENABLE_LOG */
   explicit CppHTTPClient(LogFnCallback oLogger);
   virtual ~CppHTTPClient();

   // copy constructor and assignment operator are disabled
   CppHTTPClient(const CppHTTPClient& Copy) = delete;
   CppHTTPClient& operator=(const CppHTTPClient& Copy) = delete;

   // Setters - Getters (just for unit tests)
   inline void SetTimeout(const int& iTimeout) { m_iCurlTimeout = iTimeout; }
   inline void SetNoSignal(const bool& bNoSignal) { m_bNoSignal = bNoSignal; }
   inline void SetHTTPS(const bool& bEnableHTTPS) { m_bHTTPS = bEnableHTTPS; }
   inline const int GetTimeout() const { return m_iCurlTimeout; }
   inline const bool GetNoSignal() const { return m_bNoSignal; }
   inline const std::string& GetURL()      const { return m_strURL; }
   inline const unsigned char GetSettingsFlags() const { return m_eSettingsFlags; }
   inline const bool GetHTTPS() const { return m_bHTTPS; }

   // Session
   const bool InitSession(const bool& bHTTPS = false,
                          const SettingsFlag& SettingsFlags = ALL_FLAGS);
   const bool CleanupSession();

   static int GetCurlSessionCount() { return s_iCurlSession; }
   const CURL* GetCurlPointer() const { return m_pCurlSession; }

   // HTTP requests
   inline void AddHeader(const std::string& strHeader)
   {
      m_pHeaderlist = curl_slist_append(m_pHeaderlist, strHeader.c_str());
   }

   // REST requests
   const bool Head(const std::string& strUrl, const HeadersMap& Headers, HttpResponse& Response);
   const bool Get(const std::string& strUrl, const HeadersMap& Headers, HttpResponse& Response);
   const bool Del(const std::string& strUrl, const HeadersMap& Headers, HttpResponse& Response);
   const bool Post(const std::string& strUrl, const HeadersMap& Headers,
             const std::string& strPostData, HttpResponse& Response);
   const bool Put(const std::string& strUrl, const HeadersMap& Headers,
            const std::string& strPutData, HttpResponse& Response);
   const bool Put(const std::string& strUrl, const HeadersMap& Headers,
            const ByteBuffer& Data, HttpResponse& Response);
   
   // SSL certs
   static const std::string& GetCertificateFile() { return s_strCertificationAuthorityFile; }
   static void SetCertificateFile(const std::string& strPath) { s_strCertificationAuthorityFile = strPath; }

   void SetSSLCertFile(const std::string& strPath) { m_strSSLCertFile = strPath; }
   const std::string& GetSSLCertFile() const { return m_strSSLCertFile; }

   void SetSSLKeyFile(const std::string& strPath) { m_strSSLKeyFile = strPath; }
   const std::string& GetSSLKeyFile() const { return m_strSSLKeyFile; }

   void SetSSLKeyPassword(const std::string& strPwd) { m_strSSLKeyPwd = strPwd; }
   const std::string& GetSSLKeyPwd() const { return m_strSSLKeyPwd; }

protected:
   // payload to upload on POST requests.
   struct UploadObject
   {
      UploadObject() : pszData(nullptr), usLength(0) {}
      const char* pszData; // data to upload
      size_t usLength; // length of the data to upload
   };

   /* common operations are performed here */
   inline const CURLcode Perform();
   inline void CheckURL(const std::string& strURL);
   inline const bool InitRestRequest(const std::string& strUrl, const HeadersMap& Headers,
                               HttpResponse& Response);
   inline const bool PostRestRequest(const CURLcode ePerformCode, HttpResponse& Response);

   // Curl callbacks
   static size_t WriteInStringCallback(void* ptr, size_t size, size_t nmemb, void* data);
   static size_t ThrowAwayCallback(void* ptr, size_t size, size_t nmemb, void* data);
   static size_t RestWriteCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
   static size_t RestHeaderCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
   static size_t RestReadCallback(void* ptr, size_t size, size_t nmemb, void* userdata);
   
   // String Helpers
   static std::string StringFormat(const std::string strFormat, ...);
   static inline void TrimSpaces(std::string& str);

   std::string          m_strURL;

   bool                 m_bNoSignal;
   bool                 m_bHTTPS;
   SettingsFlag         m_eSettingsFlags;

   struct curl_slist*    m_pHeaderlist;

   // SSL
   static std::string   s_strCertificationAuthorityFile;
   std::string          m_strSSLCertFile;
   std::string          m_strSSLKeyFile;
   std::string          m_strSSLKeyPwd;
   
   static std::mutex     s_mtxCurlSession; // mutex used to manage API global operations
   volatile static int   s_iCurlSession;   // Count of the actual sessions

   CURL*         m_pCurlSession;
   int           m_iCurlTimeout;

   // Log printer callback
   LogFnCallback         m_oLog;

};

// Logs messages
#define LOG_ERROR_EMPTY_HOST_MSG                "[CppHTTPClient][Error] Empty hostname."
#define LOG_WARNING_OBJECT_NOT_CLEANED          "[CppHTTPClient][Warning] Object was freed before calling CHTTPClient::CleanupSession(). The API session was cleaned though."
#define LOG_ERROR_CURL_ALREADY_INIT_MSG         "[CppHTTPClient][Error] Curl session is already initialized ! Use CleanupSession() to clean the present one."
#define LOG_ERROR_CURL_NOT_INIT_MSG             "[CppHTTPClient][Error] Curl session is not initialized ! Use InitSession() before."

#define LOG_ERROR_CURL_REST_FAILURE_FORMAT      "[CppHTTPClient][Error] Unable to perform a REST request from '%s' (Error = %d | %s)"