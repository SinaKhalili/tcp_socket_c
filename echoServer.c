/* #include <stdlib.h> */
#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <pthread.h>

#define   PROTOPORT   33455    /* default protocol port number           */
#define   PROTOPORT6   33446    /* default protocol port number           */

#define   QLEN        24        /* size of request queue                  */

int   visits       =   0;      /* counts client connections              */

struct arg_struct {
    int mtu;
    int socket_n;
    int *socket;
};


 void *file_sender(void *arguments) {
   printf("Stated thread to hand connection [ ok ]\n" );
   //Get the socket descriptor
   struct arg_struct *args = arguments;
   int ClientSockNum = *(int*) args->socket;
   int read_size;
   int mtu = args->mtu;
   char message[500];
   char client_message[2000];
   printf("MTU is : %d \n", mtu );

   //Send some messages to the client%




   //Receive a message from client
   read_size = recv(ClientSockNum , client_message , 2000 , 0);

   printf("Got : %s \n", client_message);

   if(client_message[0] == '\0'){
     printf("Got no file descriptor. Exiting. \n" );
     pthread_exit(NULL);
   }




   if(read_size == 0)
   {
       puts("Client disconnected");
       fflush(stdout);
   }
   else if(read_size == -1)
   {
       perror("recv failed");
   }

   FILE* fp = fopen( client_message, "r");
   printf("Opening requested file \n");
   if (fp == NULL) {
      printf("File pointer is null [ dead ]\n" );
      sprintf(message, "COULD NOT OPEN REQUESTED FILE\n");
      write(ClientSockNum , message , strlen(message));
      pthread_exit(NULL);
   }

   printf("File opened [ ok ]\n" );
   fseek(fp, 0L, SEEK_END);
   long sz = ftell(fp);
   rewind(fp);
   printf("Size of your file is %ld \n", sz);
   sprintf(message, "FILE SIZE IS %ld bytes", sz);
   write(ClientSockNum , message , strlen(message));
   printf("Sent file size [ ok ]\n" );
   long numPackets = (sz / mtu) + 1;
   printf("Packets needed to send file : %ld \n",numPackets );
   char * fileReturn = malloc(sizeof(char)*(mtu+1));
   printf("Made buffer to send file in segments [ ok ]\n" );
   for(int i = 0; i < numPackets; i++){
     sleep(1);
     printf("Begin sending packet %d \n", i);
     size_t newLen = fread(fileReturn, sizeof(char), mtu, fp);
     printf("Put in %ld bytes into packet \n", newLen);
     write(ClientSockNum , fileReturn , strlen(fileReturn));
   }
   fclose(fp);
   close(ClientSockNum);
   pthread_exit(NULL);

   return 0;
 }

 void * acceptThread(void* arguments){
   struct arg_struct *args = arguments;
   int tcpsd = args->socket_n;
   int mtu = args->mtu;
   int * new_sock = malloc(1);
   int connfd;

   while( connfd = accept(tcpsd, NULL, NULL)){

     *new_sock = connfd;
     pthread_t sniffer_thread;
     struct arg_struct argsT;
     argsT.mtu = mtu;
     argsT.socket = new_sock;
     if( pthread_create( &sniffer_thread , NULL ,  file_sender , (void*) &argsT ) < 0)
       {
           perror("could not create thread");
            pthread_exit(NULL);
       }

     pthread_join( sniffer_thread, NULL);

   }

 }

