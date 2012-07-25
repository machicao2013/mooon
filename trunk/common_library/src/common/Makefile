libmcommon_so_LDADD = $(LDADD)
libmcommon_OBJECTS=../util/*.o ../sys/*.o ../net/*.o

all: libmcommon.a

libmcommon.a:
	$(AR) $(ARFLAGS) libmcommon.a $(libmcommon_OBJECTS)
	$(RANLIB) libmcommon.a
	$(CXXLINK) $(libmcommon_OBJECTS) $(libmcommon_so_LDADD) $(LIBS)

clean:
	rm -f libmcommon.a libmcommon.so

install:
	cp libmcommon.a $(prefix)/lib
	cp libmcommon..so $(prefix)/lib

uninstall:
	rm -f $(prefix)/lib/libmcommon.a
	rm -f $(prefix)/lib/libmcommon.so

