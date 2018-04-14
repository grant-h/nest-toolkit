# Nest Config Makefile
#  by Grant Hernandez
HARDFP=no

ifeq ($(HARDFP), yes)
  PREFIX=/home/digital/rsrch/nest/toolchain/crosstool-ng-1.18.0/targets/arm-cortex_a8-linux-gnueabi/bin/arm-cortex_a8-linux-gnueabi-
  HF_LIB_PATH=LD_LIBRARY_PATH=/opt/hardfloat/lib
else
  PREFIX=/home/digital/rsrch/arm-2010q1/bin/arm-none-linux-gnueabi-
  HF_LIB_PATH=
endif

# allow lower Makefiles to know the toolchain prefix
export PREFIX

# toolchain
CC=${PREFIX}gcc
CPP=${PREFIX}g++
LD=${PREFIX}g++
STRIP=${PREFIX}strip

# required library path
LIBPATH=$(CURDIR)/../lib/

# source files
SOURCES := main.cpp

PIDFILE=/tmp/killme.pid # hardcoded in main.cpp
INCLUDE=$(LIBPATH)

# linking/compiling flags
LDFLAGS=-g -L$(LIBPATH)
LIBS=-lrt -lm -lnest

ifeq ($(HARDFP), yes)
  # -O{X} is broken when combined with neon
  TUNE_CFLAGS = -mtune=cortex-a8 -march=armv7-a -O1
  TUNE_CFLAGS += -mfpu=neon -mfloat-abi=hard -ftree-vectorize -funsafe-math-optimizations
else
  TUNE_CFLAGS = -mtune=cortex-a8 -march=armv7-a -O2
  #TUNE_CFLAGS += -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -funsafe-math-optimizations
endif

# export common CFLAGS
export TUNE_CFLAGS

CFLAGS = -iquote$(CURDIR)/include/ -I$(INCLUDE) -ggdb -Wall -Wextra
CFLAGS += $(TUNE_CFLAGS) 

# build directory outputs
BUILD_DIR=build
OBJECTS=$(addsuffix .o, $(addprefix $(BUILD_DIR)/, $(SOURCES)))

# executable
EXENAME=nest-config
EXECUTABLE=build/${EXENAME}

include ../tools.mak

##################################

.PHONY: all clean run debug enter_test leave_test shutdown
all: $(EXECUTABLE)

# linking stage. all objects required to be compiled
$(EXECUTABLE) : $(BUILD_DIR) libnest $(OBJECTS) 
	$(LD) $(LDFLAGS) $(OBJECTS) -o $(EXECUTABLE) $(LIBS)

	@echo "File size before strip: `stat -c %s ${EXECUTABLE}`"
	@-${STRIP} -s ${EXECUTABLE}
	@echo "Final size: `stat -c %s ${EXECUTABLE}`"

# make sure our build directory is created
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# compiling C++
$(BUILD_DIR)/%.cpp.o : %.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

# compiling C
$(BUILD_DIR)/%.c.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

# build our libraries
libnest :
	@make -C $(LIBPATH)

# BE CAREFUL!
clean: 
	-make -C $(LIBPATH) clean
	-rm -f $(OBJECTS) $(EXECUTABLE)
	-rmdir $(BUILD_DIR)/

# make sure everything is built first
run : all
	@echo "Transferring and running ${EXENAME}"
	@-${CMD} 'kill -INT `cat ${PIDFILE}` > /dev/null 2>&1'
	${SCP} -P2222 "${EXECUTABLE}" ${NESTUSER}@"${NESTIP}":"${EXENAME}"
	${CMD} "${HF_LIB_PATH} ./${EXENAME}"

debug : all
	@echo "Transferring and running ${EXENAME} (DEBUG)"
	@-${CMD} 'kill -9 `cat ${PIDFILE}` > /dev/null 2>&1'
	${SCP} -P2222 "${EXECUTABLE}" ${NESTUSER}@"${NESTIP}":"${EXENAME}"
	# debug prevents forking to the background
	${CMD} "${HF_LIB_PATH} ./${EXENAME} debug"

enter_test : 
	${TESTMODE} enter

leave_test : 
	${TESTMODE} leave

shutdown : 
	${CMD} "/sbin/poweroff"
