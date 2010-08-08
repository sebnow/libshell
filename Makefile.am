ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = autogen.sh

AM_CFLAGS = -I@srcdir@/source -Wall -Wextra -Werror -g

# Dummy library so the `all` target works.
lib_LTLIBRARIES = libshell.la
libshell_la_SOURCES = /dev/null
libshell_la_LDFLAGS = -version-info @SH_CURRENT@:@SH_REVISION@:@SH_AGE@
include_HEADERS = source/shell.h

# Dummy tests, just so the `check` and `prove` targets work.
TESTS = /bin/true

PROVE ?= prove
PROVEFLAGS ?= -f
prove:
	$(PROVE) --exec '' "echo -e '1..1\nok 1'"

.PHONY: prove

