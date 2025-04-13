# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Source files
SRCS = main2.cpp DataBase.cpp  page.cpp Table.cpp tuple.cpp ExcuetionEngine.cpp parser.cpp

# Header files
HDRS = DataBase.hpp page.hpp Table.hpp tuple.hpp ExcuetionEngine.hpp parser.hpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Output executable
TARGET = program

# Default rule
all: $(TARGET)

# Link the object files to create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.cpp $(HDRS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)

# Phony targets
.PHONY: all clean
