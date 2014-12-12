# Settings
# For release builds set RELEASE=1|2|3|s, depending on the desired optimization level.
ifeq ($(RELEASE),)
    ADDITIONAL_FLAGS = -g -DDEBUG
else
    ADDITIONAL_FLAGS = -O$(RELEASE)
endif
CFLAGS = -Wall -MMD -std=c++11 $(ADDITIONAL_FLAGS)
LFLAGS = -Wall $(DEBUG)

# Files
BUILDDIR = build
TARGET = logic
TEST_TARGET = $(BUILDDIR)/testsuite

CPPS = core/lisp.cpp core/logic.cpp core/namespace.cpp core/theory.cpp \
       core/traverse.cpp core/atom.cpp

TESTS = write
TEST_CPPS = $(patsubst %,test/%.cpp,$(TESTS))

OBJS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(CPPS))
TEST_OBJS = $(patsubst test/%.cpp,$(BUILDDIR)/test/%.o,$(TEST_CPPS))

DOCS = $(patsubst %.md,%.html,$(wildcard doc/*.md))

# Compile everything
all: $(BUILDDIR)/ $(BUILDDIR)/core/ $(BUILDDIR)/test/ $(TEST_TARGET) # $(TARGET)

# Main target
$(TARGET): $(OBJS)
	$(CXX) $(LFLAGS) -o $@ $^

# Tests
test: $(TEST_TARGET)
	$(TEST_TARGET)

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
	$(CXX) $(LFLAGS) -lboost_unit_test_framework -o $@ $^

# Object files
$(OBJS): $(BUILDDIR)/%.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

$(TEST_OBJS): $(BUILDDIR)/test/%.o: test/%.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

# Use the dependency files created by the compiler
-include $(patsubst %.cpp,%.d,$(CPPS) $(TEST_CPPS))

# Directories
$(BUILDDIR)/:
	mkdir $@

$(BUILDDIR)/%/:
	mkdir $@

doc: $(DOCS)

$(DOCS): doc/%.html: doc/%.md
	@echo -e "<!DOCTYPE html>\n<html>\n<head>\n\t<title>$^</title>\n\
		<meta charset='utf-8'></head>\n<body>" >$@
	markdown $^ >>$@
	@echo -e "</body>\n</html>" >>$@

clean:
	-rm $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)
	-rm $(DOCS)

.PHONY: all test clean doc
