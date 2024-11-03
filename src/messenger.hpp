#include <string>
#include "ip.hpp"

class Messenger
{
    public:
        Messenger(int socket_type, int port, bool server);

        void start();

    private:
        IP_handler handler_;
        bool stop_flag = false;

        int width_ = 0;
        int height_ = 0;
        int frame_height_ = 3;

        int last_msg_y_ = -1;

        size_t send_msg_offset_ = 20;
        size_t recv_msg_offset_ = 0;

        size_t enter_field_x_ = 0;
        size_t enter_field_y_ = 0;

        bool server_;

        void clear_messages();
        void proc_message(const char *msg, size_t length, IP_handler*);
        void send(const std::string& msg, IP_handler *handler);
        void draw_frame(const std::string &enter_msg);
        void handle_input(IP_handler *handler);
        void handle_server(IP_handler *handler);
};