#ifndef LOGGER_H
#define LOGGER_H

int logger_open(const char *path);
void logger_write(const char *data);
void logger_close(void);

#endif // LOGGER_H