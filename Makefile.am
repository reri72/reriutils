AUTOMAKE_OPTIONS = foreign

SUBDIRS = tests

ACLOCAL_AMFLAGS = -I m4

lib_LIBRARIES = libreriutils.a

libreriutils_a_SOURCES = \
                    socks.c \
                    sockUtils.c \
                    utiles.c \
                    readconf.c \
                    writelog.c \
                    sslUtils.c \
                    myutils.c

AM_CPPFLAGS = -I./include -I.

libreriutilsdir = $(include)
libreriutils_HEADERS = \
                    include/sockC.h \
                    include/socks.h \
                    include/sockUtils.h \
                    include/utiles.h \
                    include/readconf.h \
                    include/writelog.h \
                    myutils.h

noinst_libreriutils_ladir = $(topdir)

.PHONY: lib test

lib : libreriutils.a

test:
	@for dir in tests/*/ ; do \
		if [ -f "$$dir/Makefile" ]; then \
			echo "Running make in $$dir..."; \
			$(MAKE) -C $$dir || exit 1; \
		fi \
	done