include $(top_srcdir)/Make.rules
AUTOMAKE_OPTIONS= foreign

lib_LIBRARIES = libmcommon.a
module_PROGRAMS = libmcommon.so

libmcommon_OBJECTS=../util/*.o ../sys/*.o ../net/*.o
AM_LDFLAGS  += -fPIC -shared 
AM_CXXFLAGS += -fPIC

all: $(lib_LIBRARIES) $(module_PROGRAMS)

$(lib_LIBRARIES):
	$(AR) $(ARFLAGS) $@ $(libmcommon_OBJECTS)
	ranlib $@

$(module_PROGRAMS):
	 $(CXX) -o $@ $(libmcommon_OBJECTS) $(AM_LDFLAGS) -pthread

clean:
	rm -f $(lib_LIBRARIES) $(module_PROGRAMS)

install:
	cp $(lib_LIBRARIES) $(prefix)/lib
	cp $(module_PROGRAMS) $(prefix)/lib

uninstall:
	rm -f $(prefix)/lib/$(lib_LIBRARIES)
	rm -f $(prefix)/lib/$(module_PROGRAMS)
