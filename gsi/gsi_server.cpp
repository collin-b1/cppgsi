#include "gsi_server.h"
#include <iostream>
#include <stdexcept>

namespace cs2gsi {
    GSIServer::GSIServer(GSIDispatcher &dispatcher, std::string host, int port) :
        dispatcher_(dispatcher), host_(std::move(host)), port_(port) {
        server_.Post("/", [this](const httplib::Request &req, httplib::Response &res) {
            if (capturing_) {
                if (std::chrono::steady_clock::now() < capture_until_) {
                    capture_file_ << req.body << "\n";
                    capture_file_.flush();
                } else {
                    capturing_ = false;
                    capture_file_.close();
                    std::cout << "[GSIServer] capture complete\n";
                }
            }

            try {
                dispatcher_.dispatch(nlohmann::json::parse(req.body));
                res.status = 200;
            } catch (const std::exception &e) {
                std::cerr << "[GSIServer] parse error: " << e.what() << "\n";
                res.status = 400;
            }
        });
    }

    void GSIServer::capture_to_file(const std::string &path, std::chrono::seconds duration) {
        capture_file_.open(path, std::ios::trunc);
        capture_until_ = std::chrono::steady_clock::now() + duration;
        capturing_ = true;
        std::cout << "[GSIServer] capturing payloads to " << path << " for " << duration.count() << "s\n";
    }

    void GSIServer::start() {
        std::cout << "[GSIServer] listening on http://" << host_ << ":" << port_ << "\n";
        if (!server_.listen(host_, port_))
            throw std::runtime_error("GSIServer failed to bind to " + host_ + ":" + std::to_string(port_));
    }

    void GSIServer::stop() { server_.stop(); }
} // namespace cs2gsi
