#include <iostream>

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

#include "device.h"

#include <thread>
#include <mutex>
#include <queue>
#include <string>

std::queue<std::string> message_queue;
std::mutex queue_mutex;

typedef websocketpp::server<websocketpp::config::asio> server;

void on_message(websocketpp::connection_hdl, server::message_ptr msg) {
        auto const & message = msg->get_payload();
        std::cout << "Websocket Message: "<< message << std::endl;
        
        std::lock_guard<std::mutex> lock(queue_mutex);
        message_queue.emplace(message);
}

void server_thread_main() {
    server websocket_server;

    websocket_server.set_message_handler(&on_message);
    websocket_server.set_access_channels(websocketpp::log::alevel::all);
    websocket_server.set_error_channels(websocketpp::log::elevel::all);

    websocket_server.init_asio();
    websocket_server.listen(9002);
    websocket_server.start_accept();

    std::cout << "starting server" << std::endl;

    websocket_server.run();
}

void device_thread_main() {
    for (;;) {
        // check incomming messages
        if (device_get_pending()) {
            uint32_t info = device_begin_read();

            uint8_t command = (info >> 24) & 0xFF;
            uint32_t size = info & 0xFFFFFF;
            
            std::vector<char> buf(size, 0);
            device_read(buf.data(), size);
            device_end_read();

            std::string message(buf.data(), size);
            std::cout << "incomming message: " << message << std::endl;
        }

        // send any outgoing messages
        if (message_queue.size() > 0){
            std::lock_guard<std::mutex> lock(queue_mutex);
            
            while (message_queue.size() > 0) {
                auto & message = message_queue.front();
                device_senddata(DATATYPE_TEXT, const_cast<char*>(message.c_str()), message.length());
                message_queue.pop();
            }
        } 
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
}

void on_device_error(const char* error) {
    std::cout << "Fatal Error: " << error << std::endl;
}

void on_device_message(const char* message) {
    std::cout << message << std::endl;
}

void on_sendrom(float percent) {
    if (percent == 1.0f) {
        std::cout << "rom uploaded" << std::endl;
    }
}

std::string rom_path = "D:/development/repos/framework64/build_n64/bin/data_link.z64";

int main() {
    device_sendrom_params_t sendrom_params;
    device_sendrom_params_init(&sendrom_params);
    device_set_fatal_error_callback(on_device_error);
    device_set_message_callback(on_device_message);
    device_set_sendrom_progress_callback(on_sendrom);
    device_find(CART_ANY);
    device_open();
    device_sendrom(rom_path.c_str(), &sendrom_params);

    std::thread server_thread(server_thread_main);
    std::thread device_thread(device_thread_main);

    server_thread.join();
    device_thread.join();

    return 0;
}