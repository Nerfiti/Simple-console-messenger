#include <cstring>
#include <iostream>
#include "messenger.hpp"

int main(int argc, const char **argv)
{
    int socket_type = SOCK_STREAM;
    if (argc > 2)
    {
        std::cerr << "Too much arguments.\n";
        return -1;
    }
    if (argc == 2)
    {
        if (!strcmp(argv[1], "-tcp"))
        {
            socket_type = SOCK_STREAM;
        }
        else if (!strcmp(argv[1], "-udp"))
        {
            socket_type = SOCK_DGRAM;
        }
        else
        {
            std::cerr << "Wrong parameter \"" << argv[1] << "\"\n";
            return -1;
        }
    }

    Messenger telegram_pro(socket_type, 8080, false);
    telegram_pro.start();

    return 0;
}

// int main(int argc, char *argv[])
// {
//     int s;
//     s = socket(AF_INET, SOCK_STREAM, 0);
//     if (s < 0) {
//         printf("Error creating socket.\n");
//         return -1;
//     }
//     struct sockaddr_in sa;
//     memset (&sa, 0, sizeof(sa));
//     sa.sin_family      = AF_INET;
//     sa.sin_addr.s_addr = inet_addr("173.194.222.139"); // address of google.ru
//     sa.sin_port        = htons (443); 
//     socklen_t socklen = sizeof(sa);
//     if (connect(s, (struct sockaddr *)&sa, socklen)) {
//         printf("Error connecting to server.\n");
//         return -1;
//     }
//     SSL_library_init();
//     SSLeay_add_ssl_algorithms();
//     SSL_load_error_strings();
//     const SSL_METHOD *meth = TLSv1_2_client_method();
//     SSL_CTX *ctx = SSL_CTX_new (meth);
//     ssl = SSL_new (ctx);
//     if (!ssl) {
//         printf("Error creating SSL.\n");
//         log_ssl();
//         return -1;
//     }
//     sock = SSL_get_fd(ssl);
//     SSL_set_fd(ssl, s);
//     int err = SSL_connect(ssl);
//     if (err <= 0) {
//         printf("Error creating SSL connection.  err=%x\n", err);
//         log_ssl();
//         fflush(stdout);
//         return -1;
//     }
//     printf ("SSL connection using %s\n", SSL_get_cipher (ssl));
    
//     char *request = "GET https://about.google/intl/en/ HTTP/1.1\r\n\r\n";       
//     SendPacket(request);
//     RecvPacket();
//     return 0;
// }