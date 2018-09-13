IDIR = .
ODIR = .

CC = gcc 
CFLAGS = -Wall -g -I$(IDIR)

LIBS = -lm

_DEPS = cdf.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJS = trace_generator.o cdf.o 
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: trace_generator

trace_generator: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o