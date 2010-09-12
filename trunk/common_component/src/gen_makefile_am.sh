#!/bin/sh

basedir=.

gen_makefile_am()
{
	old=$sub/Makefile.am.in
	new=$sub/Makefile.am

	if test ! -f $old; then
		return;
	fi

	if test -f $new; then
		rm $new;
	fi
	
	# Format from DOS to unix
	tr -d "\r" < $old > $old.tmp
	mv $old.tmp $old

	# Get all cpp files in $sub directory
	cd $sub
	cpp_files=`ls *.cpp 2>/dev/null`
	cd - > /dev/null 2>&1
	
	while read line
	do
		flag=`echo $line|grep -E "_SOURCES |_SOURCES="`
		if test $? -eq 0; then
			line="$line $cpp_files"
		fi
		echo $line >> $new
	done < $old
	
}

rec_subdir()
{
	if test $# -ne 1; then
		echo "Parameter error in rec_subdir"
		exit
	fi

	subdirs=`find $1 -type d`
	for sub in $subdirs
	do
		# Skip the current and parent directory
		if test $sub = "." -o $sub = ".."; then 
			continue;
		fi

		gen_makefile_am $sub
	done
}

rec_subdir $basedir