int main(int argc, char const *argv[]) {

   struct   protoent    *tcpptrp; /*pointer to udp protocol table entry  */
   struct   sockaddr_in sad;      /* structure to hold server's address  */
   struct   sockaddr_in6 sad6;      /* structure to hold server's address  */
   struct   sockaddr_in cad;      /* structure to hold client's address  */
   struct   sockaddr_in6 cad6;      /* structure to hold client's address  */
   int      tcpsd;         /* server socket descriptors           */
   int      tcpsd6;         /* server socket descriptors           */
   int      connfd;               /* client socket descriptor            */
   int      connfd6;               /* client socket descriptor            */
   int      maxfdp1;              /* maximum descriptor plus 1           */
   int      port;                 /* protocol port number                */
   int      port6;                 /* protocol port number                */
   size_t   lenbuf;               /* length of buffer                    */
   int *    new_sock;
   int      segmentcnt;           /* cumulative # of segments received   */
   int      packetcnt;            /* cumulative # of packets received    */
   int      tcpcharcntin;         /* cumulative # of octets received     */
   int      tcpcharcntout;        /* cumulative # of octets sent         */
   int      nread;                /* # of octets received in one read    */
   int      nwrite;               /* # of octets sent in one write       */
   int      retval;               /* function return flag for testing    */
   int      mtu;
   char * message;
   struct timeval tval;           /* max time to wait before next select */
   socklen_t      len;            /* length of the socket address struct */
   pid_t    pid;
   fd_set   descset;
   int      val;

   /* Initialize variables                                               */
   packetcnt = 0;
   segmentcnt = 0;
   tcpcharcntin = 0;
   tcpcharcntout = 0;
   memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure      */
   memset((char *)&sad6,0,sizeof(sad6)); /* clear sockaddr structure      */
   memset((char *)&cad,0,sizeof(cad)); /* clear sockaddr structure      */
   sad.sin_family = AF_INET;           /* set family to Internet        */
   sad.sin_addr.s_addr = INADDR_ANY;   /* set the local IP address      */
   sad6.sin6_addr = in6addr_any;   /* set the local IP address      */
   sad6.sin6_family = AF_INET6;
   cad.sin_family = AF_INET;          /* set family to Internet         */
   cad6.sin6_family = AF_INET6;          /* set family to Internet         */

   if(argc != 4){
     printf("Please enter options in the following order: \n");
     printf("The ipv4 port for this server (default : 33455)\n");
     printf("The ipv6 port for this server (default : 33446)\n");
     printf("The packet size (mtu) for this server (default : 1440)\n");
     printf("To use a default value put a . (dot) for that argument\n");
     return 1;
   }
   else {
       port = strcmp(argv[1], ".") == 0? 33455 : atoi(argv[1]);
       port6 = strcmp(argv[2], ".") == 0? 33446 : atoi(argv[2]);
       mtu = strcmp(argv[3], ".") == 0? 1440 : atoi(argv[3]);
   }



   /* Check command-line argument for the protocol port and extract      */
   /* port number if one is specified.  Otherwise, use the   default     */
   /* port value given by constant PROTOPORT                             */
   /* check the resulting port number to assure it is valid (>0)         */
   /* convert the valid port number to network byte order and insert it  */
   /* ---  into the socket address structure.                            */
   /* OR print an error message and exit if the port is invalid          */


   //TODO : read argv for port numbers here
   port = PROTOPORT;
   port6 = PROTOPORT6;

   if (port > 0) {
      sad.sin_port = htons((u_short)port);
   }
   if (port6 > 0) {
     sad6.sin6_port = htons((u_short)port6);
     sad6.sin6_addr = in6addr_any;
   }
   else {
      fprintf(stderr,"bad port number/s %s\n",argv[1]);
      exit(1);
   }

   /* Map TCP transport protocol name to protocol number                 */
   /* Create a tcp socket with a socket descriptor tcpsd                 */
   /* Bind a local address to the tcp socket                             */
   /* Put the TCP socket in passive listen state and specify the lengthi */
   /* --- of request queue                                               */
   /* If any of these four processes fail an explanatory error message   */
   /* --- will be printed to stderr and the server will terminate        */

   tcpsd = socket(AF_INET, SOCK_STREAM, 0);
   tcpsd6 = socket(AF_INET6, SOCK_STREAM, 0);
   int flag = 1;
   setsockopt(tcpsd6, IPPROTO_IPV6, IPV6_V6ONLY, &flag, sizeof(int));

   if (tcpsd < 0) {
      fprintf(stderr, "tcp socket creation failed\n");
      exit(1);
   }
   printf("Created ipv4 socket [ ok ]\n");
   if (tcpsd6 < 0) {
      fprintf(stderr, "tcp6 socket creation failed\n");
      exit(1);
   }
   printf("Created ipv6 socket [ ok ]\n");

   if (bind(tcpsd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
      fprintf(stderr,"tcp bind failed\n");
      close(tcpsd);
      exit(1);
   }
   if (listen(tcpsd, QLEN) < 0) {
      fprintf(stderr,"listen failed\n");
      close(tcpsd);
      exit(1);
   }
   printf("Bind And listen ipv4 socket [ ok ]\n");

   if (bind(tcpsd6, (struct sockaddr *)&sad6, sizeof(sad6)) < 0) {
      fprintf(stderr,"tcp6 bind failed\n");
      close(tcpsd6);
      exit(1);
   }
   if (listen(tcpsd6, QLEN) < 0) {
      fprintf(stderr,"listen6 failed\n");
      close(tcpsd6);
      exit(1);
   }
   printf("Bind And listen ipv6 socket [ ok ]\n");

    pthread_t ipv4_blocker;
    pthread_t ipv6_blocker;
    struct arg_struct args4;
    args4.mtu = argc > 1 ? atoi(argv[3]) : 1440;
    args4.socket_n = tcpsd;
    if( pthread_create( &ipv4_blocker , NULL ,  acceptThread , (void*) &args4  ) < 0)
      {
          perror("could not create thread");
          return 1;
      }

    struct arg_struct args6;
    args6.mtu = argc > 1 ? atoi(argv[3]) : 1280;
    args6.socket_n = tcpsd6;
    if( pthread_create( &ipv6_blocker , NULL ,  acceptThread , (void*) &args6)  < 0)
      {
          perror("could not create thread");
          return 1;
      }

    pthread_join( ipv4_blocker, NULL);
    pthread_join( ipv6_blocker, NULL);

}
