EXTRA_DIST =
SUBDIRS = shell util sys net plugin

all:
    cd util&&make&&cd ../sys&&make&&cd ../net&&make
    ar cr libmcl.a util/*.o sys/*.o net/*.a

install-data-hook:
    cp libmcl.a $(prefix)/lib
    
uninstall-hook:
    rm -f $(prefix)/lib/libmcl.a

clean:
    rm -fr libmcl.a
