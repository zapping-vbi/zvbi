## Process this file with automake to produce Makefile.in

compile_tests = \
	ctest$(EXEEXT) \
	cpptest$(EXEEXT)

cpptest_SOURCES = cpptest.cc

if HAVE_GCC_C89_SUPPORT
compile_tests += ctest-c89$(EXEEXT) ctest-gnu89$(EXEEXT)

ctest_c89_SOURCES = ctest.c
ctest_c89_CFLAGS = -pedantic-errors -std=c89

ctest_gnu89_SOURCES = ctest.c
ctest_gnu89_CFLAGS = -pedantic-errors -std=gnu89
endif

if HAVE_GCC_C94_SUPPORT
compile_tests += ctest-c94$(EXEEXT)

ctest_c94_SOURCES = ctest.c
ctest_c94_CFLAGS = -pedantic-errors -std=iso9899:199409
endif

if HAVE_GCC_C99_SUPPORT
compile_tests += ctest-c99$(EXEEXT) ctest-gnu99$(EXEEXT)

ctest_c99_SOURCES = ctest.c
ctest_c99_CFLAGS = -pedantic-errors -std=c99

ctest_gnu99_SOURCES = ctest.c
ctest_gnu99_CFLAGS = -pedantic-errors -std=gnu99
endif

if HAVE_GXX_CXX98_SUPPORT
compile_tests += cpptest-cxx98$(EXEEXT) cpptest-gnuxx98$(EXEEXT)

cpptest_cxx98_SOURCES = cpptest.cc
cpptest_cxx98_CXXFLAGS = -pedantic-errors -std=c++98

cpptest_gnuxx98_SOURCES = cpptest.cc
cpptest_gnuxx98_CXXFLAGS = -pedantic-errors -std=gnu++98
endif

scripts =

# Rewrite scripts to execute in linux for windows
if LINUX_FOR_WINDOWS
scripts += \
	exoptest_windows \
	test-unicode_windows
endif

if !WINDOWS
scripts += \
	exoptest \
	test-unicode
endif

# Currently failing in windows, need to debug
if WINDOWS
only_linux =
else
only_linux = \
	test-pdc$(EXEEXT) # TODO: Fix timezone conversions
endif

TESTS = \
	$(compile_tests) \
	$(only_linux) \
	$(scripts) \
	test-dvb_demux$(EXEEXT) \
	test-dvb_mux$(EXEEXT) \
	test-hamm$(EXEEXT) \
	test-packet-830$(EXEEXT) \
	test-raw_decoder$(EXEEXT) \
	test-vps$(EXEEXT)

check_PROGRAMS = \
	$(compile_tests) \
	$(only_linux) \
	test-dvb_demux$(EXEEXT) \
	test-dvb_mux$(EXEEXT) \
	test-hamm$(EXEEXT) \
	test-packet-830$(EXEEXT) \
	test-raw_decoder$(EXEEXT) \
	test-vps$(EXEEXT)

check_SCRIPTS = \
	$(scripts)

test_dvb_demux_SOURCES = \
	test-dvb_demux.cc \
	test-common.cc test-common.h

test_dvb_mux_SOURCES = \
	test-dvb_mux.cc \
	test-common.cc test-common.h

test_hamm_SOURCES = test-hamm.cc

test_packet_830_SOURCES = \
	test-packet-830.cc \
	test-pdc.h \
	test-common.cc test-common.h

test_pdc_SOURCES = \
	test-pdc.cc test-pdc.h \
	test-common.cc test-common.h

test_raw_decoder_SOURCES = \
	test-raw_decoder.cc \
	test-common.cc test-common.h

test_vps_SOURCES = \
	test-vps.cc \
	test-pdc.h \
	test-common.cc test-common.h

# exoptest: explist

# test-unicode: unicode

if ENABLE_PROXY
proxy_programs = proxy-test$(EXEEXT)
else
proxy_programs =
endif

if HAVE_X
x_programs = caption$(EXEEXT) osc$(EXEEXT)
else
x_programs =
endif

noinst_PROGRAMS = \
	capture$(EXEEXT) \
	date$(EXEEXT) \
	decode$(EXEEXT) \
	explist$(EXEEXT) \
	export$(EXEEXT) \
	glyph$(EXEEXT) \
	sliced2pes$(EXEEXT) \
	test-vps$(EXEEXT) \
	ttxfilter$(EXEEXT) \
	unicode$(EXEEXT) \
	$(proxy_programs) \
	$(x_programs)

capture_SOURCES = \
	capture.c \
	sliced.c sliced.h

caption_SOURCES = \
	caption.c \
	sliced.c sliced.h

date_SOURCES = \
	date.c \
	sliced.c sliced.h

decode_SOURCES = \
	decode.c \
	sliced.c sliced.h

export_SOURCES = \
	export.c \
	sliced.c sliced.h

sliced2pes_SOURCES = \
	sliced2pes.c \
	sliced.c sliced.h

ttxfilter_SOURCES = \
	ttxfilter.c \
	sliced.c sliced.h

noinst_SCRIPTS = \
	uclist

EXTRA_DIST = \
	README \
	$(noinst_SCRIPTS) \
	$(check_SCRIPTS) \
	unicode-out-ref.txt

CLEANFILES = \
	unicode-out.txt

AM_CFLAGS = \
	-I$(top_srcdir) \
	$(X_CFLAGS) \
	-D_REENTRANT \
	-D_GNU_SOURCE

AM_CPPFLAGS = $(AM_CFLAGS)

# Libtool creates a wrapper script around the application until the
# library is installed, which is a major annoyance for debugging with
# gdb and valgrind. So we link these apps statically if possible.
if BUILD_STATIC_LIB
LDADD = $(top_builddir)/src/.libs/libzvbi.a
else
LDADD = $(top_builddir)/src/libzvbi.la
endif

LDADD += \
	$(LIBS) \
	$(X_LIBS)

unrename:
	for file in *.cc *.c *.h ; do \
	  case "$$file" in \
	  test-raw_decoder.cc|osc.c) continue ; ;; \
	  esac ; \
	  if grep -q -i vbi3_ $$file ; then \
	    sed 's/vbi3_/vbi_/g;s/VBI3_/VBI_/g' <$$file >tmp ; \
	    mv tmp $$file ; \
	  fi ; \
	done
