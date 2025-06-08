
// ===== Makefile =====
# Makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=native -I include
LDFLAGS = -pthread

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = build
BINDIR = bin
TESTDIR = tests

# Files
CORE_SOURCES = $(wildcard $(SRCDIR)/core/*.cpp)
IO_SOURCES = $(wildcard $(SRCDIR)/io/*.cpp)
UTILS_SOURCES = $(wildcard $(SRCDIR)/utils/*.cpp)
ALL_SOURCES = $(CORE_SOURCES) $(IO_SOURCES) $(UTILS_SOURCES)

OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(ALL_SOURCES))

# Main target
TARGET = $(BINDIR)/matching_engine

# Default target
all: directories $(TARGET)

# Create directories
directories:
	@mkdir -p $(OBJDIR)/core $(OBJDIR)/io $(OBJDIR)/utils $(BINDIR)

# Main executable
$(TARGET): $(OBJECTS) main.cpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp $(OBJECTS) $(LDFLAGS)

# Object files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Run with example
run: $(TARGET)
	./$(TARGET) input.csv output.csv

# Phony targets
.PHONY: all clean directories run
