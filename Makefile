# Makefile for arp-interleaver-designer

CC      = gcc
CFLAGS  = -O2 -std=c99 -Wall -Wextra -Isrc -Iinclude
LDFLAGS = -lm

TARGET = arp_designer
SRCDIR = src
INCDIR = include

SRCS = $(SRCDIR)/main.c \
       $(SRCDIR)/encoder.c \
       $(SRCDIR)/interleaver.c \
       $(SRCDIR)/rtz_search.c \
       $(SRCDIR)/designer.c \
       $(SRCDIR)/utils.c

OBJS = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(TARGET)

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)
	rm -f result.txt final_result.txt temp.txt _dedup_tmp.txt
	rm -f output_*.txt layer_*.txt

# Header dependencies
$(SRCDIR)/main.o:        $(INCDIR)/config.h $(INCDIR)/encoder.h $(INCDIR)/interleaver.h $(INCDIR)/designer.h $(INCDIR)/utils.h
$(SRCDIR)/encoder.o:     $(INCDIR)/config.h $(INCDIR)/encoder.h $(INCDIR)/utils.h
$(SRCDIR)/interleaver.o: $(INCDIR)/config.h $(INCDIR)/interleaver.h
$(SRCDIR)/rtz_search.o:  $(INCDIR)/config.h $(INCDIR)/encoder.h $(INCDIR)/rtz_search.h $(INCDIR)/utils.h
$(SRCDIR)/designer.o:    $(INCDIR)/config.h $(INCDIR)/encoder.h $(INCDIR)/interleaver.h $(INCDIR)/rtz_search.h $(INCDIR)/designer.h $(INCDIR)/utils.h
$(SRCDIR)/utils.o:       $(INCDIR)/config.h $(INCDIR)/encoder.h $(INCDIR)/utils.h
