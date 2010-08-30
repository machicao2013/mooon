EXTRA_DIST = Makefile.am

# 检测log4cxx安装目录，首先查看它是否安装在/usr/local目录下，其实检查是否在$(HOME)目录下，如果都不在则认为未安装
LOG4CXX_HOME=$(shell \
	if test -d /usr/local/log4cxx; then \
		echo /usr/local/log4cxx; \
	elif test -d $HOME/log4cxx; then \
		echo $HOME/log4cxx; \
	else \
		echo "NOT_INSTALLED"; \
	fi \
)

# 如果LOG4CXX没有安装，则退出来, 请注意下面的error必须顶格写，这里不能使用warning
ifeq (($LOG4CXX_HOME), "NOT_INSTALLED")
	SUBDIRS = plugin_tinyxml plugin_log4cxx
else
	SUBDIRS = plugin_tinyxml
endif
