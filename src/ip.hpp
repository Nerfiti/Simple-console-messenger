#pragma once

#include <netinet/in.h>
#include <functional>
#include <net/ethernet.h>

class IP_handler
{
    public:
        static constexpr size_t Buff_size = 50000;
        using processor = std::function<void(const char *, size_t, IP_handler *)>;
        IP_handler(int socket_type, int port, processor proc, bool server = false);
        virtual ~IP_handler();

        int connect();
        int close();

        void listen();

        const char *send(const char *message, size_t length);

    private:
        int socket_type_;
        sockaddr_in server_address_;
        socklen_t server_length_;
        sockaddr_in client_address_; //for server only
        socklen_t client_length_;    //for server only

        int client_socket_;          //for tcp server ony
        int socket_;

        bool server_;
        processor process_msg_;

        char buf[Buff_size];

        bool active_client_ = false;

        void accept_tcp_connection();
        void break_tcp_connection();
        int listen_as_server();
        int listen_as_client();
};