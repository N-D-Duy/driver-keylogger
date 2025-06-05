#ifndef NETLINK_COMM_H
#define NETLINK_COMM_H

int netlink_init(void);
void netlink_exit(void);
void netlink_send(const char *data, size_t len);

#endif // NETLINK_COMM_H
