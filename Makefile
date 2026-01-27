CC = cc
CFLAGS = -Wall -O2
TARGET = aecal
PREFIX ?= /usr/local
BINDIR = $(DESTDIR)$(PREFIX)/bin

all: $(TARGET)

$(TARGET): aecal.c
	$(CC) $(CFLAGS) -o $(TARGET) aecal.c

install: $(TARGET)
	mkdir -p $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)

uninstall:
	rm -f $(BINDIR)/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
