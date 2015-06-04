# Settings
# For debug builds set VARIANT = debug.
VARIANT ?= release
ifeq ($(VARIANT),release)
    ADDITIONAL_FLAGS = -O2
else
    ADDITIONAL_FLAGS = -ggdb3 -DDEBUG
endif

CFLAGS = -Wall -MMD -std=c++11 $(ADDITIONAL_FLAGS)
LFLAGS = -Wall

# Files
BUILDDIR = $(VARIANT)
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

DOC_DIR = doc
DOCS = $(DOC_DIR)/language.md
DOXY_CONFIG = $(DOC_DIR)/doxygen.cfg

# Compile everything
all: $(BUILDDIR)/ $(BUILDDIR)/core/ $(BUILDDIR)/test/ $(BUILDDIR)/tools/ \
	$(TEST_TARGETS)	$(TOOL_TARGETS)# $(TARGET)

# Main target
$(TARGET): $(OBJS)
	$(CXX) $(LFLAGS) -o $@ $^

# Tools
tools: $(TOOL_TARGETS)

$(TOOL_TARGETS): $(BUILDDIR)/%: $(OBJS) $(BUILDDIR)/tools/%.o
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

# Use a wildcard here, since doxygen also scans all sources
doc: $(wildcard core/*.*) $(DOCS)
	doxygen $(DOXY_CONFIG)

clean:
	-rm -rf debug/ release/
	-rm $(TARGET)
	-rm -rf $(DOC_DIR)/html

.PHONY: all test tools clean doc

# Use the dependency files created by the compiler
-include $(patsubst %.o,%.d,$(OBJS) $(TEST_OBJS))
