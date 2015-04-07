TESTS = run-bolt_tests.sh

check_PROGRAMS = X86_emu_test

X86_emu_test_SOURCE = X86_emu_test.c
X86_emu_test_LDADD = $(top_builddir)/lib/libbolt.a @PORG_LIBS@ @X86_DISASM_LIBS@ @OCTOTHORPE_LIBS@ -ldbghelp -limagehlp

AM_CPPFLAGS = -I$(top_srcdir)/include

EXTRA_DIST = run-bolt_tests.sh
