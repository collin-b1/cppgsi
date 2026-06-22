#pragma once
#include <chrono>
#include <fstream>
#include <string>
#include "gsi_dispatcher.h"
#include "httplib/httplib.h"

namespace cs2gsi {
    class GSIServer {
    public:
        GSIServer(GSIDispatcher &dispatcher, std::string host = "127.0.0.1", int port = 3000);

        // Blocks until stop() is called.
        void start();

        void stop();

        // Capture raw GSI payloads to a file for the given duration, then stop.
        // Each payload is written as a JSON line. Call before start().
        void capture_to_file(const std::string &path, std::chrono::seconds duration);

        GSIDispatcher &dispatcher() { return dispatcher_; }

    private:
        GSIDispatcher &dispatcher_;
        httplib::Server server_;
        std::string host_;
        int port_;

        std::ofstream capture_file_;
        std::chrono::steady_clock::time_point capture_until_;
        bool capturing_{false};
    };
} // namespace cs2gsi
