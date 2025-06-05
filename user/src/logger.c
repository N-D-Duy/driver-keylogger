#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"

static FILE *log_file = NULL;

int logger_open(const char *path) {
    log_file = fopen(path, "a");
    return log_file != NULL ? 0 : -1;
}

void logger_write(const char *data) {
    if (log_file) {
        fputs(data, log_file);
        fflush(log_file);
    }
}

void logger_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}
