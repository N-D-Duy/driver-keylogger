CC      = gcc
CFLAGS  = -Iinclude -Wall -O2
#wayland
LDFLAGS = -lX11

SRCDIR  = src
OBJDIR  = obj
TARGET_DEVICE   = logkey_client

SRCS_DEVICE     := $(filter-out $(SRCDIR)/main.c, $(wildcard $(SRCDIR)/*.c))
OBJS_DEVICE     := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS_DEVICE))

all: $(TARGET_DEVICE)

$(TARGET_DEVICE): $(OBJS_DEVICE)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(TARGET_DEVICE) $(OBJDIR)

.PHONY: all clean
