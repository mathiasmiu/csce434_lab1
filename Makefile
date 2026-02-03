CXX := g++
TARGET := 434fe
SRC := main.cpp

.PHONY: clean build

build:

	$(CXX) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
