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
    active_client_(!server),
    ctx_(nullptr),
    ssl_(nullptr)
{
    memset(&server_address_, 0, sizeof(server_address_));
    server_address_.sin_addr.s_addr = INADDR_ANY;
    server_address_.sin_family = AF_INET;
    server_address_.sin_port = htons(port);

    if (!server)
        client_address_ = server_address_;

    client_length_ = sizeof(client_address_);
    server_length_ = sizeof(server_address_);

    init_openssl();
    ctx_ = create_context();
    configure_context(ctx_);

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
    if (socket_type_ == SOCK_DGRAM || !server_)
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
    else
    {
        // ::connect(socket_, (sockaddr *)&server_address_, server_length_);
        std::cerr << "CONNECTING TLS\n";
        connect_tls();
        std::cerr << "TLS CONNECTED\n";
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
                connect();
                client_socket_ = socket_;
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

        // std::cerr << "BBBBB\n";
        // int code = SSL_write(ssl_, message_to_send.c_str(), message_to_send.length());
        std::cerr << "Socket: " << socket_ << ". Client: " << client_socket_ << ". Message: " << message_to_send << ".\n";
        int code = sendto(socket_, message_to_send.c_str(), message_to_send.length(), 0, NULL, NULL);
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
            accept_tls_connection();
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
        length = SSL_read(ssl_, buf, Buff_size);
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

    int length = 0;
    if (socket_type_ == SOCK_STREAM)
    {
        length = recvfrom(socket_, buf, Buff_size, MSG_PEEK, 0, 0);
        if (length <= 0)
            return -1;

        length = SSL_read(ssl_, buf, Buff_size);
    }
    else
        length = recvfrom(socket_, buf, Buff_size, 0, NULL, NULL);

    if (length <= 0)
        break_tcp_connection();\

    return length;
}

void IP_handler::init_openssl()
{
    SSL_library_init();
    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();
}

SSL_CTX* IP_handler::create_context()
{
    const SSL_METHOD *method = TLS_method();
    SSL_CTX *ctx;

    // method = server_ ? TLS_server_method() : TLS_client_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void IP_handler::configure_context(SSL_CTX *ctx, const char *path_to_cert, const char *path_to_key)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    if (server_) 
    {
        if (SSL_CTX_use_certificate_file(ctx, path_to_cert, SSL_FILETYPE_PEM) <= 0)
        {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, path_to_key, SSL_FILETYPE_PEM) <= 0)
        {
            ERR_print_errors_fp(stderr);
            exit(EXIT_FAILURE);
        }

        if (!SSL_CTX_check_private_key(ctx))
        {
            std::cerr << "Private key does not match the public certificate" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}

void IP_handler::accept_tls_connection()
{
    ssl_ = SSL_new(ctx_);
    SSL_set_fd(ssl_, client_socket_);
    if (SSL_accept(ssl_) <= 0)
    {
        ERR_print_errors_fp(stderr);
    }
}

void IP_handler::connect_tls()
{
    std::cerr << "Connecting TLS..." << std::endl;

    if (ctx_ == nullptr)
    {
        std::cerr << "SSL_CTX is null" << std::endl;
        return;
    }

    ssl_ = SSL_new(ctx_);
    if (ssl_ == nullptr)
    {
        ERR_print_errors_fp(stderr);
        std::cerr << "SSL_new failed" << std::endl;
        return;
    }

    std::cerr << "SSL_new succeeded" << std::endl;

    if (socket_ < 0)
    {
        std::cerr << "Socket is invalid" << std::endl;
        return;
    }

    int code = SSL_set_fd(ssl_, socket_);
    std::cerr << "SSL_set_fd succeeded: " << code << std::endl;
    std::cerr << "Socket is " << SSL_get_fd(ssl_) << '\n';

    SSL_connect(ssl_);
}