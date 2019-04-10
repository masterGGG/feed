CC         = gcc
XX         = g++
CFLAGS     = -Wall -fPIC -ggdb
CXXFLAGS   = -Wall -fPIC -ggdb -Wno-non-virtual-dtor
LIBS       = `mysql_config --libs` -ldl -lmemcached
INCLUDES   = -I./
CSOURCES   = $(wildcard *.c)
CXXSOURCES = $(wildcard *.cpp */*.cpp)
COBJS      = $(patsubst %.c,%.o, $(CSOURCES))
CXXOBJS    = $(patsubst %.cpp,%.o, $(CXXSOURCES))
BASENAME   = storage_server
LINKNAME   = lib$(BASENAME).so
SONAME	   = lib$(BASENAME).so.1
TARGET     = lib$(BASENAME).so.1.0.0
BUILD_DIR  = ../build
RUNLIBS_DIR= $(BUILD_DIR)/libs/
OBJS_DIR   = $(BUILD_DIR)/objs/
CFG_DIR    = $(BUILD_DIR)/conf/
CFG_FILES  = `ls ../conf/*.conf`
FRM_FILES  = ../newbench  ../storagectl 
FRM_LIBS   = ../libnetioserver.so.2  ../libringqueue.so.1

all: tags $(TARGET)

$(TARGET): $(COBJS) $(CXXOBJS)
	$(XX) -shared -Wl,-soname,$(SONAME) -o $(TARGET) $(COBJS) $(CXXOBJS) $(LIBS)
	-ln -sv $(TARGET) $(SONAME)
	-ln -sv $(TARGET) $(LINKNAME)
	@echo create \<BUILD DIRS\> if needed
#	-mkdir $(BUILD_DIR)
#	-mkdir $(RUNLIBS_DIR)
#	-mkdir $(OBJS_DIR)
#	-mkdir $(CFG_DIR)
#	-mkdir $(BUILD_DIR)/log
#	-mkdir $(BUILD_DIR)/stat
	@echo clean build process generated stuff ...
	-mv -fv $(LINKNAME) $(SONAME) $(TARGET) $(RUNLIBS_DIR)
#	-cp -RPuv $(FRM_FILES) $(BUILD_DIR)
#	-cp -RPuv $(FRM_LIBS) $(RUNLIBS_DIR)
#	-cp -RPuv $(CFG_FILES) $(BUILD_DIR)/conf
	
	@echo  build process successfully ...
	@echo

%.o: %.cpp
	$(XX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	-mv -fv *.o */*.o $(OBJS_DIR)
	-rm -fv $(LINKNAME) $(SONAME) $(TARGET) 
	-rm -rfv $(RUNLIBS_DIR)/lib$(BASENAME).so.*

c: clean

r: clean all
	
tags:
	-ctags -R

.PHONY : all c r tags cmd

cmd:
	@echo CXXSRC: $(CXXSOURCES)
	-alias cp=cp ; cp -Pfv $(BUILD_DIR)/$(LINKNAME)* libs/
#	ar -rs libstat.a $(COBJS) $(CXXOBJS)
#	/sbin/ldconfig -n .
