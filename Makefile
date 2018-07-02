test:
	gcc echoServer.c -pthread -o test
client:
	g++ main.cpp -o client
ctest:
	./client 127.0.0.1 33455 123 server.c 1000
