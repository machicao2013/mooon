libmcommon_so_LDADD = $(LDADD)
libmcommon_OBJECTS=../util/*.o ../sys/*.o ../net/*.o

all: libmcommon.a libmcommon.so

libmcommon.a:
	ar cru $@ $(libmcommon_OBJECTS)
	ranlib $@

libmcommon.so:
	g++ -g -o $@ $(libmcommon_OBJECTS) -pthread -fPIC -shared

clean:
	rm -f libmcommon.a libmcommon.so

install:
	cp libmcommon.a $(prefix)/lib
	cp libmcommon..so $(prefix)/lib

uninstall:
	rm -f $(prefix)/lib/libmcommon.a
	rm -f $(prefix)/lib/libmcommon.so

