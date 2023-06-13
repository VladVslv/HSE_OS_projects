#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdbool.h>

#define BUFFER_SIZE 2048 /* Buffer size */

void DieWithError(char *errorMessage); /* External error handling function */
char *getStr(FILE *fp, size_t size);

int main(int argc, char *argv[])
{
    int sock;                         /* Socket descriptor */
    struct sockaddr_in servAddr;      /* Server address */
    struct sockaddr_in fromAddr;      /* Source address */
    unsigned short servPort;          /* Server port */
    unsigned int fromSize;            /* In-out of address size for recvfrom() */
    char *servIP;                     /* IP address of server */
    char *inputString;                /* String to send*/
    char outputString[BUFFER_SIZE + 1]; /* Buffer for receiving */
    int inputStringLen;               /* Length of string to send */
    int respStringLen;                /* Length of received string */

    if (argc < 2) /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage: %s <Server IP> <Serv Port>\n", argv[0]);
        exit(1);
    }

    servIP = argv[1]; /* First arg: server IP address (dotted quad) */

    servPort = atoi(argv[2]); 

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&servAddr, 0, sizeof(servAddr));       /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet addr family */
    servAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
    servAddr.sin_port = htons(servPort);          /* Server port */

    while (true)
    {

        printf("Enter encoded message:\n");
        inputString = getStr(stdin, 2048);
        
        /* Send the string to the server */
        if ((inputStringLen = strlen(inputString)) > BUFFER_SIZE) /* Check input length */
            DieWithError("String is too long");
        if (sendto(sock, inputString, inputStringLen, 0, (struct sockaddr *)&servAddr, sizeof(servAddr)) != inputStringLen)
            DieWithError("sendto() sent a different number of bytes than expected");

        if (strcmp(inputString, "end") == 0)
        {
            break;
        }

        /* Recv a response */
        fromSize = sizeof(fromAddr);
        if ((respStringLen = recvfrom(sock, outputString, BUFFER_SIZE, 0,
                                      (struct sockaddr *)&fromAddr, &fromSize)) < 0)
            DieWithError("recvfrom() failed");

        if (servAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
        {
            fprintf(stderr, "Error: received a packet from unknown source.\n");
            exit(1);
        }
        /* null-terminate the received data */
        outputString[respStringLen] = '\0';
        printf("\nDecoded message: \n%s\n\n", outputString);
    }
    close(sock);
    exit(0);
}
