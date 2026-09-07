CC := gcc
LD := gcc

CFLAGS := -Wall -Wextra -Werror

LIBS := -lncurses -lmenu
LFLAGS := $(LIBS)

TARGET := obd2tui
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,%.o,$(SRCS))

$(TARGET): $(OBJS)
	$(CC) $(LFLAGS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean test

test:
	@echo "SRCS = $(SRCS)"
	@echo "OBJS = $(OBJS)"

clean: 
	rm -f $(TARGET) $(OBJS)
