#include "gsi_server.h"
#include <iostream>

namespace cs2gsi
{
    GSIServer::GSIServer(GSIDispatcher& dispatcher, std::string host, int port)
        : dispatcher_(dispatcher), host_(std::move(host)), port_(port)
    {
        server_.Post("/", [this](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                dispatcher_.dispatch(nlohmann::json::parse(req.body));
                res.status = 200;
            }
            catch (const std::exception& e)
            {
                std::cerr << "[GSIServer] parse error: " << e.what() << "\n";
                res.status = 400;
            }
        });
    }

    void GSIServer::start()
    {
        std::cout << "[GSIServer] listening on http://" << host_ << ":" << port_ << "\n";
        server_.listen(host_, port_);
    }

    void GSIServer::stop()
    {
        server_.stop();
    }
} // namespace cs2gsi
