MAKE=make --no-print-directory
Q=@

BUILD:=build
SRC:=src
INCLUDE+=include
INCLUDE:=$(patsubst %,-I%,$(INCLUDE))
SOURCES:=$(wildcard $(SRC)/*.c)
OBJECTS:=$(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SOURCES))
DEPENDS:=$(OBJECTS:.o=.d)

#COMMON+=-fsanitize=thread

CFLAGS+=$(COMMON)
CFLAGS+=-Wall
CFLAGS+=-Wextra
CFLAGS+=-Wpedantic
CFLAGS+=-std=c11
CFLAGS+=-MD
CFLAGS+=-O0
CFLAGS+=-g
CFLAGS+=$(INCLUDE)

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
LIBS+=-pthread
LIBS+=-D_REENTRANT -lpulse-simple -lpulse -lfftw3

LDFLAGS+=$(COMMON)
LDFLAGS+=$(LIBS)

TARGET=main

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo "  LD      $@"
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $^ $(MODOBJ) -o $@

$(OBJECTS): | $(BUILD)

$(BUILD):
	$(Q)mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC)/%.c
	@echo "  CC      $@"
	$(Q)$(CC) -c $(CFLAGS) -o $@ $<

clean:
	$(Q)rm -rfv main $(BUILD)

-include $(DEPENDS)
.PHONY: all clean
