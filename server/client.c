#include <arpa/inet.h>
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
  printf("Starting client\n");

  int sockfd, j;
  size_t msgLen;
  ssize_t numBytes;
  char resp[BUF_SIZE];

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd == -1)
    error("socket");

  struct sockaddr_in svaddr;
  memset(&svaddr, 0, sizeof(struct sockaddr_in));
  svaddr.sin_addr.s_addr = inet_addr(INET_ADDR);
  svaddr.sin_port = htons(PORT_NUMBER);
  if (inet_pton(AF_INET, INET_ADDR, &svaddr.sin_addr) <= 0)
    error("inet_pton");

  // Why doesn't the client need this?
  // if (bind(sockfd, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_in)) ==
  //     -1)
  //   exit(1);

  for (j = 1; j < argc; j++) {
    msgLen = strlen(argv[j]);
    if (sendto(sockfd, argv[j], msgLen, 0, (struct sockaddr *)&svaddr,
               sizeof(struct sockaddr_in)) != msgLen)
      error("sendto");

    numBytes = recvfrom(sockfd, resp, BUF_SIZE, 0, NULL, NULL);
    if (numBytes == -1)
      error("recvfrom");

    printf("Response %d: %.*s\n", j - 1, (int)numBytes, resp);
  }
}
