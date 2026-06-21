#pragma once
#include <string>
#include "httplib/httplib.h"
#include "gsi_dispatcher.h"

namespace cs2gsi
{
    class GSIServer
    {
    public:
        GSIServer(GSIDispatcher& dispatcher,
                  std::string host = "127.0.0.1",
                  int port = 3000);

        // Blocks until stop() is called.
        void start();

        void stop();

        GSIDispatcher& dispatcher() { return dispatcher_; }

    private:
        GSIDispatcher& dispatcher_;
        httplib::Server server_;
        std::string host_;
        int port_;
    };
} // namespace cs2gsi
