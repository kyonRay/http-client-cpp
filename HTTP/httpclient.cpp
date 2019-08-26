#include "httpclient.h"

// Static members initialization
volatile int CppHTTPClient::s_iCurlSession = 0;
std::string CppHTTPClient::s_strCertificationAuthorityFile;
std::mutex CppHTTPClient::s_mtxCurlSession;

/**
 * @brief constructor of the HTTP client object
 *
 * @param Logger - a callabck to a logger function void(const std::string&)
 *
 */
CppHTTPClient::CppHTTPClient(LogFnCallback Logger) : m_oLog(Logger),
                                                     m_iCurlTimeout(0),
                                                     m_bHTTPS(false),
                                                     m_bNoSignal(false),
                                                     m_eSettingsFlags(ALL_FLAGS),
                                                     m_pCurlSession(nullptr),
                                                     m_pHeaderlist(nullptr)
{
   s_mtxCurlSession.lock();
   if (s_iCurlSession++ == 0)
   {
      curl_global_init(CURL_GLOBAL_ALL);
   }
   s_mtxCurlSession.unlock();
}

/**
 * @brief destructor of the HTTP client object
 *
 */
CppHTTPClient::~CppHTTPClient()
{
   if (m_pCurlSession != nullptr)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog(LOG_WARNING_OBJECT_NOT_CLEANED);

      CleanupSession();
   }

   s_mtxCurlSession.lock();
   if (--s_iCurlSession <= 0)
   {
      curl_global_cleanup();
   }
   s_mtxCurlSession.unlock();
}

/**
 * @brief Starts a new HTTP session, initializes the cURL API session
 *
 * If a new session was already started, the method has no effect.
 *
 * @param [in] bHTTPS Enable/Disable HTTPS (disabled by default)
 * @param [in] eSettingsFlags optional use | operator to choose multiple options
 *
 * @retval true   Successfully initialized the session.
 * @retval false  The session is already initialized
 * Use CleanupSession() before initializing a new one or the Curl API is not initialized.
 *
 * Example Usage:
 * @code
 *    m_pHTTPClient->InitSession();
 * @endcode
 */
const bool CppHTTPClient::InitSession(const bool &bHTTPS /* = false */,
                                      const SettingsFlag &eSettingsFlags /* = ALL_FLAGS */)
{
   if (m_pCurlSession)
   {
      if (eSettingsFlags & ENABLE_LOG)
         m_oLog(LOG_ERROR_CURL_ALREADY_INIT_MSG);

      return false;
   }
   m_pCurlSession = curl_easy_init();

   m_bHTTPS = bHTTPS;
   m_eSettingsFlags = eSettingsFlags;

   return (m_pCurlSession != nullptr);
}

/**
 * @brief Cleans the current HTTP session
 *
 * If a session was not already started, the method has no effect
 *
 * @retval true   Successfully cleaned the current session.
 * @retval false  The session is not already initialized.
 *
 * Example Usage:
 * @code
 *    m_pHTTPClient->CleanupSession();
 * @endcode
 */
const bool CppHTTPClient::CleanupSession()
{
   if (!m_pCurlSession)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog(LOG_ERROR_CURL_NOT_INIT_MSG);

      return false;
   }

   curl_easy_cleanup(m_pCurlSession);
   m_pCurlSession = nullptr;

   if (m_pHeaderlist)
   {
      curl_slist_free_all(m_pHeaderlist);
      m_pHeaderlist = nullptr;
   }

   return true;
}

/**
 * @brief checks a URI
 * adds the proper protocol scheme (HTTP:// or HTTPS://)
 * if the URI has no protocol scheme, the added protocol scheme
 * will depend on m_bHTTPS that can be set when initializing a session
 * or with the SetHTTPS(bool)
 *
 * @param [in] strURL user URI
 */
inline void CppHTTPClient::CheckURL(const std::string &strURL)
{
   std::string strTmp = strURL;

   std::transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::toupper);

   if (strTmp.compare(0, 7, "HTTP://") == 0)
      m_bHTTPS = false;
   else if (strTmp.compare(0, 8, "HTTPS://") == 0)
      m_bHTTPS = true;
   else
   {
      m_strURL = ((m_bHTTPS) ? "https://" : "http://") + strURL;
      return;
   }
   m_strURL = strURL;
}

/**
 *  @brief performs the chosen HTTP request
 * sets up the common settings (Timeout, proxy,...)
 *
 * @retval true   Successfully performed the request.
 * @retval false  An error occured while CURL was performing the request.
 */
