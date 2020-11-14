CC = gcc
CXX = g++

CPPFLAGS = -I..
CFLAGS = -g -std=gnu99 -Wall -Wno-unused-parameter -Wno-unused-function -O3
CXXFLAGS = -g -Wall -O3
LDFLAGS = -lpthread -lm

TARGETS := distributed_vision

.PHONY: all
all: distributed_vision

distributed_vision: bin/distributed_vision.bin apriltag/libapriltag.a
	@echo "   [$@]"
	@$(CXX) -o $@ $^ $(LDFLAGS) `pkg-config --cflags --libs opencv`

%.o: %.c
	@echo "   $@"
	@$(CC) -o $@ -c $< $(CFLAGS) $(CPPFLAGS)

%.o: %.cc
	@echo "   $@"
	@$(CXX) -o $@ -c $< $(CXXFLAGS) $(CPPFLAGS)

.PHONY: clean
clean:
	@rm -rf *.bin $(TARGETS)
