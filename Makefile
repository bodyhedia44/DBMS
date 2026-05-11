# Compiler and flags
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -I include

# Source files
SRCS = src/main.cpp \
       src/storage/DataBase.cpp \
       src/storage/page.cpp \
       src/storage/Table.cpp \
       src/storage/tuple.cpp \
       src/engine/ExecutionEngine.cpp \
       src/parser/parser.cpp \
       src/buffer/Buffer.cpp

# Object files (placed alongside sources)
OBJS = $(SRCS:.cpp=.o)

# Output executable
TARGET = dbms

# Default rule
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Remove database data files
cleandata:
	rm -rf data/*

.PHONY: all clean cleandata
