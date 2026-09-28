
// controller/EchoController.cc

#include "EchoController.h"
#include "utils/ResponseBuilder.h"
#include "validation/Validator.h"

using namespace faststore::utils;
using namespace faststore::validation;

// Cria o tipo função de uma chamada
 using Callback =
     std::function<void(const drogon::HttpResponsePtr &)>;


 void EchoController::echo(const HttpRequestPtr &req, Callback &&callback)
 {
     auto jsonBody = req->getJsonObject();
     if (!jsonBody)
     {
         callback(ResponseBuilder::error(
             "INVALID_JSON", "Request body must be valid JSON", k400BadRequest));
         return;
     };
     Validator v;
     v.required(*jsonBody, "message").minLength(*jsonBody, "message", 1);
     if (v.hasErrors())
     {
         callback(ResponseBuilder::validationFailed(v.issues()));
         return;
     }
     Json::Value data;
     data["echoed"] = (*jsonBody)["message"];
     callback(ResponseBuilder::success(data));
 }