const CURLcode CppHTTPClient::Perform()
{
   if (!m_pCurlSession)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog(LOG_ERROR_CURL_NOT_INIT_MSG);

      return CURLE_FAILED_INIT;
   }

   CURLcode res = CURLE_OK;

   curl_easy_setopt(m_pCurlSession, CURLOPT_URL, m_strURL.c_str());

   if (m_pHeaderlist != nullptr)
      curl_easy_setopt(m_pCurlSession, CURLOPT_HTTPHEADER, m_pHeaderlist);

   curl_easy_setopt(m_pCurlSession, CURLOPT_USERAGENT, CLIENT_USERAGENT);
   curl_easy_setopt(m_pCurlSession, CURLOPT_AUTOREFERER, 1L);
   curl_easy_setopt(m_pCurlSession, CURLOPT_FOLLOWLOCATION, 1L);

   if (m_iCurlTimeout > 0)
   {
      curl_easy_setopt(m_pCurlSession, CURLOPT_TIMEOUT, m_iCurlTimeout);
      // don't want to get a sig alarm on timeout
      curl_easy_setopt(m_pCurlSession, CURLOPT_NOSIGNAL, 1);
   }

   if (m_bNoSignal)
   {
      curl_easy_setopt(m_pCurlSession, CURLOPT_NOSIGNAL, 1L);
   }

   // SSL
   if (m_bHTTPS)
   {
      curl_easy_setopt(m_pCurlSession, CURLOPT_USE_SSL, CURLUSESSL_ALL);
   }

   if (m_bHTTPS && !(m_eSettingsFlags & VERIFY_PEER))
      curl_easy_setopt(m_pCurlSession, CURLOPT_SSL_VERIFYPEER, 0L);

   if (m_bHTTPS && !(m_eSettingsFlags & VERIFY_HOST))
      curl_easy_setopt(m_pCurlSession, CURLOPT_SSL_VERIFYHOST, 0L); // use 2L for strict name check

   if (m_bHTTPS && !s_strCertificationAuthorityFile.empty())
      curl_easy_setopt(m_pCurlSession, CURLOPT_CAINFO, s_strCertificationAuthorityFile.c_str());

   if (m_bHTTPS && !m_strSSLCertFile.empty())
      curl_easy_setopt(m_pCurlSession, CURLOPT_SSLCERT, m_strSSLCertFile.c_str());

   if (m_bHTTPS && !m_strSSLKeyFile.empty())
      curl_easy_setopt(m_pCurlSession, CURLOPT_SSLKEY, m_strSSLKeyFile.c_str());

   if (m_bHTTPS && !m_strSSLKeyPwd.empty())
      curl_easy_setopt(m_pCurlSession, CURLOPT_KEYPASSWD, m_strSSLKeyPwd.c_str());

   // Perform the requested operation
   res = curl_easy_perform(m_pCurlSession);

   if (m_pHeaderlist)
   {
      curl_slist_free_all(m_pHeaderlist);
      m_pHeaderlist = nullptr;
   }

   return res;
}

// REST REQUESTS

/**
 * @brief initializes a REST request
 * some common operations to REST requests are performed here,
 * the others are performed in Perform method
 *
 * @param [in] Headers headers to send
 * @param [out] Response response data
 */
inline const bool CppHTTPClient::InitRestRequest(const std::string &strUrl,
                                                 const CppHTTPClient::HeadersMap &Headers,
                                                 CppHTTPClient::HttpResponse &Response)
{
   if (strUrl.empty())
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog(LOG_ERROR_EMPTY_HOST_MSG);

      return false;
   }
   if (!m_pCurlSession)
   {
      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog(LOG_ERROR_CURL_NOT_INIT_MSG);

      return false;
   }
   // Reset is mandatory to avoid bad surprises
   curl_easy_reset(m_pCurlSession);

   CheckURL(strUrl);

   // set the received body's callback function
   curl_easy_setopt(m_pCurlSession, CURLOPT_WRITEFUNCTION, &CppHTTPClient::RestWriteCallback);

   // set data object to pass to callback function above
   curl_easy_setopt(m_pCurlSession, CURLOPT_WRITEDATA, &Response);

   // set the response's headers processing callback function
   curl_easy_setopt(m_pCurlSession, CURLOPT_HEADERFUNCTION, &CppHTTPClient::RestHeaderCallback);

   // callback object for server's responses headers
   curl_easy_setopt(m_pCurlSession, CURLOPT_HEADERDATA, &Response);

   std::string strHeader;
   for (HeadersMap::const_iterator it = Headers.cbegin();
        it != Headers.cend();
        ++it)
   {
      strHeader = it->first + ": " + it->second; // build header string
      AddHeader(strHeader);
   }

   return true;
}

