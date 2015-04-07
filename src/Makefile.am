boltdir = $(exec_prefix)
bolt_PROGRAMS = PE_add_imports PE_dump_imports PE_patcher PE_search_str_refs PE_filling_code_patterns_test

PE_dump_imports_SOURCE = PE_dump_imports.c
PE_dump_imports_LDADD = $(top_builddir)/lib/libbolt.a @PORG_LIBS@ @X86_DISASM_LIBS@ @OCTOTHORPE_LIBS@ -ldbghelp -limagehlp

PE_add_imports_SOURCE = PE_add_imports.c
PE_add_imports_LDADD = $(top_builddir)/lib/libbolt.a @PORG_LIBS@ @X86_DISASM_LIBS@ @OCTOTHORPE_LIBS@ -ldbghelp -limagehlp

PE_patcher_SOURCE = PE_patcher.c
PE_patcher_LDADD = $(top_builddir)/lib/libbolt.a @PORG_LIBS@ @X86_DISASM_LIBS@ @OCTOTHORPE_LIBS@ -ldbghelp -limagehlp

PE_search_str_refs_SOURCE = PE_search_str_refs.c
PE_search_str_refs_LDADD = $(top_builddir)/lib/libbolt.a @PORG_LIBS@ @X86_DISASM_LIBS@ @OCTOTHORPE_LIBS@ -ldbghelp -limagehlp

PE_filling_code_patterns_test_SOURCE = PE_filling_code_patterns_test.c
PE_filling_code_patterns_test_LDADD = $(top_builddir)/lib/libbolt.a @PORG_LIBS@ @X86_DISASM_LIBS@ @OCTOTHORPE_LIBS@ -ldbghelp -limagehlp

AM_CPPFLAGS = -I$(top_srcdir)/include
