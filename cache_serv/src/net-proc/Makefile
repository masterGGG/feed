XX       = g++
CFLAGS   = -Wall -ggdb -Wno-non-virtual-dtor
INCLUDES = -I../ -I../../../newsfeed-lib/ -I../../../newsfeed-lib/net-io-server-2.1.0/include/
SOURCES  = $(wildcard *.cpp)
OBJS     = $(patsubst %.cpp,%.o,$(SOURCES))
TARGET   = $(OBJS)

all: $(TARGET)
	
%.o: %.cpp
	$(XX) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf *.o $(TARGET)
	
c: clean

r: clean all
