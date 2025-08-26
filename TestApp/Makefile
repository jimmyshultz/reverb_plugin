# Simple Makefile for testing reverb algorithm
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
TARGET = reverb_test
SOURCE = main.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)
	@echo "✅ Build complete! Run with: ./$(TARGET)"

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) reverb_test_output.txt

install-deps-ubuntu:
	sudo apt update
	sudo apt install build-essential

install-deps-macos:
	# Requires Xcode command line tools
	xcode-select --install

help:
	@echo "Available targets:"
	@echo "  all          - Build the test application"
	@echo "  run          - Build and run the test"
	@echo "  clean        - Remove build files"
	@echo "  install-deps-ubuntu - Install dependencies on Ubuntu"
	@echo "  install-deps-macos  - Install dependencies on macOS"