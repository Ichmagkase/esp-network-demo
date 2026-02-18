#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define INET_ADDR "127.0.0.1"
#define PORT_NUMBER 2222
#define BUF_SIZE 32

#define error(message)                                                         \
  {                                                                            \
    fprintf(stderr, "%s: %s: %s\n", argv[0], message, strerror(errno));        \
    exit(1);                                                                   \
  }

int main(int argc, char *argv[]) {
  printf("Starting server\n");

  struct sockaddr_in svaddr, claddr;
  int sockfd, j;
  ssize_t numBytes;
  socklen_t len;
  char buf[BUF_SIZE];
  char claddrStr[INET_ADDRSTRLEN];

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1)
    error("socket sockfd");

  memset(&svaddr, 0, sizeof(struct sockaddr_in));
  svaddr.sin_family = AF_INET;
  svaddr.sin_addr.s_addr = inet_addr(INET_ADDR);
  svaddr.sin_port = htons(PORT_NUMBER);

  if (bind(sockfd, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_in)) ==
      -1)
    error("bind");

  for (;;) {
    len = sizeof(struct sockaddr_in);
    numBytes =
        recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *)&claddr, &len);
    if (numBytes == -1)
      error("recvfrom");

    if (inet_ntop(AF_INET, &claddr.sin_addr, claddrStr, INET_ADDRSTRLEN) ==
        NULL)
      printf("Couldn't convert address to string\n");
    else
      printf("Server received %ld bytes from (%s, %u)\n", (long)numBytes,
             claddrStr, ntohs(claddr.sin_port));

    for (j = 0; j < numBytes; j++)
      buf[j] = toupper((unsigned char)buf[j]);

    if (sendto(sockfd, buf, numBytes, 0, (struct sockaddr *)&claddr, len) !=
        numBytes)
      error("sendto");
  }
}
