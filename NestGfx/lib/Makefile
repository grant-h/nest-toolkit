# Nest Library Makefile
#  by Grant Hernandez

# toolchain
CC=${PREFIX}gcc
CPP=${PREFIX}g++
LD=${PREFIX}g++
STRIP=${PREFIX}strip

# source files
SOURCES := Gfx.cpp util.cpp Timer.cpp

# linking/compiling flags
LDFLAGS=-g
LIBS=-lrt -lm #-lnest

CFLAGS=-ggdb -Wall -Wextra
CFLAGS+= $(TUNE_CFLAGS)


# build directory outputs
BUILD_DIR=build
OBJECTS=$(addsuffix .o, $(addprefix $(BUILD_DIR)/, $(SOURCES)))

# library
LIBNAME=libnest.a

##################################

.PHONY: all clean
all: $(LIBNAME)

# linking stage. all objects required to be compiled
$(LIBNAME) : $(BUILD_DIR) $(OBJECTS) 
	ar cvr $(LIBNAME) $(OBJECTS)

# make sure our build directory is created
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# compiling C++
$(BUILD_DIR)/%.cpp.o : %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

# compiling C
$(BUILD_DIR)/%.c.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

# watch out!
clean: 
	-rm -f $(OBJECTS) $(LIBNAME)
	-rmdir $(BUILD_DIR)/
