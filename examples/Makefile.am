## Process this file with automake to produce Makefile.in

noinst_PROGRAMS = \
	network \
	pdc1 \
	pdc2 \
	rawout \
	wss

if WINDOWS
scripts =
else
scripts = \
	pdc2-test1.sh
endif

check_SCRIPTS = \
	$(scripts)

TESTS = \
	$(check_SCRIPTS)

EXTRA_DIST = \
	$(check_SCRIPTS)

AM_CPPFLAGS = -I$(top_srcdir)/src

LDADD = $(top_builddir)/src/libzvbi.la \
	$(PTHREAD_LIB) -lm $(PNG_LIB)
