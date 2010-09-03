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
	
	# Get all cpp files in $sub directory
	cd $sub
	cpp_files=`ls *.cpp 2>/dev/null`
	cpp_files=`echo $cpp_files`
	cd - > /dev/null 2>&1
	
	# Remove carriage return, and append CPP after the line included _SOURCES
	awk -v files="$cpp_files" '{ gsub("\r",""); if (match($0,"_SOURCES")) printf("%s %s\n",$0,files); else printf("%s\n",$0); }' $old > $new
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
