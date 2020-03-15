CC = g++
OPENCV =  `pkg-config --cflags --libs opencv`
PTHREAD = -pthread

CLIENT = client.cpp
SERVER = server.cpp
CLI = client
SER = server

all: server client
  
server: $(SERVER)
	$(CC) $(SERVER) -o $(SER)  $(OPENCV)
client: $(CLIENT)
	$(CC) $(CLIENT) -o $(CLI)  $(OPENCV)

.PHONY: clean

clean:
	rm $(CLI) $(SER)