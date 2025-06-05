#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include "netlink_comm.h"

#define NETLINK_ID 31
static struct sock *nl_sock = NULL;
static u32 user_pid = 0;

static void nl_recv(struct sk_buff *skb) {
    struct nlmsghdr *nlh = nlmsg_hdr(skb);
    user_pid = nlh->nlmsg_pid;
}

int netlink_init(void) {
    struct netlink_kernel_cfg cfg = {
        .input = nl_recv,
    };
    nl_sock = netlink_kernel_create(&init_net, NETLINK_ID, &cfg);
    return nl_sock ? 0 : -ENOMEM;
}

void netlink_exit(void) {
    netlink_kernel_release(nl_sock);
}

void netlink_send(const char *data, size_t len) {
    struct sk_buff *skb;
    struct nlmsghdr *nlh;

    if (!user_pid) return;

    skb = nlmsg_new(len, GFP_KERNEL);
    if (!skb) return;

    nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, len, 0);
    memcpy(nlmsg_data(nlh), data, len);

    netlink_unicast(nl_sock, skb, user_pid, MSG_DONTWAIT);
}
