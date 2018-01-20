MAKE=make --no-print-directory
CC=gcc

BUILD:=build
SRC:=src
INCLUDE+=include
INCLUDE:=$(patsubst %,-I%,$(INCLUDE))
SOURCES:=$(wildcard $(SRC)/*.c)
OBJECTS:=$(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SOURCES))

CFLAGS+=-Wall
CFLAGS+=-std=c11
CFLAGS+=-O0
CFLAGS+=-g
CFLAGS+=$(INCLUDE)

LIBS+=-lSOIL
LIBS+=-lm
LIBS+=-lglfw
LIBS+=-pthread
LIBS+=-lGLEW
LIBS+=-lGLU
LIBS+=-lGL
LIBS+=-lrt
LIBS+=-lXrandr
LIBS+=-lXxf86vm
LIBS+=-lXi
LIBS+=-lXinerama
LIBS+=-lX11
LIBS+=-lexpat

TARGET=main

all: $(TARGET)
	@echo "Compilation successfull"

$(TARGET): $(OBJECTS)
	@echo "Compiling: $@"
	@$(CC) $(CFLAGS) $(LIBS) $^ $(MODOBJ) -o $@

$(OBJECTS): | $(BUILD)

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC)/%.c
	@echo "Compiling: $@"
	@$(CC) -c $(CFLAGS) $(LIBS) -o $@ $<

clean:
	@rm -rfv main $(BUILD)

.PHONY: all clean
