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
    int* socket;
};
/*------------------------------------------------------------------------
 * Program:   echoserver
 *
 * Purpose:   allocate a TCP and UDP socket and then repeatedly
 *            executes the following:
 *      (1) wait for the next TCP connection or TCP packet or
 *          UDP packet from a client
 *      (2) when accepting a TCP connection a child process is spawned
 *          to deal with the TCP data transfers on that new connection
 *      (2) when a TCP segment arrives it is served by the child process.
 *          The arriving segment is echoed back to the client
 *      (2) when a UDP packet arrives it is echoed back to the client.
 *      (3) when the TCP connection is terminated the child then terminates
 *      (4) go back to step (1)
 *
 * Syntax:    server [[ port ] [buffer size]]
 *
 *       port  - protocol port number to use
 *       buffer size  - MSS of each packet sent
 *
 * Note:      The port argument is optional.  If no port is specified,
 *         the server uses the default given by PROTOPORT.
 *
 *------------------------------------------------------------------------


 */
 void *file_sender(void *arguments) {
   //Get the socket descriptor
   struct arg_struct *args = arguments;
   int ClientSockNum = *(int*) args->socket;
   int read_size;
   int mtu = args->mtu;
   char *message;
   char client_message[2000];

   //Send some messages to the client
   message = "Greetings! I am your connection handler\n";
   write(ClientSockNum , message , strlen(message));



   //Receive a message from client
   read_size = recv(ClientSockNum , client_message , 2000 , 0);


   message = "Looking for file... \n";
   write(ClientSockNum , message , strlen(message));
   write(ClientSockNum , client_message , strlen(client_message));


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

   if (fp == NULL) {
      message = "COULD NOT OPEN REQUESTED FILE\n";
      write(ClientSockNum , message , strlen(message));
      fclose(fp);
      exit(1);
   }

   fseek(fp, 0L, SEEK_END);
   long sz = ftell(fp);
   rewind(fp);
   sprintf(message, "Size of your file is %ld \n", sz);
   write(ClientSockNum , message , strlen(message));
   //TODO: Set mtu
   long numPackets = (sz / mtu) + 1;
   char * fileReturn = malloc(sizeof(char)*(mtu+1));
   for(int i = 0; i < numPackets; i++){
     size_t newLen = fread(fileReturn, sizeof(char), mtu+1, fp);
     write(ClientSockNum , fileReturn , strlen(fileReturn));
   }
   fclose(fp);
   close(ClientSockNum);
   exit(1);

   return 0;
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
   memset((char *)&sad,0,sizeof(sad6)); /* clear sockaddr structure      */
   memset((char *)&cad,0,sizeof(cad)); /* clear sockaddr structure      */
   sad.sin_family = AF_INET;           /* set family to Internet        */
   sad.sin_addr.s_addr = INADDR_ANY;   /* set the local IP address      */
   sad6.sin6_addr = in6addr_any;   /* set the local IP address      */
   sad6.sin6_family = AF_INET6;
   cad.sin_family = AF_INET;          /* set family to Internet         */
   cad6.sin6_family = AF_INET6;          /* set family to Internet         */
//TODO: make more error checks
   if ( (argc > 1)) {
       port =  atoi(argv[1]);
       port6 = atoi(argv[2]);
       mtu = atoi(argv[3]);
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

   if (tcpsd < 0) {
      fprintf(stderr, "tcp socket creation failed\n");
      exit(1);
   }
   if (tcpsd6 < 0) {
      fprintf(stderr, "tcp6 socket creation failed\n");
      exit(1);
   }
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

   connfd = -222;
   connfd6 = -222;
   while( (connfd = accept(tcpsd, (struct sockaddr *)&cad, &len)) ||
          (connfd6 = accept(tcpsd6, (struct sockaddr *)&cad6, &len))){
            puts("Connection accepted");
            //Reply to the client
             message = "Hello Client , I have received your connection. And now I will assign a handler for you\n";

            pthread_t sniffer_thread;
            if(connfd == -222){
              write(connfd6 , message , strlen(message));
              new_sock = malloc(1);
              *new_sock = connfd6;
            }
            if(connfd6 == -222){
              write(connfd , message , strlen(message));
              new_sock = malloc(1);
              *new_sock = connfd;
            }


            struct arg_struct args;
            args.mtu = mtu;
            args.socket = new_sock;

            if( pthread_create( &sniffer_thread , NULL ,  file_sender , (void*) &args ) < 0)
              {
                  perror("could not create thread");
                  return 1;
              }

              pthread_join( sniffer_thread, NULL);
              puts("thread assigned");

              connfd = -222;
              connfd6 = -222;
          }

}
