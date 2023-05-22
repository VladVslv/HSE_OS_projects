#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <unistd.h>     /* for close() */

#define RCVBUFSIZE 2048 /* Size of receive buffer */

void DieWithError(char *errorMessage); /* Error handling function */
char *inputString(FILE *fp, size_t size);

int main(int argc, char *argv[]) {
  int sock;                        /* Socket descriptor */
  struct sockaddr_in echoServAddr; /* Echo server address */
  unsigned short echoServPort;     /* Echo server port */
  char *servIP;                    /* Server IP address (dotted quad) */
  char *message;                   /* String to send to echo server */
  unsigned int echoStringLen;      /* Length of string to echo */

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
  memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
  echoServAddr.sin_family = AF_INET;              /* Internet address family */
  echoServAddr.sin_addr.s_addr = inet_addr(servIP); /* Server IP address */
  echoServAddr.sin_port = htons(echoServPort);      /* Server port */

  /* Establish the connection to the echo server */
  if (connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0)
    DieWithError("connect() failed");
  printf("Enter encoded message:\n");
  message = inputString(stdin, 2048);
  echoStringLen = strlen(message);
  while (strcmp(message, "end") != 0) {
    /* Send the string to the server */
    if (send(sock, message, echoStringLen, 0) != echoStringLen)
      DieWithError("send() sent a different number of bytes than expected");
    if (send(sock, "\n", 1, 0) != 1)
      DieWithError("send() sent a different number of bytes than expected");
    printf("Enter encoded message:\n");
    free(message);
    message = inputString(stdin, 2048);
    echoStringLen = strlen(message);
  }
  if (send(sock, "end", 3, 0) != 3)
    DieWithError("send() sent a different number of bytes than expected");
  free(message);
  close(sock);
  exit(0);
}
