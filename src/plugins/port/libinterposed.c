#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>

int __interposed_ipc = 3;

/* __attribute__((constructor)) static void __interposed_init() { */
/*   struct sockaddr_un address; */
/*   char* ipc_fd = getenv("INTERPOSED_IPC"); */
/*  */
/*   if (ipc_fd == NULL) { */
/*     ipc_fd = "3"; */
/*   } */
/*  */
/*   __interposed_ipc = socket(PF_UNIX, SOCK_STREAM, 0); */
/*   if (__interposed_ipc < 0) { */
/*     perror("socket(PF_UNIX, SOCK_STREAM, 0)"); */
/*     return; */
/*   } */
/*  */
/*   memset(&address, 0, sizeof(struct sockaddr_un)); */
/*   address.sun_family = AF_UNIX; */
/*   snprintf(address.sun_path, sizeof(address.sun_path) - 1, "%s", ipc_fd); */
/*   connect(__interposed_ipc, (struct sockaddr*) &address, sizeof(struct sockaddr_un)); */
/* } */

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int result;
  int desired;
  char msg[128];
  int msg_length;
  struct sockaddr_in* in_addr;
  static const int (*original_bind) (int, const struct sockaddr *, socklen_t) = NULL;

  if (!original_bind) {
    original_bind = dlsym(RTLD_NEXT, "bind");
  }

  if (addr->sa_family == AF_INET) {
    in_addr = (struct sockaddr_in*) addr;
    desired = in_addr->sin_port;
    result = original_bind(sockfd, addr, addrlen);
    while (result = -1 && (errno == EADDRINUSE || errno == EACCES)) {
      in_addr->sin_port = ntohs(htons(in_addr->sin_port) + 1);
      in_addr->sin_addr.s_addr = INADDR_ANY;
      result = original_bind(sockfd, addr, addrlen);
    }

    msg_length = sprintf(msg, "port=%d\n", htons(in_addr->sin_port));
    write(__interposed_ipc, msg, msg_length);
    return result;
  }

  return original_bind(sockfd, addr, addrlen);
}
