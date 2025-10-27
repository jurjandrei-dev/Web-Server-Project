CC := g++
SFMLFLAGS := -I/usr/local/SFML/include -L/usr/local/SFML/lib -lsfml-network -lsfml-system
TARGET := main
SRC := main.cpp APIClient.cpp ClientSocket.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(SFMLFLAGS) -o $(TARGET)

clean:
	echo "Cleaning up!"
	rm main