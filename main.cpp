//
//  main.cpp
//  dualStackClient
//
//  Created by jimmy zhong on 2018-06-28.
//  Copyright © 2018 jimmy zhong. All rights reserved.
//

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

    if(argc==6){
        //todo create checks here
        const char* serverIPAddress = argv[1];
        int serverPort = atoi(argv[2]);
        const char * clientIP =argv[3];
        const char * fileName = argv[4];
        int receiveSize = atoi(argv[5]);
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
        int destinationServerSocketNum =NULL;
        returnvalue = getaddrinfo(argv[1], NULL, &hint, &IPv6Or4);


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
            serverConnect.sin_addr.s_addr = inet_addr(serverIPAddress);
            serverConnect.sin_family = AF_INET;
            serverConnect.sin_port = htons( serverPort );
            //ipv4 address
            printf("%s is an ipv4 address\n",argv[0]);

        } else if(IPv6Or4->ai_family == AF_INET6) {
            destinationServerSocketNum = socket(AF_INET6 , SOCK_STREAM , 0);
            if (destinationServerSocketNum == -1)
            {
                cout<<"Could not create socket";
            }
            struct sockaddr_in serverConnect;
            serverConnect.sin_addr.s_addr = inet_addr(serverIPAddress);
            serverConnect.sin_family = AF_INET6;
            serverConnect.sin_port = htons( serverPort );
            printf("%s is an ipv6 address\n",argv[0]);
        } else {
            printf("%s is an is unknown address format %d\n",argv[1],IPv6Or4->ai_family);
        }


        if(connect(destinationServerSocketNum, (struct sockaddr *)&serverConnect , sizeof(serverConnect))){
            puts("connect error");
            return 1;
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
        do{
          firstLineRead = read(destinationServerSocketNum, serverReply,100);
        } while(firstLineRead <= 1);


        printf("server sends : %s \n", serverReply );

        printf("Trying to make string \n" );
        printf("server's fourth letter is  : %c \n", serverReply[3] );

        bool hasNum = false;
        char *NumOfBytes;
        long IntNumOfBytes = atoi(serverReply);
        printf("FILE IS OF SIZE : %ld  [ recv ]\n", IntNumOfBytes);
        //TODO : FILE DOENS'T EXIST
        /*if(hasNum ==true){
            if(atoi(NumOfBytes)!=0){
                IntNumOfBytes = atoi(NumOfBytes);
            }
          }*/
          printf("About to create file : \n");
          FILE* file = fopen("outputfile.txt", "a+" );


          if(file ==NULL){
              cout<<"could not open file";
              return 0;
          }
          // int totalBytesRead = 0;
          int bytesRead =0;
            while(bytesRead<=IntNumOfBytes){
                bytesRead += read(destinationServerSocketNum, receivedBuffer,receiveSize);//change to total byte size
                fwrite(receivedBuffer, sizeof(char),receiveSize, file);
                // bytesRead += bytesRead;
            }


            //read read from socket another byte stream
            //read that many bytes from the socket.
            //into a buffer,
            //or into the file directly.


        // else{
        //     cout<< "The server could not Open file";
        //     return 0;
        // }

        //for lenghth of server reply get the number inside. if serverReply
        //is digit->
        //flag,
        //loop
        //append
        //string cat append that digit.
        //
        //at atoi it.





        //read that total size into a buffer of that size.
        //write that into a file.



        //
        //receive buffer of max size to be read into.


        //fwrite( array, 1, 100, file );

        // receive extract into buffer of max size from command line.
        // if its ok

        // create file with name name
        // if cannot open send message cannot open.

        //if ok write it all to a file.


        //packet will contain a message. extract that number of bytes

        //read
        //write the received data->> all of it into a file
        //
                 //create and open a file for this
        puts("Reply received\n");



        ///send some data
        //
        //to write to the file need that.

        // possibly needs to convert to bytes..



    }else{
        std::cout<<"incorrect number of arguments";
        return 0;
    }

    //create the sockets and bind them
    // send and parse the filename into a get request

    // listen for a response.


    return 0;
}
