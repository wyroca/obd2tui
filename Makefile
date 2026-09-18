CC := gcc
LD := gcc

CFLAGS := -g -Wall -Wextra -Werror

LIBS := -lncurses -lmenu
LFLAGS := $(LIBS)

TARGET := obd2tui
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean test

test:
	@echo "SRCS = $(SRCS)"
	@echo "OBJS = $(OBJS)"
	@echo "TARGET = $(TARGET)"

clean: 
	rm -f $(TARGET) $(OBJS)