/**
 * @brief post REST request operations are performed here
 *
 * @param [in] ePerformCode curl easy perform returned code
 * @param [out] Response response data
 */
inline const bool CppHTTPClient::PostRestRequest(const CURLcode ePerformCode,
                                                 CppHTTPClient::HttpResponse &Response)
{
   // Check for errors
   if (ePerformCode != CURLE_OK)
   {
      Response.strBody.clear();
      Response.iCode = -1;

      if (m_eSettingsFlags & ENABLE_LOG)
         m_oLog(StringFormat(LOG_ERROR_CURL_REST_FAILURE_FORMAT, m_strURL.c_str(), ePerformCode,
                             curl_easy_strerror(ePerformCode)));

      return false;
   }
   long lHttpCode = 0;
   curl_easy_getinfo(m_pCurlSession, CURLINFO_RESPONSE_CODE, &lHttpCode);
   Response.iCode = static_cast<int>(lHttpCode);

   return true;
}

/**
 * @brief performs a HEAD request
 *
 * @param [in] strUrl url to request
 * @param [in] Headers headers to send
 * @param [out] Response response data
 *
 * @retval true   Successfully requested the URI.
 * @retval false  Encountered a problem.
 */
const bool CppHTTPClient::Head(const std::string &strUrl,
                               const CppHTTPClient::HeadersMap &Headers,
                               CppHTTPClient::HttpResponse &Response)
{
   if (InitRestRequest(strUrl, Headers, Response))
   {
      /** set HTTP HEAD METHOD */
      curl_easy_setopt(m_pCurlSession, CURLOPT_CUSTOMREQUEST, "HEAD");
      curl_easy_setopt(m_pCurlSession, CURLOPT_NOBODY, 1L);

      CURLcode res = Perform();

      return PostRestRequest(res, Response);
   }
   else
      return false;
}

/**
 * @brief performs a GET request
 *
 * @param [in] strUrl url to request
 * @param [in] Headers headers to send
 * @param [out] Response response data
 *
 * @retval true   Successfully requested the URI.
 * @retval false  Encountered a problem.
 */
const bool CppHTTPClient::Get(const std::string &strUrl,
                              const CppHTTPClient::HeadersMap &Headers,
                              CppHTTPClient::HttpResponse &Response)
{
   if (InitRestRequest(strUrl, Headers, Response))
   {
      // specify a GET request
      curl_easy_setopt(m_pCurlSession, CURLOPT_HTTPGET, 1L);

      CURLcode res = Perform();

      return PostRestRequest(res, Response);
   }
   else
      return false;
}

/**
 * @brief performs a DELETE request
 *
 * @param [in] strUrl url to request
 * @param [in] Headers headers to send
 * @param [out] Response response data
 *
 *  @retval true   Successfully requested the URI.
 * @retval false  Encountered a problem.
 */
const bool CppHTTPClient::Del(const std::string &strUrl,
                              const CppHTTPClient::HeadersMap &Headers,
                              CppHTTPClient::HttpResponse &Response)
{
   if (InitRestRequest(strUrl, Headers, Response))
   {
      curl_easy_setopt(m_pCurlSession, CURLOPT_CUSTOMREQUEST, "DELETE");

      CURLcode res = Perform();

      return PostRestRequest(res, Response);
   }
   else
      return false;
}

const bool CppHTTPClient::Post(const std::string &strUrl,
                               const CppHTTPClient::HeadersMap &Headers,
                               const std::string &strPostData,
                               CppHTTPClient::HttpResponse &Response)
{
   if (InitRestRequest(strUrl, Headers, Response))
   {
      // specify a POST request
      curl_easy_setopt(m_pCurlSession, CURLOPT_POST, 1L);

      // set post informations
      curl_easy_setopt(m_pCurlSession, CURLOPT_POSTFIELDS, strPostData.c_str());
      curl_easy_setopt(m_pCurlSession, CURLOPT_POSTFIELDSIZE, strPostData.size());

      CURLcode res = Perform();

      return PostRestRequest(res, Response);
   }
   else
      return false;
}

