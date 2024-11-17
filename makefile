all: client server

CLIENT_OBJECTS = $(shell cat client.flist)

client: $(CLIENT_OBJECTS)
	g++ $^ -o build/client

SERVER_OBJECTS = $(shell cat server.flist)

server: $(SERVER_OBJECTS)
	g++ $^ -o build/server

run_server: server
	cd build; ./server $(ARGS)

run_client: client
	cd build; ./client $(ARGS)

rm:
	find . -name "*.o" -type f -delete
	find build ! -name '.gitkeep' -type f -delete
