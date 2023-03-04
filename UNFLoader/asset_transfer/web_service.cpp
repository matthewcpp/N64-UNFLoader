#include "web_service.hpp"

#define _WIN32_WINNT 0x0601
#include <restinio/all.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace framework64::asset_transfer {

class WebService::Handler {
public:
    Handler(Processor & p) : processor(p) {}
public:
    auto processStaticMesh( const restinio::request_handle_t& req, restinio::router::route_params_t ) const {
        json req_json = json::parse(req->body());
        auto const & file_path = req_json["file"].get<std::string>();

        auto result = processor.processStaticMesh(file_path);
        return req->create_response().set_body(file_path).done();
    }

    auto nonMatchedRequest(const restinio::request_handle_t& req) {
        return req->create_response(restinio::status_not_found()).connection_close().done();

    }

private:
    Processor & processor;
};

void WebService::run() {
    auto router = std::make_unique<restinio::router::express_router_t<>>();
    router->http_post("/static_mesh", std::bind(&Handler::processStaticMesh, handler.get(), std::placeholders::_1, std::placeholders::_2));
    router->non_matched_request_handler(std::bind(&Handler::nonMatchedRequest, handler.get(), std::placeholders::_1));

    // Launching a server with custom traits.
    struct my_server_traits : public restinio::default_single_thread_traits_t {
        using request_handler_t = restinio::router::express_router_t<>;
    };

    restinio::run(
            restinio::on_this_thread<my_server_traits>()
                    .address("localhost")
                    .port(8080)
                    .request_handler(std::move(router)));
}

WebService::WebService(Processor & processor) {
    handler = std::make_unique<Handler>(processor);
}

WebService::~WebService() = default;


}