/**
 * @brief performs a PUT request with a string
 *
 * @param [in] strUrl url to request
 * @param [in] Headers headers to send
 * @param [out] Response response data
 *
 * @retval true   Successfully requested the URI.
 * @retval false  Encountered a problem.
 */
const bool CppHTTPClient::Put(const std::string &strUrl, const CppHTTPClient::HeadersMap &Headers,
                              const std::string &strPutData, CppHTTPClient::HttpResponse &Response)
{
   if (InitRestRequest(strUrl, Headers, Response))
   {
      CppHTTPClient::UploadObject Payload;

      Payload.pszData = strPutData.c_str();
      Payload.usLength = strPutData.size();

      // specify a PUT request
      curl_easy_setopt(m_pCurlSession, CURLOPT_PUT, 1L);
      curl_easy_setopt(m_pCurlSession, CURLOPT_UPLOAD, 1L);

      // set read callback function
      curl_easy_setopt(m_pCurlSession, CURLOPT_READFUNCTION, &CppHTTPClient::RestReadCallback);
      // set data object to pass to callback function
      curl_easy_setopt(m_pCurlSession, CURLOPT_READDATA, &Payload);

      // set data size
      curl_easy_setopt(m_pCurlSession, CURLOPT_INFILESIZE, static_cast<long>(Payload.usLength));

      CURLcode res = Perform();

      return PostRestRequest(res, Response);
   }
   else
      return false;
}

/**
 * @brief performs a PUT request with a byte buffer (vector of char)
 *
 * @param [in] strUrl url to request
 * @param [in] Headers headers to send
 * @param [out] Response response data
 *
 * @retval true   Successfully requested the URI.
 * @retval false  Encountered a problem.
 */
const bool CppHTTPClient::Put(const std::string &strUrl, const CppHTTPClient::HeadersMap &Headers,
                              const CppHTTPClient::ByteBuffer &Data, CppHTTPClient::HttpResponse &Response)
{
   if (InitRestRequest(strUrl, Headers, Response))
   {
      CppHTTPClient::UploadObject Payload;

      Payload.pszData = Data.data();
      Payload.usLength = Data.size();

      // specify a PUT request
      curl_easy_setopt(m_pCurlSession, CURLOPT_PUT, 1L);
      curl_easy_setopt(m_pCurlSession, CURLOPT_UPLOAD, 1L);

      // set read callback function
      curl_easy_setopt(m_pCurlSession, CURLOPT_READFUNCTION, &CppHTTPClient::RestReadCallback);
      // set data object to pass to callback function
      curl_easy_setopt(m_pCurlSession, CURLOPT_READDATA, &Payload);

      // set data size
      curl_easy_setopt(m_pCurlSession, CURLOPT_INFILESIZE, static_cast<long>(Payload.usLength));

      CURLcode res = Perform();

      return PostRestRequest(res, Response);
   }
   else
      return false;
}

std::string CppHTTPClient::ParseHttpResponse(const CppHTTPClient::HttpResponse &Resonse)
{
   std::string re = "{";
   re += "\"Status-Code\":" + std::to_string(Resonse.iCode);
   re += ", \"Headers\":[{";
   for (auto it = Resonse.mapHeaders.cbegin(); it != Resonse.mapHeaders.cend(); it++)
   {
      re += "\"" + it->first + "\":" + "\"" + it->second + "\",";
   }
   re.pop_back();
   re += "}],";
   re += "\"Body\":" + Resonse.strBody + "}";
   return re;
}

// STRING HELPERS

/**
 * @brief returns a formatted string
 *
 * @param [in] strFormat string with one or many format specifiers
 * @param [in] parameters to be placed in the format specifiers of strFormat
 *
 * @retval string formatted string
 */
std::string CppHTTPClient::StringFormat(const std::string strFormat, ...)
{
   int n = (static_cast<int>(strFormat.size())) * 2; // Reserve two times as much as the length of the strFormat

   std::unique_ptr<char[]> pFormatted;

   va_list ap;

   while (true)
   {
      pFormatted.reset(new char[n]); // Wrap the plain char array into the unique_ptr
      strcpy(&pFormatted[0], strFormat.c_str());

      va_start(ap, strFormat);
      int iFinaln = vsnprintf(&pFormatted[0], n, strFormat.c_str(), ap);
      va_end(ap);

      if (iFinaln < 0 || iFinaln >= n)
      {
         n += abs(iFinaln - n + 1);
      }
      else
      {
         break;
      }
   }

   return std::string(pFormatted.get());
}
/*std::string CppHTTPClient::StringFormat(const std::string strFormat, ...)
{
   char buffer[1024];
   va_list args;
   va_start(args, strFormat);
   vsnprintf(buffer, 1024, strFormat.c_str(), args);
   va_end (args);
   return std::string(buffer);
}
*/

