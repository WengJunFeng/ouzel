// Copyright 2015-2019 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_NETWORK_SERVER_HPP
#define OUZEL_NETWORK_SERVER_HPP

#include "network/Socket.hpp"

namespace ouzel
{
    namespace network
    {
        class Network;

        class Server final
        {
        public:
            Server(Network& initNetwork);
            ~Server();

            Server(const Server&) = delete;
            Server& operator=(const Server&) = delete;

            Server(Server&& other);
            Server& operator=(Server&& other);

            void listen(const std::string& address, uint16_t port);
            void disconnect();

        private:
            Network* network = nullptr;
            Socket sock;
        };
    } // namespace network
} // namespace ouzel

#endif // OUZEL_NETWORK_SERVER_HPP
