XX	= g++
CFLAGS	= -Wall -O -ggdb -pipe -fPIC
INCLUDE	= -I.
INCLUDE	+= -I../../../newsfeed-lib/
SOURCES	= $(wildcard *.cpp)
OBJS	= $(patsubst %.cpp,%.o,$(SOURCES))
TARGET	= $(OBJS)

all: $(TARGET)
%.o: %.cpp
	$(XX) $(CFLAGS) $(INCLUDE) -c $< -o $@ $(LIBS)
clean:
	rm -fr *.o $(TARGET)
c: clean
r: clean all
