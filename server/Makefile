CC := g++
SFMLFLAGS := -I/usr/local/SFML/include -L/usr/local/SFML/lib -lsfml-network -lsfml-system
LDFLAGS := -lhttp -L/usr/local/lib 
COMMONFLAGS := -I../common
THREADFLAGS := -pthread
TARGET := main
SRC := main.cpp APIServer.cpp ServerSocket.cpp AppManager.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) $(SFMLFLAGS) $(COMMONFLAGS) $(LDFLAGS) $(THREADFLAGS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	echo "Cleaning up!"
	rm -f $(TARGET)