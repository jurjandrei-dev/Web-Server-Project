CC := g++
SFMLFLAGS := -I/usr/local/SFML/include -L/usr/local/SFML/lib -lsfml-network -lsfml-system
THREADFLAGS := -pthread
TARGET := main
SRC := main.cpp APIServer.cpp ServerSocket.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(SFMLFLAGS) -o $(TARGET)

clean:
	echo "Cleaning up!"
	rm main 