CC = cc
CFLAGS = -Wall -O2
TARGET = aecal

all: $(TARGET)

$(TARGET): aecal.c
	$(CC) $(CFLAGS) -o $(TARGET) aecal.c

clean:
	rm -f $(TARGET)
