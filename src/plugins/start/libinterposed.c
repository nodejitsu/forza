#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>

int __interposed_ipc = 3;

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int result;
  char msg[128];
  int msg_length;
  struct sockaddr_in* in_addr;

  struct sockaddr_in actual;
  socklen_t actual_len = sizeof(actual);

  static const int (*original_bind) (int, const struct sockaddr *, socklen_t) = NULL;

  if (!original_bind) {
    original_bind = dlsym(RTLD_NEXT, "bind");
  }

  if (addr->sa_family == AF_INET) {
    in_addr = (struct sockaddr_in*) addr;
    in_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    errno = 0;
    result = original_bind(sockfd, addr, addrlen);
    while (result != 0 && (errno == EADDRINUSE || errno == EACCES)) {
      in_addr->sin_port = ntohs(htons(in_addr->sin_port) + 1);

      errno = 0;
      result = original_bind(sockfd, addr, addrlen);
    }

    getsockname(sockfd, (struct sockaddr*) &actual, &actual_len);
    msg_length = sprintf(msg, "port=%d\n", htons(actual.sin_port));
    write(__interposed_ipc, msg, msg_length);
    return result;
  }

  return original_bind(sockfd, addr, addrlen);
}

int _so_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen, int vers) {
  int result;
  char msg[128];
  int msg_length;
  struct sockaddr_in* in_addr;

  struct sockaddr_in actual;
  socklen_t actual_len = sizeof(actual);

  static const int (*original_bind) (int, const struct sockaddr *, socklen_t, int) = NULL;

  if (!original_bind) {
    original_bind = dlsym(RTLD_NEXT, "_so_bind");
  }

  if (addr->sa_family == AF_INET) {
    in_addr = (struct sockaddr_in*) addr;
    in_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    errno = 0;
    result = original_bind(sockfd, addr, addrlen, vers);
    while (result != 0 && (errno == EADDRINUSE || errno == EACCES)) {
      in_addr->sin_port = ntohs(htons(in_addr->sin_port) + 1);

      errno = 0;
      result = original_bind(sockfd, addr, addrlen, vers);
    }

    getsockname(sockfd, (struct sockaddr*) &actual, &actual_len);
    msg_length = sprintf(msg, "port=%d\n", htons(actual.sin_port));
    write(__interposed_ipc, msg, msg_length);
    return result;
  }

  return original_bind(sockfd, addr, addrlen, vers);
}
