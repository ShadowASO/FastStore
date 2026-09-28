// main.cpp
/*
Configure o projeto:

  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

Compile:

  cmake --build build

*/
#include <drogon/drogon.h>
#include "routes/Routes.h"
#include "config/DatabaseConfig.h"
#include "config/AppConfig.h"

//namespace config=faststore::config;
//using namespace faststore::config;

int main()
{
  
  initialize(".env");
  
  drogon::app().addDbClient(
      drogon::orm::PostgresConfig{
          .host = appConfig().get("DB_HOST"),
          .port = static_cast<unsigned short>(std::stoi(appConfig().get("DB_PORT"))),
          .databaseName = appConfig().get("DB_NAME"),
          .username = appConfig().get("DB_USER"),
          .password = appConfig().get("DB_PASSWORD"),
          .connectionNumber = static_cast<unsigned short>(std::stoi(appConfig().get("DB_POOL_SIZE"))),
          
          .name = "default",
          .isFast = false,
          .characterSet = "utf8",
          .timeout = -1.0,
          .autoBatch = false});

  //std::cout << dbConfig.host << " " << dbConfig.port << std::endl;
  std::cout << appConfig().get("DB_HOST") << " " << appConfig().get("DB_PORT") << std::endl;

  // std::cout << dbConfig.user << " " << dbConfig.password << std::endl;
  // auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
  //  Register the port and address we listen on.
  //  0.0.0.0 binds to all interfaces; fine for local dev, we'll
  //  tighten this in Chapter 11's security pass.
  drogon::app()
      .addListener("0.0.0.0", 8080);
  // Use multiple I/O threads to take advantage of every core.
  // std::thread::hardware_concurrency() falls back sensibly if the
  // platform can't report core count.
  drogon::app().setThreadNum(std::max(2u, std::thread::hardware_concurrency()));

  // Faz o registro de todas as rotas
  faststore::routes::registerAll();

  // Basic logging so we can see the server come up.
  LOG_INFO << "Starting FastStore API on port 8080..." << "Threads em uso: ";
  LOG_INFO << "Threads em uso: " << std::thread::hardware_concurrency();
  //LOG_INFO << "Conectado ao database: " << dbConfig.dbName << "@" << dbConfig.host;
   
 
  drogon::app().run();

  return 0;
}