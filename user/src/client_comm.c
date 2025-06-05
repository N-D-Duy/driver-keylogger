#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>
#include "client_comm.h"

#define NETLINK_ID 31
#define MAX_PAYLOAD 1024

static int sock_fd;
static struct sockaddr_nl src_addr, dest_addr;

int client_comm_init(void) {
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ID);
    if (sock_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; // kernel
    dest_addr.nl_groups = 0;

    // Send initial message to register
    struct nlmsghdr *nlh = malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(0);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    sendto(sock_fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    free(nlh);

    return 0;
}

int client_comm_recv(char *buffer, size_t size) {
    struct nlmsghdr *nlh = malloc(NLMSG_SPACE(MAX_PAYLOAD));
    int ret;

    ret = recv(sock_fd, nlh, NLMSG_SPACE(MAX_PAYLOAD), 0);
    if (ret < 0) {
        perror("recv");
        free(nlh);
        return -1;
    }

    size_t len = nlh->nlmsg_len - NLMSG_HDRLEN;
    if (len > size) len = size;
    memcpy(buffer, NLMSG_DATA(nlh), len);
    buffer[len] = '\0';
    free(nlh);
    return len;
}

void client_comm_cleanup(void) {
    close(sock_fd);
}
