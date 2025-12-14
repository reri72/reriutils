bin_PROGRAMS = sslclient sslserver sslcrypto

AM_CPPFLAGS = -I$(top_srcdir)/include

sslclient_SOURCES = sslclient.c
sslserver_SOURCES = sslserver.c
sslcrypto_SOURCES = sslcrypto.c

LDADD = $(top_builddir)/libreriutils.a

AM_LDFLAGS = -lssl -lcrypto
LIBS = 