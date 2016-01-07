////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_HTTP_SERVER_APPLICATION_ENDPOINT_SERVER_H
#define ARANGOD_HTTP_SERVER_APPLICATION_ENDPOINT_SERVER_H 1

#include "Basics/Common.h"

#include "ApplicationServer/ApplicationFeature.h"

#include <openssl/ssl.h>

#include "Rest/EndpointList.h"
#include "HttpServer/HttpHandlerFactory.h"


namespace triagens {
namespace rest {
class ApplicationDispatcher;
class ApplicationScheduler;
class AsyncJobManager;
class HttpServer;


////////////////////////////////////////////////////////////////////////////////
/// @brief application http server feature
////////////////////////////////////////////////////////////////////////////////

class ApplicationEndpointServer : public ApplicationFeature {
 private:
  ApplicationEndpointServer(ApplicationEndpointServer const&);
  ApplicationEndpointServer& operator=(ApplicationEndpointServer const&);

  
 public:
  ////////////////////////////////////////////////////////////////////////////////
  /// @brief constructor
  ////////////////////////////////////////////////////////////////////////////////

  ApplicationEndpointServer(ApplicationServer*, ApplicationScheduler*,
                            ApplicationDispatcher*, AsyncJobManager*,
                            std::string const&,
                            HttpHandlerFactory::context_fptr, void*);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief destructor
  ////////////////////////////////////////////////////////////////////////////////

  ~ApplicationEndpointServer();

  
 public:
  ////////////////////////////////////////////////////////////////////////////////
  /// @brief builds the servers
  ////////////////////////////////////////////////////////////////////////////////

  bool buildServers();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief get the handler factory
  ////////////////////////////////////////////////////////////////////////////////

  HttpHandlerFactory* getHandlerFactory() const { return _handlerFactory; }

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief set the server basepath
  ////////////////////////////////////////////////////////////////////////////////

  void setBasePath(std::string const& basePath) { _basePath = basePath; }

  
  ////////////////////////////////////////////////////////////////////////////////
  /// {@inheritDoc}
  ////////////////////////////////////////////////////////////////////////////////

  void setupOptions(std::map<std::string, basics::ProgramOptionsDescription>&);

  ////////////////////////////////////////////////////////////////////////////////
  /// {@inheritDoc}
  ////////////////////////////////////////////////////////////////////////////////

  bool afterOptionParsing(basics::ProgramOptions&);

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief return a list of all endpoints
  ////////////////////////////////////////////////////////////////////////////////

  std::map<std::string, std::vector<std::string>> getEndpoints();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief restores the endpoint list
  ////////////////////////////////////////////////////////////////////////////////

  bool loadEndpoints();

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief get the list of databases for an endpoint
  ////////////////////////////////////////////////////////////////////////////////

  std::vector<std::string> const& getEndpointMapping(std::string const&);

  ////////////////////////////////////////////////////////////////////////////////
  /// {@inheritDoc}
  ////////////////////////////////////////////////////////////////////////////////

  bool prepare();

  ////////////////////////////////////////////////////////////////////////////////
  /// {@inheritDoc}
  ////////////////////////////////////////////////////////////////////////////////

  bool open();

  ////////////////////////////////////////////////////////////////////////////////
  /// {@inheritDoc}
  ////////////////////////////////////////////////////////////////////////////////

  void close();

  ////////////////////////////////////////////////////////////////////////////////
  /// {@inheritDoc}
  ////////////////////////////////////////////////////////////////////////////////

  void stop();

  
 private:
  ////////////////////////////////////////////////////////////////////////////////
  /// @brief return the filename for the endpoints file
  ////////////////////////////////////////////////////////////////////////////////

  std::string getEndpointsFilename() const;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief creates an ssl context
  ////////////////////////////////////////////////////////////////////////////////

  bool createSslContext();

  
 protected:
  ////////////////////////////////////////////////////////////////////////////////
  /// @brief application server
  ////////////////////////////////////////////////////////////////////////////////

  ApplicationServer* _applicationServer;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief application scheduler
  ////////////////////////////////////////////////////////////////////////////////

  ApplicationScheduler* _applicationScheduler;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief application dispatcher or null
  ////////////////////////////////////////////////////////////////////////////////

  ApplicationDispatcher* _applicationDispatcher;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief application job manager
  ////////////////////////////////////////////////////////////////////////////////

  AsyncJobManager* _jobManager;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief authentication realm
  ////////////////////////////////////////////////////////////////////////////////

  std::string _authenticationRealm;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief set context callback function
  ////////////////////////////////////////////////////////////////////////////////

  HttpHandlerFactory::context_fptr _setContext;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief context data passed to callback functions
  ////////////////////////////////////////////////////////////////////////////////

  void* _contextData;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief the handler factory
  ////////////////////////////////////////////////////////////////////////////////

  HttpHandlerFactory* _handlerFactory;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief all constructed servers
  ////////////////////////////////////////////////////////////////////////////////

  std::vector<HttpServer*> _servers;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief server basepath
  ////////////////////////////////////////////////////////////////////////////////

  std::string _basePath;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief endpoint list container
  ////////////////////////////////////////////////////////////////////////////////

  rest::EndpointList _endpointList;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief deprecated hidden option for downwards compatibility
  ////////////////////////////////////////////////////////////////////////////////

  std::string _httpPort;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverEndpoint
////////////////////////////////////////////////////////////////////////////////

  std::vector<std::string> _endpoints;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverReuseAddress
////////////////////////////////////////////////////////////////////////////////

  bool _reuseAddress;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock keep_alive_timeout
////////////////////////////////////////////////////////////////////////////////

  double _keepAliveTimeout;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverDefaultApi
////////////////////////////////////////////////////////////////////////////////

  int32_t _defaultApiCompatibility;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverAllowMethod
////////////////////////////////////////////////////////////////////////////////

  bool _allowMethodOverride;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverBacklog
////////////////////////////////////////////////////////////////////////////////

  int _backlogSize;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverKeyfile
////////////////////////////////////////////////////////////////////////////////

  std::string _httpsKeyfile;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverCafile
////////////////////////////////////////////////////////////////////////////////

  std::string _cafile;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverSSLProtocol
////////////////////////////////////////////////////////////////////////////////

  uint32_t _sslProtocol;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverSSLCache
////////////////////////////////////////////////////////////////////////////////

  bool _sslCache;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverSSLOptions
////////////////////////////////////////////////////////////////////////////////

  uint64_t _sslOptions;

////////////////////////////////////////////////////////////////////////////////
/// @brief was docuBlock serverSSLCipher
////////////////////////////////////////////////////////////////////////////////

  std::string _sslCipherList;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief ssl context
  ////////////////////////////////////////////////////////////////////////////////

  SSL_CTX* _sslContext;

  ////////////////////////////////////////////////////////////////////////////////
  /// @brief random string used for initialization
  ////////////////////////////////////////////////////////////////////////////////

  std::string _rctx;
};
}
}

#endif


