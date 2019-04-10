XX	= g++
CFLAGS	= -Wall -pipe -D_GNU_SOURCE -D_DEBUG -fPIC -shared -export-dynamic -ggdb
LIB_PATH = ../lib/
LIBS	= -linifile -L$(LIB_PATH)/ini-file-1.1.1 -L/usr/local/berkeley-db/lib -ldb
INCLUDE	= -I. -I$(LIB_PATH)/ini-file-1.1.1/ -I./utils/
TARGET	= libtimestamp_serv.so
VERSION = libtimestamp_serv.so.1.0.0
SOURCES	= $(wildcard *.cpp)
OBJS	= $(patsubst %.cpp,%.o,$(SOURCES))
SUBOBJS	= ./utils/*.o ./uvalue/*.o

all: $(VERSION)

$(VERSION): $(OBJS) $(SUBOBJS)
	$(XX) -Wl,-soname,libtimestamp_serv.so.1.0 $(CFLAGS) $(OBJS) $(SUBOBJS) -o $@ $(LIBS)
	@/sbin/ldconfig -n -v .
	ln -sf $(VERSION) $(TARGET)
	mv *.so* ../bin/

$(OBJS): %.o: %.cpp %.h Makefile
	$(XX) $(CFLAGS) $(INCLUDE) -c $< -o $@ $(LIBS)

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
	rm -rf *.o $(TARGET) $(SUBOBJS) *.so*
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


