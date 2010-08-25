ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = autogen.sh

RAGEL ?= ragel
AM_CFLAGS = -I@srcdir@/source -Wall -Wextra

if ENABLE_DEBUG
	AM_CFLAGS += -Werror -DSH_DEBUG -g
# Some extra warnings
	AM_CFLAGS += -Wpointer-arith -Wstrict-prototypes -Wfloat-equal -Wshadow \
		-Wwrite-strings -Wbad-function-cast -Wcast-align -Wcast-qual \
		-Wdeclaration-after-statement -Wuninitialized
endif

#
# Library
#
lib_LTLIBRARIES = libshell.la
libshell_la_SOURCES = source/scanner.c
# Prevent "created with both libtool and without" errors for tests. This
# doesn't actually do anything.
libshell_la_CFLAGS = $(AM_CFLAGS)
if HAVE_VISIBILITY
libshell_la_CFLAGS += $(CFLAG_VISIBILITY)
endif
libshell_la_LDFLAGS = -version-info @SH_CURRENT@:@SH_REVISION@:@SH_AGE@
include_HEADERS = source/shell.h

#
# Unit tests
#
check_PROGRAMS = test/test_scanner.t
test_test_scanner_t_SOURCES = test/test_scanner.c source/scanner.c
test_test_scanner_t_LDFLAGS = -ltap

TESTS = $(check_PROGRAMS)

PROVE ?= prove
PROVEFLAGS ?= -f
prove: $(check_PROGRAMS)
	$(PROVE) --exec '' $(PROVEFLAGS) $(check_PROGRAMS)

%.c: %.rl
	$(AM_V_GEN)$(RAGEL) -o $@ -C $(RAGELFLAGS) $^

.PHONY: prove

