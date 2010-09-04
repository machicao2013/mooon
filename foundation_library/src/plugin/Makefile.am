EXTRA_DIST = Makefile.am

# 如果LOG4CXX没有安装，则退出来, 请注意下面的error必须顶格写，这里不能使用warning
if LOG4CXX_INSTALLED_USR
SUBDIRS = plugin_tinyxml plugin_log4cxx
else
if LOG4CXX_INSTALLED_HOME
SUBDIRS = plugin_tinyxml plugin_log4cxx
else
if LOG4CXX_INSTALLED_ENV
SUBDIRS = plugin_tinyxml plugin_log4cxx
else
SUBDIRS = plugin_tinyxml
endif
endif
endif

uninstall-hook:
	rm -fr $(prefix)/include/plugin