/**
 * @brief removes leading and trailing whitespace from a string
 *
 * @param [in/out] str string to be trimmed
 */
inline void CppHTTPClient::TrimSpaces(std::string &str)
{
   // trim from left
   str.erase(str.begin(),
             std::find_if(str.begin(), str.end(), [](char c) { return !isspace(c); }));

   // trim from right
   str.erase(std::find_if(str.rbegin(), str.rend(), [](char c) { return !isspace(c); }).base(),
             str.end());
}

// CURL CALLBACKS
// REST CALLBACKS

/**
 * @brief write callback function for libcurl
 * this callback will be called to store the server's Body reponse
 * in a struct response
 *
 * we can also use an std::vector<char> instead of an std::string but in this case
 * there isn't a big difference... maybe resizing the container with a max size can
 * enhance performances...
 *
 * @param data returned data of size (size*nmemb)
 * @param size size parameter
 * @param nmemb memblock parameter
 * @param userdata pointer to user data to save/work with return data
 *
 * @return (size * nmemb)
 */
size_t CppHTTPClient::RestWriteCallback(void *pCurlData, size_t usBlockCount, size_t usBlockSize, void *pUserData)
{
   CppHTTPClient::HttpResponse *pServerResponse;
   pServerResponse = reinterpret_cast<CppHTTPClient::HttpResponse *>(pUserData);
   pServerResponse->strBody.append(reinterpret_cast<char *>(pCurlData), usBlockCount * usBlockSize);

   return (usBlockCount * usBlockSize);
}

/**
 * @brief header callback for libcurl
 * callback used to process response's headers (received)
 *
 * @param data returned (header line)
 * @param size of data
 * @param nmemb memblock
 * @param userdata pointer to user data object to save header data
 * @return size * nmemb;
 */
size_t CppHTTPClient::RestHeaderCallback(void *pCurlData, size_t usBlockCount, size_t usBlockSize, void *pUserData)
{
   CppHTTPClient::HttpResponse *pServerResponse;
   pServerResponse = reinterpret_cast<CppHTTPClient::HttpResponse *>(pUserData);

   std::string strHeader(reinterpret_cast<char *>(pCurlData), usBlockCount * usBlockSize);
   size_t usSeperator = strHeader.find_first_of(":");
   if (std::string::npos == usSeperator)
   {
      //roll with non seperated headers or response's line
      TrimSpaces(strHeader);
      if (0 == strHeader.length())
      {
         return (usBlockCount * usBlockSize); //blank line;
      }
      pServerResponse->mapHeaders[strHeader] = "present";
   }
   else
   {
      std::string strKey = strHeader.substr(0, usSeperator);
      TrimSpaces(strKey);
      std::string strValue = strHeader.substr(usSeperator + 1);
      TrimSpaces(strValue);
      pServerResponse->mapHeaders[strKey] = strValue;
   }

   return (usBlockCount * usBlockSize);
}

/**
 * @brief read callback function for libcurl
 * used to send (or upload) a content to the server
 *
 * @param pointer of max size (size*nmemb) to write data to (used by cURL to send data)
 * @param size size parameter
 * @param nmemb memblock parameter
 * @param userdata pointer to user data to read data from
 *
 * @return (size * nmemb)
 */
size_t CppHTTPClient::RestReadCallback(void *pCurlData, size_t usBlockCount, size_t usBlockSize, void *pUserData)
{
   // get upload struct
   CppHTTPClient::UploadObject *Payload;

   Payload = reinterpret_cast<CppHTTPClient::UploadObject *>(pUserData);

   // set correct sizes
   size_t usCurlSize = usBlockCount * usBlockSize;
   size_t usCopySize = (Payload->usLength < usCurlSize) ? Payload->usLength : usCurlSize;

   /** copy data to buffer */
   std::memcpy(pCurlData, Payload->pszData, usCopySize);

   // decrement length and increment data pointer
   Payload->usLength -= usCopySize; // remaining bytes to be sent
   Payload->pszData += usCopySize;  // next byte to the chunk that will be sent

   /** return copied size */
   return usCopySize;
}
