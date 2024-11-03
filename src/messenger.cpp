#include <cstring>
#include <iostream>
#include <ncurses.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "messenger.hpp"

#define PORT 8080

#define proc_lambda [this](const char *msg, size_t length, IP_handler *handler){ proc_message(msg, length, handler); }

Messenger::Messenger(int socket_type, int port, bool server):
    handler_(socket_type, port, proc_lambda, server),
    stop_flag(false),
    width_(0),
    height_(0),
    frame_height_(3),
    last_msg_y_(-1),
    send_msg_offset_(20),
    recv_msg_offset_(0),
    enter_field_x_(0),
    enter_field_y_(0),
    server_(server)
    {}

void Messenger::proc_message(const char *msg, size_t length, IP_handler*) 
{
    int y, x;
    getyx(stdscr, y, x);

    std::string TLDR_string = std::string("TLDR: [") + std::to_string(length) + std::string(" bytes].");
    if (length > 100)
        msg = TLDR_string.c_str();

    ++last_msg_y_;
    int max_msg_y = height_ - frame_height_ - 1;
    if (last_msg_y_ > max_msg_y)
        clear_messages();
    int msg_y = ++last_msg_y_;
    int msg_x = recv_msg_offset_;
    std::string prefix = std::string(server_ ? "Client: " : "Server: ");
    mvprintw(msg_y, msg_x, (prefix + msg).c_str());

    move(y, x);
    refresh();
}

void Messenger::send(const std::string& msg, IP_handler *handler)
{
    const char *sended = handler->send(msg.c_str(), msg.length());

    int y, x;
    getyx(stdscr, y, x);

    ++last_msg_y_;
    int max_msg_y = height_ - frame_height_ - 1;
    if (last_msg_y_ > max_msg_y)
        clear_messages();
    int msg_y = ++last_msg_y_;
    int msg_x = send_msg_offset_;
    mvprintw(msg_y, msg_x, (std::string("You: ") + sended).c_str());

    move(y, x);
    refresh();
}

void Messenger::draw_frame(const std::string &enter_msg)
{
    int x = enter_field_x_ + 1;
    int y = enter_field_y_;
    int terminal_width = getmaxx(stdscr);

    mvprintw(y - 1, 0, std::string(terminal_width, '-').c_str());
    mvprintw(y + 0, 0, std::string(terminal_width, ' ').c_str());
    mvprintw(y + 1, 0, std::string(terminal_width, '-').c_str());
    mvprintw(y, x, (std::string("Enter your message: ") + enter_msg).c_str());
    refresh();
}

void Messenger::clear_messages()
{
    wclear(stdscr);
    refresh();
    draw_frame("");
    last_msg_y_ = -1;
}

void Messenger::handle_input(IP_handler *handler)
{
    std::string input = "";

    draw_frame(input);
    while (!stop_flag)
    {
        wchar_t ch = getch();
        switch (ch)
        {
            case '\n':
            case KEY_ENTER:
            {
                if (input == "exit()")
                    stop_flag = true;

                send(input, handler);
                input.clear();
                break;
            }
            case KEY_BACKSPACE:
            case 127:
            case '\b':
            {
                if (!input.empty())
                    input.pop_back();
                break;
            }
            default:
            {
                input.push_back(ch);
                break;
            }
        }
        draw_frame(input);
    }
}

void Messenger::handle_server(IP_handler *handler)
{
    while (!stop_flag)
    {
        handler->listen();
    }
}

void Messenger::start()
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    enter_field_y_ = getmaxy(stdscr) - 2;
    width_ = getmaxx(stdscr);
    height_ = getmaxy(stdscr);

    auto send_thread = std::thread(&Messenger::handle_input, this, &handler_);
    auto recv_thread = std::thread(&Messenger::handle_server, this, &handler_);

    send_thread.join();

    endwin();
    recv_thread.detach();
}