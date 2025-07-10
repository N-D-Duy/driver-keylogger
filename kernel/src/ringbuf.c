#include <linux/slab.h>
#include <linux/spinlock.h>
#include "ringbuf.h"

struct ring_buffer {
    char *data;
    size_t size;
    size_t head, tail;
    spinlock_t lock;
};

struct ring_buffer *ringbuf_create(size_t size) {
    struct ring_buffer *rb = kzalloc(sizeof(*rb), GFP_KERNEL);
    if (!rb) return NULL;
    rb->data = kzalloc(size, GFP_KERNEL);
    if (!rb->data) {
        kfree(rb);
        return NULL;
    }
    rb->size = size;
    spin_lock_init(&rb->lock);
    return rb;
}

void ringbuf_free(struct ring_buffer *rb) {
    if (rb) {
        kfree(rb->data);
        kfree(rb);
    }
}

int ringbuf_write(struct ring_buffer *rb, const char *data, size_t len) {
    size_t i;
    spin_lock(&rb->lock);
    for (i = 0; i < len; ++i) {
        size_t next = (rb->head + 1) % rb->size;
        if (next == rb->tail) break; // full
        rb->data[rb->head] = data[i];
        rb->head = next;
    }
    spin_unlock(&rb->lock);
    return i;
}

int ringbuf_read(struct ring_buffer *rb, char *out, size_t maxlen) {
    size_t i = 0;
    spin_lock(&rb->lock);
    while (i < maxlen && rb->tail != rb->head) {
        out[i++] = rb->data[rb->tail];
        rb->tail = (rb->tail + 1) % rb->size;
    }
    spin_unlock(&rb->lock);
    return i;
}

bool ringbuf_is_empty(struct ring_buffer *rb) {
    return rb->head == rb->tail;
}

void ringbuf_clear(struct ring_buffer *rb) {
    rb->head = rb->tail = 0;
}