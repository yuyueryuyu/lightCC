# Compiler settings
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -Isrc -Isrc/util
LDFLAGS := 

# Project structure
SRCDIR := src
UTILDIR := $(SRCDIR)/util
BUILDDIR := build
TARGET := compiler

# Collect all .cpp files (both src/ and src/util/)
SOURCES := $(wildcard $(SRCDIR)/*.cpp $(UTILDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(filter $(SRCDIR)/%.cpp,$(SOURCES)))
OBJECTS += $(patsubst $(UTILDIR)/%.cpp,$(BUILDDIR)/util/%.o,$(filter $(UTILDIR)/%.cpp,$(SOURCES)))
DEPENDS := $(OBJECTS:.o=.d)

# Main target
all: $(BUILDDIR)/$(TARGET)

# Link all objects into executable
$(BUILDDIR)/$(TARGET): $(OBJECTS) | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Compile src/*.cpp → build/*.o
$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Compile src/util/*.cpp → build/util/*.o
$(BUILDDIR)/util/%.o: $(UTILDIR)/%.cpp | $(BUILDDIR)/util
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Create build directories
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/util:
	mkdir -p $(BUILDDIR)/util

# Include auto-generated dependencies
-include $(DEPENDS)

# Clean build artifacts
clean:
	rm -rf $(BUILDDIR)

# Run the compiler
run: all
	./$(BUILDDIR)/$(TARGET)

.PHONY: all clean run