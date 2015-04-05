TESTS = run-bolt_tests.sh

check_PROGRAMS = X86_emu_test

X86_emu_test_SOURCE = X86_emu_test.c
X86_emu_test_LDADD = $(top_builddir)/lib/libbolt.a

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = run-bolt_tests.sh
