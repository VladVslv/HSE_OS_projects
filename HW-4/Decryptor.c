#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdbool.h>

#define BUFFER_SIZE 2048 /* Buffer size */

void DieWithError(char *errorMessage); /* External error handling function */

char get_char(int code)
{
  return (char)code;
}

int main(int argc, char *argv[])
{
    int sock;                        /* Socket */
    struct sockaddr_in servAddr; /* Local address */
    struct sockaddr_in clntAddr; /* Client address */
    unsigned int cliAddrLen;         /* Length of incoming message */
    char inputString[BUFFER_SIZE];        /* Buffer for input string */
    unsigned short servPort;     /* Server port */
    int recvMsgSize;                 /* Size of received message */
    char resultBuffer[BUFFER_SIZE];
    int i, num;
    char *token;

    if (argc != 2)
    { /* Test for correct number of parameters */
        fprintf(stderr, "Usage:  %s <Serv Port>\n", argv[0]);
        exit(1);
    }

    servPort = atoi(argv[1]); /* First arg:  local port */

    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(servPort);      /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    while (true)
    { /* Run forever */
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(clntAddr);
        

        /* Block until receive message from a client */
        if ((recvMsgSize = recvfrom(sock, inputString, BUFFER_SIZE, 0,
                                    (struct sockaddr *)&clntAddr, &cliAddrLen)) < 0)
            DieWithError("recvfrom() failed");

        printf("Recieved encoded message from client %s\n\n", inet_ntoa(clntAddr.sin_addr));

        inputString[recvMsgSize] = '\0';
        i = 0;
        token = strtok(inputString, " ");
        while (token != NULL)
        {
            num = atoi(token);
            resultBuffer[i++] = get_char(num);
            token = strtok(NULL, " ");
        }
        
        if(strcmp(inputString, "end") == 0){
        	exit(0);
        }
        
        printf("Sent decoded message to client %s\n\n", inet_ntoa(clntAddr.sin_addr));

        /* Send received datagram back to the client */
        if (sendto(sock, resultBuffer, i, 0,
                   (struct sockaddr *)&clntAddr, sizeof(clntAddr)) != i)
            DieWithError("sendto() sent a different number of bytes than expected");
    }
}
