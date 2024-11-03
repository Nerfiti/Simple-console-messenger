#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include "ip.hpp"
IP_handler::IP_handler(int socket_type, int port, processor proc, bool server):
    socket_type_(socket_type),
    server_address_(),
    server_length_(0),
    client_address_(),
    client_length_(sizeof(client_address_)),
    client_socket_(-1),
    socket_(-1),
    server_(server),
    process_msg_(proc),
    active_client_(!server)
{
    memset(&server_address_, 0, sizeof(server_address_));
    server_address_.sin_addr.s_addr = INADDR_ANY;
    server_address_.sin_family = AF_INET;
    server_address_.sin_port = htons(port);

    if (!server)
        client_address_ = server_address_;

    client_length_ = sizeof(client_address_);
    server_length_ = sizeof(server_address_);
    connect();
}

IP_handler::~IP_handler() 
{
    close();
}

int IP_handler::connect()
{
    if (socket_ >= 0)
    {
        std::cout << "Error while connecting socket. Close previous one.\n";
        return -1;
    }

    socket_ = socket(AF_INET, socket_type_, 0);
    if (socket_type_ == SOCK_DGRAM)
        client_socket_ = socket_;
    if (socket_ < 0)
    {
        std::cout << "Error while creating new socket.\n" << strerror(errno) << '\n';
        std::cout << errno << '\n';
        return -1;
    }

    if (server_)
    {
        if (bind(socket_, (sockaddr *)&server_address_, sizeof(server_address_)) < 0)
        {
            std::cerr << "Binding error: " << strerror(errno) << '\n';
            return -1;
        }

        if (socket_type_ == SOCK_STREAM && ::listen(socket_, 0) < 0)
        {
            std::cerr << "Waiting tcp connections failed.\n";
            return -1;
        }
    }
    return 0;
}

int IP_handler::close()
{
    if (client_socket_ >= 0)
        ::close(socket_);
    if (socket_ >= 0)
        return ::close(client_socket_);

    return -1;
}

void IP_handler::listen()
{
    sleep(0.5);
    if (socket_ == -1)
        return;

    memset(buf, 0, Buff_size);
    int length = -1;
    if (server_)
        length = listen_as_server();
    else
        length = listen_as_client();

    if (active_client_ && length > 0)
        process_msg_(buf, length, this);
}

const char *IP_handler::send(const char *message, size_t length)
{
    std::string message_to_send = message;
    if (!strcmp(message, "attack"))
    {
        message_to_send = "";
        for (int i = 0; i < 666; ++i)
            message_to_send += "Now you give me your computer. You can just see what I will do with it.";
    }

    if (!active_client_)
    {
        if (server_)
            return "Sending error. No active clients.";
        else
            return "Unable to connect to the server.";
    }

    if (socket_type_ == SOCK_STREAM)
    {
        if (!server_)
        {
            if (client_socket_ == -1)
            {
                if (socket_ == -1)
                    socket_ = socket(AF_INET, socket_type_, 0);
                if (::connect(socket_, (sockaddr *)&server_address_, server_length_) == 0)
                    client_socket_ = socket_;
                else
                    return "No connected server.";
            }
            if (!strcmp(message, "exit()") || !strcasecmp(message, "bye"))
            {
                break_tcp_connection();
                ::close(socket_);
                socket_ = -1;
                active_client_ = true;
                return "Disconnected.";
            }
        }

        int code = ::send(client_socket_, message_to_send.c_str(), message_to_send.length(), 0);
        if (code < 0)
            return "Error sending packets.";

        return message;
    }

    int code = sendto(socket_, message_to_send.c_str(), message_to_send.length(), 0, (sockaddr *)&client_address_, client_length_);
    if (code < 0)
        return "Error sending packets.";

    return message;
}

void IP_handler::accept_tcp_connection()
{
    client_socket_ = accept(socket_, (sockaddr *)&client_address_, &client_length_);
    active_client_ = client_socket_ >= 0;
}

void IP_handler::break_tcp_connection()
{
    if (shutdown(client_socket_, SHUT_RDWR) == 0)
    {
        if (socket_ != client_socket_)
            ::close(client_socket_);
        client_socket_ = -1;
        active_client_ = false;
    }
}

int IP_handler::listen_as_server()
{
    if (client_socket_ == -1)
    {
        if (socket_type_ == SOCK_STREAM)
        {
            accept_tcp_connection();
            return 0;
        }
        else
        {
            client_socket_ = socket_;
        }
    }

    int length = -1;
    if (socket_type_ == SOCK_STREAM)
    {
        length = recv(client_socket_, buf, Buff_size, 0);
        if (length <= 0)
            break_tcp_connection();
    }
    else
    {
        sockaddr_in tmp_client;
        socklen_t tmp_client_length = sizeof(tmp_client);
        length = recvfrom(socket_, buf, Buff_size, 0, (sockaddr *)&tmp_client, &tmp_client_length);
        if (!active_client_)
        {
            client_address_ = tmp_client;
            client_length_ = tmp_client_length;
            active_client_ = true;
        }
        if (tmp_client.sin_addr.s_addr != client_address_.sin_addr.s_addr || tmp_client.sin_port != client_address_.sin_port)
            return 0;
    }

    if (!strcmp(buf, "exit()") || !strcasecmp(buf, "bye"))
    {
        if (socket_type_ == SOCK_STREAM)
            break_tcp_connection();

        active_client_ = false;
        client_address_ = {};
        client_length_ = 0;
    }

    return length;
}

int IP_handler::listen_as_client()
{
    if (socket_ == -1)
        return -1;

    int length = recvfrom(socket_, buf, Buff_size, 0, NULL, NULL);
    if (length <= 0)
        break_tcp_connection();

    return length;

}
