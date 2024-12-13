CC = g++

CFLAGS = -std=c++20 -Wno-format-security

LINKERFLAGS = -I/usr/include/openssl -L/usr/lib -lncurses -lssl -lcrypto

SRCS = ./src/messenger.cpp ./src/ip.cpp

SERVER = server.out
CLIENT = client.out

DEBUG_SERVER = server_gdb.out
DEBUG_CLIENT = client_gdb.out

all: $(SERVER) $(CLIENT)

$(SERVER): ./src/server.cpp $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LINKERFLAGS)

$(CLIENT): ./src/client.cpp $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LINKERFLAGS)

$(DEBUG_SERVER): ./src/server.cpp $(SRCS)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LINKERFLAGS)

$(DEBUG_CLIENT): ./src/client.cpp $(SRCS)
	$(CC) -g $(CFLAGS) -o $@ $^ $(LINKERFLAGS)

server_udp: $(SERVER)
	sudo ./$(SERVER) -udp

server_tcp: $(SERVER)
	sudo ./$(SERVER) -tcp

client_udp: $(CLIENT)
	sudo ./$(CLIENT) -udp

client_tcp: $(CLIENT)
	sudo ./$(CLIENT) -tcp

debug_server: $(DEBUG_SERVER)
	sudo gdb ./$(DEBUG_SERVER)

debug_client: $(DEBUG_CLIENT)
	sudo gdb ./$(DEBUG_CLIENT)
clean:
	rm ./*out