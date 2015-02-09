# Settings
# For debug builds set DEBUG=0|1|2|3|s, depending on the desired optimization level.
ifeq ($(DEBUG),)
    ADDITIONAL_FLAGS = -O2
else
    ADDITIONAL_FLAGS = -ggdb3 -DDEBUG -O$(DEBUG)
endif
CFLAGS = -Wall -MMD -std=c++11 $(ADDITIONAL_FLAGS)
LFLAGS = -Wall $(DEBUG)

# Files
BUILDDIR = build
TARGET = logic

CPPS =	core/base.cpp \
	core/debug.cpp \
	core/expression.cpp \
	core/lisp.cpp \
	core/logic.cpp \
	core/theory.cpp \
	core/traverse.cpp \
	core/tree.cpp

TOOLS = parser
TOOL_CPPS = $(patsubst %,tools/%.cpp,$(TOOLS))
TOOL_TARGETS = $(patsubst %,$(BUILDDIR)/%,$(TOOLS))

TESTS = core
TEST_CPPS = $(patsubst %,test/%.cpp,$(TESTS))
TEST_TARGETS = $(patsubst %,$(BUILDDIR)/test/%test,$(TESTS))

OBJS = $(patsubst %.cpp,$(BUILDDIR)/%.o,$(CPPS))

DOCS = $(patsubst %.md,%.html,$(wildcard doc/*.md))

# Compile everything
all: $(BUILDDIR)/ $(BUILDDIR)/core/ $(BUILDDIR)/test/ $(BUILDDIR)/tools/ \
	$(TEST_TARGETS)	$(TOOL_TARGETS)# $(TARGET)

# Main target
$(TARGET): $(OBJS)
	$(CXX) $(LFLAGS) -o $@ $^

# Tools
tools: $(TOOL_TARGETS)

$(TOOL_TARGETS): build/%: $(OBJS) build/tools/%.o
	$(CXX) $(LFLAGS) -o $@ $^

# Tests
test: $(TEST_TARGETS)
	$(patsubst %,% &&,$(TEST_TARGETS)) true

$(TEST_TARGETS): %test: $(OBJS) %.o
	$(CXX) $(LFLAGS) -lboost_unit_test_framework -o $@ $^

# Object files
$(BUILDDIR)/%.o: %.cpp
	$(CXX) -c $(CFLAGS) -o $@ $<

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
	-rm -rf build/
	-rm $(TARGET)
	-rm $(DOCS)

.PHONY: all test tools clean doc

# Use the dependency files created by the compiler
-include $(patsubst %.o,%.d,$(OBJS) $(TEST_OBJS))
