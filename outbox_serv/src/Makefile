CC			= gcc
XX			= g++
INCLUDES	= -I./ -I./outbox/ -I./poutbox/ -I../../storage-serv/src/
LIBS		= -L/usr/local/lib -lmemcached -L./outbox/ -loutbox -L./poutbox/ -lpoutbox
FLAGS		= -Wall -Wno-non-virtual-dtor -ggdb -fPIC -shared
CFLAGS		= -Wall -fPIC -ggdb
CSOURCE		= $(wildcard *.c)
COBJS		= $(patsubst %.c, %.o, $(CSOURCE))
SOURCE		= $(wildcard *.cpp)
OBJS		= $(patsubst %.cpp, %.o, $(SOURCE))
TARGET		= ../bin/outbox_server.so

all: $(TARGET)

$(TARGET): $(OBJS) $(COBJS)
	$(XX) -o $(TARGET) $(OBJS) $(COBJS) $(FLAGS) $(LIBS)
	rm *.o

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cpp
	$(XX) $(FLAGS) $(INCLUDES) -c $< -o $@


clean:
	rm -rf *.o $(TARGET)
	@make_clean_sub_dir()\
	{\
			for dir in `ls`; do \
					if [ -d $$dir ]; then \
							cd $$dir;\
							make_clean_sub_dir;\
							if [ -f makefile -o -f Makefile ]; then\
									make clean;\
							fi;\
							cd ..;\
					fi;\
			done;\
	};\
	make_clean_sub_dir

rebuild: clean all

c: clean

r: rebuild
