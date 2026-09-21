// main.cpp
/*
Configure o projeto:

  cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

Compile:

  cmake --build build

*/
#include <drogon/drogon.h>
#include <json/json.h>

int main()
{
  //auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
  // Register the port and address we listen on.
  // 0.0.0.0 binds to all interfaces; fine for local dev, we'll
  // tighten this in Chapter 11's security pass.
  drogon::app()
      .addListener("0.0.0.0", 8080);
  // Use multiple I/O threads to take advantage of every core.
  // std::thread::hardware_concurrency() falls back sensibly if the
  // platform can't report core count.
  drogon::app().setThreadNum(std::max(2u, std::thread::hardware_concurrency()));
  // Basic logging so we can see the server come up.
  LOG_INFO
      << "Starting FastStore API on port 8080...";
  drogon::app().run();
  return 0;
}