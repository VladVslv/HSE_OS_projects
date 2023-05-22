#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <sys/types.h>
#include <unistd.h> /* for close() */

#define RCVBUFSIZE 2048 /* Size of receive buffer */

void DieWithError(char *errorMessage); /* Error handling function */

int main(int argc, char *argv[])
{
  int sock;                        /* Socket descriptor */
  struct sockaddr_in echoServAddr; /* Echo server address */
  unsigned short echoServPort;     /* Echo server port */
  char *servIP;                    /* Server IP address (dotted quad) */
  char echoBuffer[RCVBUFSIZE];     /* Buffer for echo string */
  int bytesRcvd;                   /* Bytes read in single recv()
                                                      and total bytes read */

  if (argc != 3) /* Test for correct number of arguments */
  {
    fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
    exit(1);
  }

  servIP = argv[1]; /* First arg: server IP address (dotted quad) */

  echoServPort = atoi(argv[2]); /* Use given port, if any */

  /* Create a reliable, stream socket using TCP */
  if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    DieWithError("socket() failed");

  /* Construct the server address structure */
  memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
  echoServAddr.sin_family = AF_INET;                /* Internet address family */
  echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
  echoServAddr.sin_port = htons(echoServPort);      /* Server port */
  /* Establish the connection to the echo server */
  if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    DieWithError("connect() failed");
  while (1)
  {
    /* Receive up to the buffer size (minus 1 to leave space for
    a null terminator) bytes from the sender */
    if ((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) < 0)
    {
      DieWithError("recv() failed or connection closed prematurely");
    }
    echoBuffer[bytesRcvd] = '\0';              /* Terminate the string! */
    if (bytesRcvd == 0 || strcmp(echoBuffer, "end") == 0)
    {
      break;
    }
    printf("Decoded message: %s", echoBuffer); /* Print the echo buffer */
  }
  close(sock);
  exit(0);
}
