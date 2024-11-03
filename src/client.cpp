#include <cstring>
#include <iostream>
#include "messenger.hpp"

int main(int argc, const char **argv)
{
    int socket_type = SOCK_DGRAM;
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