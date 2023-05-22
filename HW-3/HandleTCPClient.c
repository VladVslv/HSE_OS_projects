#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>
#include <stdlib.h>

#define RCVBUFSIZE 2048 /* Size of receive buffer */

void DieWithError(char *errorMessage); /* Error handling function */

char get_char(int code)
{
  return (char)code;
}

void HandleTCPClient(int clntSocketSender, int clntSocketReceiver)
{
  char echoBuffer[RCVBUFSIZE]; /* Buffer for echo string */
  char resultBufffer[RCVBUFSIZE];
  int i, num;
  int recvMsgSize; /* Size of received message */

  /* Receive message from client */
  if ((recvMsgSize = recv(clntSocketSender, echoBuffer, RCVBUFSIZE, 0)) < 0)
    DieWithError("recv() failed");

  char *token;

  /* Send received string and receive again until end of transmission */
  while (recvMsgSize > 0 && strcmp(echoBuffer, "end") != 0) /* zero indicates end of transmission */
  {
    echoBuffer[recvMsgSize] = '\0';
    i = 0;
    token = strtok(echoBuffer, " ");
    while (token != NULL)
    {
      num = atoi(token);
      resultBufffer[i++] = get_char(num);
      token = strtok(NULL, " ");
    }
    /* Echo message back to client */
    resultBufffer[i++] = '\n';
    if (send(clntSocketReceiver, resultBufffer, i, 0) != i)
      DieWithError("send() failed");

    /* See if there is more data to receive */
    if ((recvMsgSize = recv(clntSocketSender, echoBuffer, RCVBUFSIZE, 0)) < 0)
      DieWithError("recv() failed");
  }
  if (recvMsgSize > 0)
  {
    if (send(clntSocketReceiver, echoBuffer, recvMsgSize, 0) != recvMsgSize)
      DieWithError("send() failed");
  }
  close(clntSocketSender);   /* Close client sender socket */
  close(clntSocketReceiver); /* Close client receiver socket */
}
