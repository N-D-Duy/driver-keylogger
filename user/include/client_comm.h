#ifndef CLIENT_COMM_H
#define CLIENT_COMM_H

int client_comm_init(void);
int client_comm_recv(char *buffer, size_t size);
void client_comm_cleanup(void);

#endif // CLIENT_COMM_H