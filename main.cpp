#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

#include <arpa/inet.h> //inet_addr
#include <string.h>

#include <sys/types.h>
#include <stdio.h>
#include <netdb.h>


#include <stdlib.h>
#include <netinet/in.h>

using namespace std;

int main(int argc, const char * argv[]) {
    //server IP address and port number, the client IP address, the filename of the requested file and the size of the send/receive


     struct sockaddr_in serverConnect;
	   struct sockaddr_in6 serverConnect6;

    if(argc==6 || argc==1){
        if(argc == 1){
          printf("Please enter options in the following order: \n");
          printf("The IP address of the server you wish to reach (default : localhost)\n");
          printf("The port you wish to send on. (default : 33455)\n");
          printf("Your IP  (default : 127.0.0.1)\n");
          printf("The name of the file you wish to have. (default: fileToTransfer)\n");
          printf("The size of the packets you wish to have (default: 1440)\n");
          printf("To use a default value put a . (dot) for that argument\n");
          printf("\n");
          return 1;
        }
        //todo create checks here
        const char* serverIPAddress = strcmp(argv[1],".") == 0? "127.0.0.1" : argv[1];
        int serverPort = strcmp(argv[2],".") == 0? atoi("33455") : atoi(argv[2]);
        const char * fileName = argv[4] == "." ? "fileToTransfer" : argv[4];
        int receiveSize = argv[5] == "." ? atoi("1440"): atoi(argv[5]);
        char * receivedBuffer[receiveSize];
        //logic to know if its ipv4 or ipv6
        //int destinationServerSocketNum = socket(AF_INET , SOCK_STREAM , 0);
        //if server ip is ipv4 create ipv4 socket->AF_INET
        //if server ip is ipv6  if ipv6 ipv6 socket-> AF_INET6
        struct addrinfo hint, *IPv6Or4 = NULL;
        int returnvalue;

        memset(&hint, '\0', sizeof hint);

        hint.ai_family = PF_UNSPEC;
        hint.ai_flags = AI_NUMERICHOST;
        int destinationServerSocketNum = -222;
        returnvalue = getaddrinfo(serverIPAddress, NULL, &hint, &IPv6Or4);


        if (returnvalue) {
            puts("Invalid address");
            puts(gai_strerror(returnvalue));
            return 1;
        }
        if(IPv6Or4->ai_family == AF_INET) {
            destinationServerSocketNum = socket(AF_INET , SOCK_STREAM , 0);

            if (destinationServerSocketNum == -1)
            {
                cout<<"Could not create socket";
            }
            //serverConnect.sin_addr.s_addr = inet_addr(serverIPAddress);
            inet_pton(AF_INET, serverIPAddress, &(serverConnect.sin_addr.s_addr));
            serverConnect.sin_family = AF_INET;
            serverConnect.sin_port = htons( serverPort );
            //ipv4 address
            printf("%s is an ipv4 address\n",argv[0]);
			    if(connect(destinationServerSocketNum, (struct sockaddr *)&serverConnect , sizeof(serverConnect))){
            	puts("connect error");
            	return 1;
        	}

        } else if(IPv6Or4->ai_family == AF_INET6) {
            destinationServerSocketNum = socket(AF_INET6 , SOCK_STREAM , 0);
            if (destinationServerSocketNum == -1)
            {
                cout<<"Could not create socket";
            }
            //struct sockaddr_in6 serverConnect;
            int convertToV6 = inet_pton(AF_INET6, serverIPAddress, &(serverConnect6.sin6_addr.s6_addr));
            if(convertToV6 != 1){
              printf("%s is an is unknown address format %d\n",argv[1],IPv6Or4->ai_family);
              return 1;
            }
            serverConnect6.sin6_family = AF_INET6;
            serverConnect6.sin6_port = htons( serverPort );
            printf("%s is an ipv6 address\n",argv[0]);

	        if(connect(destinationServerSocketNum, (struct sockaddr *)&serverConnect6 , sizeof(serverConnect6))){
	            puts("connect error");
	            return 1;
	        }
		}

		else{
            printf("%s is an is unknown address format %d\n",argv[1],IPv6Or4->ai_family);
        }
        const char *message=  fileName;

        if( send(destinationServerSocketNum , message , strlen(message) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        //

        // from the server side sent 2 packets,
        // the first one will aways be that.
        char * serverReply = 0;
        serverReply = new char[2000];
        int firstLineRead;
        printf("Waiting for server to send file size...\n");
        do{
          firstLineRead = read(destinationServerSocketNum, serverReply,100);
        } while(firstLineRead <= 1);


        printf("server sends : %s \n", serverReply );

        bool hasNum = false;
        char *NumOfBytes;
        long IntNumOfBytes = atoi(serverReply + 13);
        if(IntNumOfBytes == 0){
          printf("File doesn't exist. Cleaning and terminating.\n");
          exit(1);
        }
        printf("FILE IS OF SIZE : %ld  [ recv ]\n", IntNumOfBytes);
        //TODO : FILE DOENS'T EXIST

        /*if(hasNum ==true){
            if(atoi(NumOfBytes)!=0){
                IntNumOfBytes = atoi(NumOfBytes);
            }
          }*/
          printf("About to create file [ ok ] \n");
          FILE* file = fopen(fileName, "a+" );


          if(file ==NULL){
              cout<<"COULD NOT OPEN OUTPUT FILE";
              return 0;
          }
          // int totalBytesRead = 0;
          //int bytesRead =0;
          int numPackets = (IntNumOfBytes / receiveSize) + 1;
          for(int  i = 0; i < numPackets; i++){
                if(receiveSize > IntNumOfBytes){
                  receiveSize = IntNumOfBytes;
                }
                read(destinationServerSocketNum, receivedBuffer,receiveSize);//change to total byte size
                fwrite(receivedBuffer, sizeof(char),receiveSize, file);
                IntNumOfBytes = IntNumOfBytes - receiveSize;
            }
            printf("File copied in full [ ok ] \n");


        puts("Finished transmission. \n");

    }else{
        std::cout<<"incorrect number of arguments";
        return 0;
    }

    return 0;
}
