
CFLAGS  += -g -D_XOPEN_SOURCE=700 -std=c11 -Wall -Wextra -Wpedantic
LDFLAGS +=
LDLIBS  +=

ifeq ($(CC),clang)
	CFLAGS += -Weverything
endif

SRC = src
OBJ = obj
DEP = .dep

TARGET  = httpd
SOURCES = $(wildcard $(SRC)/*.c $(SRC)/**/*.c)
OBJECTS = $(patsubst $(SRC)/%.c,$(OBJ)/%.o,$(SOURCES))

$(shell mkdir -p $(SRC) $(OBJ) $(DEP))

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf $(DEP) $(OBJ) $(TARGET)

$(OBJ)/%.o : $(SRC)/%.c
	$(CC) $(CFLAGS) -MM -MP -MT $@ -o $(DEP)/$*.d $<
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

valgrind: $(TARGET)
	valgrind ./$(TARGET)

gdb: $(TARGET)
	gdb --args ./$(TARGET)

$(DEP)/%.d: ;
.PRECIOUS: $(DEP)/%.d
-include $(patsubst $(SRC)/%.c,$(DEP)/%.d,$(SOURCES))

