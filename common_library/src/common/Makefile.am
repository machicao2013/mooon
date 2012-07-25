include $(top_srcdir)/Make.rules
AUTOMAKE_OPTIONS= foreign

libmcommon_OBJECTS=../util/*.o ../sys/*.o ../net/*.o
AM_LDFLAGS  += -fPIC -shared 
AM_CXXFLAGS += -fPIC

all: libmcommon.a libmcommon.so

libmcommon.a:
	$(AR) $(ARFLAGS) $@ $(libmcommon_OBJECTS) $(AM_CXXFLAGS)
	ranlib $@

libmcommon.so:
	 $(CXX) -o $@ $(libmcommon_OBJECTS) $(AM_LDFLAGS) -pthread

clean:
	rm -f libmcommon.a libmcommon.so

install:
	cp libmcommon.a $(prefix)/lib
	cp libmcommon..so $(prefix)/lib

uninstall:
	rm -f $(prefix)/lib/libmcommon.a
	rm -f $(prefix)/lib/libmcommon.so

