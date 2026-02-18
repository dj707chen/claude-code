CC = cc
CFLAGS = -O2 -Wall -Wextra -pedantic
LDFLAGS = -lcurl
TARGET = weather
SRC = weather_cli.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
