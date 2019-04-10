CC         = gcc
XX         = g++
CFLAGS     = -Wall -fPIC -ggdb 
CXXFLAGS   = -Wall -fPIC -ggdb -Wno-non-virtual-dtor
LIBS       = -lmemcached
INCLUDES   = -I./ -I../ -I../../../storage-server/src/
CSOURCES   = $(wildcard *.c)
CXXSOURCES = $(wildcard *.cpp)
COBJS      = $(patsubst %.c,%.o, $(CSOURCES))
CXXOBJS    = $(patsubst %.cpp,%.o, $(CXXSOURCES))
SONAME	   = libpoutbox.so.1
LINKNAME   = libpoutbox.so
TARGET     = libpoutbox.so.1.1.0

all: $(TARGET)

$(TARGET): $(COBJS) $(CXXOBJS)
	$(XX) -shared -Wl,-soname,$(SONAME) -o $(TARGET) $(COBJS) $(CXXOBJS) $(LIBS)
	/sbin/ldconfig -n .
	ln -sf $(SONAME) $(LINKNAME)
	cp -f $(SONAME) ../../bin/
	rm *.o
#	ar -rs libstat.a $(COBJS) $(CXXOBJS)

%.o: %.cpp
	$(XX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf *.o */*.o $(TARGET)
	rm -rf $(SONAME) $(LINKNAME) $(TARGET)
	rm -rf ../../bin/$(SONAME)
	
c: clean

r: clean all
