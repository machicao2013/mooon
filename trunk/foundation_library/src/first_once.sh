#!/bin/sh
# 每次从SVN取下来时，需要运行该脚本，之后再未从SVN取出文件之前不需要运行它
# author: eyjian@qq.com or eyjian@gmail.com

# basedir为源代码存放根目录
basedir=.
cd $basedir


############################
# 删除所有的.svn目录和文件
############################
find $basedir -name .svn | xargs rm -fr


############################
# 下面为生成Makefile.am文件
############################

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
	cpp_files=`ls *|awk '/.cpp|.cc/{printf("%s ", $0)}'`
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



############################
# 下面为生成configure.ac文件
############################

# 填写configure.ac中的autoconf版本号
replace_autoconf_version()
{
	autoconf_version=`autoconf --version|head -n1|cut -d' ' -f4`
	sed 's/AUTOCONF_VERSION/'$autoconf_version'/' configure.ac.in > configure.ac
}

# 处理Make.rules文件
check_make_rules()
{
	bit=`getconf LONG_BIT`
	if test $bit -eq 64; then
		sed 's/^MY_CXXFLAGS/#MY_CXXFLAGS/' Make.rules.in > Make.rules
	else
		cp Make.rules.in Make.rules
	fi
}

# 将文件格式从DOS转换成UNIX
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

# 需要DOS格式转换的文件
d2x ltmain.sh
d2x configure.ac.in
d2x Makefile.am
d2x Make.rules.in

check_make_rules
replace_autoconf_version


##########################################
# 下面为生成configure文件和Makefile.in文件
##########################################

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


#################################################
# 接下来就可以开始执行configure生成Makefile文件了
#################################################
