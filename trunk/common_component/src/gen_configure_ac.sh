#!/bin/sh

replace_autoconf_version()
{
	autoconf_version=`autoconf --version|head -n1|cut -d' ' -f4`	
	sed 's/AUTOCONF_VERSION/'$autoconf_version'/' configure.ac.in > configure.ac
}

check_make_rules()
{
	bit=`getconf LONG_BIT`
	if test $bit -eq 64; then
		sed 's/^MY_CXXFLAGS/#MY_CXXFLAGS/' Make.rules.in > Make.rules
	else
		cp Make.rules.in Make.rules
	fi
}

# Format from dos to unix
d2x()
{
    for file in $*
    do
            src_file=$file
            tmp_file=$file.tmp

	    if test -d $src_file; then
            continue
	    fi

            tr -d "\r" < $src_file > $tmp_file
            if test $? -eq 0; then 
                    mv $tmp_file $src_file
                    echo "Convert $src_file from the format of DOS to UNIX OK."
            fi      
    done
}


d2x ltmain.sh
d2x configure.ac.in
d2x Makefile.am
d2x Make.rules.in

check_make_rules
replace_autoconf_version

aclocal
if test $? -ne 0; then
	echo "aclocal ERROR"
	exit
fi
autoconf
if test $? -ne 0; then
	echo "autoconf ERROR"
	exit
fi
autoheader
if test $? -ne 0; then
	echo "autoheader ERROR"
	exit
fi
automake -a
if test $? -ne 0; then
	echo "automake -a ERROR"
	exit
fi

chmod +x *.sh
chmod +x configure
