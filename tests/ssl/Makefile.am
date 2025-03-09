bin_PROGRAMS = sslclient sslserver

AM_CPPFLAGS = -I$(top_srcdir)/include

sslclient_SOURCES = sslclient.c
sslserver_SOURCES = sslserver.c

LDADD = $(top_builddir)/libreriutils.a

LDFLAGS = -lssl -lcrypto
LIBS = 