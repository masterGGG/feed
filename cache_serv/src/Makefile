XX       = g++
CFLAGS   = -Wall -ggdb -Wno-non-virtual-dtor `mysql_config --cflags`
#LIBS     = -L../../newsfeed-lib -lnewsfeed -ldl -L../../newsfeed-lib/net-io-server-2.1.0/bin -lnetioserver `mysql_config --libs`
LIBS     = -lnewsfeed -ldl -lnetioserver `mysql_config --libs`
TARGET   = ../bin/cache_server
INCLUDES = -I../../newsfeed-lib/
SOURCES  = $(wildcard *.cpp)
OBJS     = $(patsubst %.cpp,%.o,$(SOURCES)) 
SUBOBJS  = ./net-proc/net_proc.o ./work-proc/work_proc.o

all: $(TARGET)

$(TARGET) : $(OBJS) $(SUBOBJS)
	dir=`pwd`;\
	cd ../../newsfeed-lib/;\
	make r;\
	cd $$dir;\
	$(XX) $(OBJS) $(SUBOBJS) -o $(TARGET) $(LIBS)
	rm -rf *.o
	
%.o: %.cpp
	$(XX) $(CFLAGS) $(INCLUDES) -c $< -o $@	
	
$(SUBOBJS):
	@make_all_sub_dir()\
	{\
		for dir in `ls`; do \
			if [ -d $$dir ]; then \
				cd $$dir;\
				make_all_sub_dir;\
				if [ -f makefile -o -f Makefile ]; then\
					make;\
				fi;\
				cd ..;\
			fi;\
		done;\
	};\
	make_all_sub_dir

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
