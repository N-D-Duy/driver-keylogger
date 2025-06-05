#ifndef RINGBUF_H
#define RINGBUF_H

#include <linux/types.h>

struct ring_buffer;

struct ring_buffer *ringbuf_create(size_t size);
void ringbuf_free(struct ring_buffer *rb);
int ringbuf_write(struct ring_buffer *rb, const char *data, size_t len);
int ringbuf_read(struct ring_buffer *rb, char *data, size_t maxlen);
bool ringbuf_is_empty(struct ring_buffer *rb);

#endif // RINGBUF_H
