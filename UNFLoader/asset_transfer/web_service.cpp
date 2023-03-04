#include "web_service.hpp"

#define _WIN32_WINNT 0x0601
#include <restinio/all.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace framework64::asset_transfer {

void WebService::run() {
using namespace restinio;
    auto router = std::make_unique<router::express_router_t<>>();
    router->http_post(
            "/static_mesh",
            [this](auto req, auto params) {
                json req_json = json::parse(req->body());
                auto const & file_path = req_json["file"].get<std::string>();
                std::cout << "Processing Static Mesh: " << file_path;
                auto result = this->processor.processStaticMesh(file_path);
                return req->create_response()
                        .set_body(file_path)
                        .done();
            });

    router->non_matched_request_handler(
            [](auto req){
                return req->create_response(restinio::status_not_found()).connection_close().done();
            });

    // Launching a server with custom traits.
    struct my_server_traits : public default_single_thread_traits_t {
        using request_handler_t = restinio::router::express_router_t<>;
    };

    restinio::run(
            restinio::on_this_thread<my_server_traits>()
                    .address("localhost")
                    .port(8080)
                    .request_handler(std::move(router)));
}


}