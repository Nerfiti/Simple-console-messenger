CC = g++

CFLAGS = -std=c++20 -Wno-format-security

LINKERFLAGS = -lncurses

SRCS = ./src/messenger.cpp ./src/ip.cpp

SERVER = server.out
CLIENT = client.out

all: $(SERVER) $(CLIENT)

$(SERVER): ./src/server.cpp $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LINKERFLAGS)

$(CLIENT): ./src/client.cpp $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LINKERFLAGS)

server_udp: $(SERVER)
	sudo ./$(SERVER) -udp

server_tcp: $(SERVER)
	sudo ./$(SERVER) -tcp

client_udp: $(CLIENT)
	sudo ./$(CLIENT) -udp

client_tcp: $(CLIENT)
	sudo ./$(CLIENT) -tcp

clean:
	rm ./*out