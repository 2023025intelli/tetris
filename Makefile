CC ?= gcc
LIBS = -lncurses -lmenu -lpthread -lm -ldl

SRC = main.c tetris.c

.PHONY=all clean

OBJDIR := objdir
OBJS := $(addprefix $(OBJDIR)/,$(SRC:.c=.o))

all: tetris

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

tetris: $(OBJS)
	$(CC) -o $(@F) $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS)
	rm -rf $(OBJDIR)
	rm -f tetris
