CXX = g++
CXXFLAGS = -std=c++17 -O3 -Iinclude -msse2

TARGET = backpipe

SRCS = $(shell find . -name "*.cpp")
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ 

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
