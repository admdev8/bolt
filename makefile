CC=gcc
GCOV_FLAGS=-fprofile-arcs -ftest-coverage

ifeq ($(BUILD),debug)
bsuffix=debug
CPPFLAGS_ADD=$(DEBUG_FLAGS)
else 
	ifeq ($(BUILD),gcov)
	bsuffix=debug
	CPPFLAGS_ADD=$(DEBUG_FLAGS) $(GCOV_FLAGS)
	else
	bsuffix=release
	CPPFLAGS_ADD=-O3
	endif
endif

OCTOTHORPE=../octothorpe
OCTOTHORPE_LIBRARY_PATH=$(OCTOTHORPE)/$(MSYSTEM)_$(bsuffix)
OCTOTHORPE_LIBRARY=$(OCTOTHORPE_LIBRARY_PATH)/octothorpe.a
X86_DISASM=../x86_disasm
X86_DISASM_LIBRARY_PATH=$(X86_DISASM)/$(MSYSTEM)_$(bsuffix)
X86_DISASM_LIBRARY=$(X86_DISASM_LIBRARY_PATH)/x86_disasm.a
PORG=../porg
PORG_LIBRARY=$(PORG)/$(MSYSTEM)_$(bsuffix)/porg.a
CPPFLAGS=-D_DEBUG -I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG) $(CPPFLAGS_ADD)
CFLAGS=-c -Wall -g -std=c11
#CFLAGS=-c -Wall -g -std=gnu99
SOURCES=CONTEXT_utils.c disas_utils.c memorycache.c X86_register_helpers.c PE.c X86_emu.c \
	bolt_stuff.c X86_emu_tests.c PE_imports.c
DEPFILES=$(SOURCES:.c=.d)
OUTDIR=$(MSYSTEM)_$(bsuffix)
OBJECTS=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.o))
LIBRARY=$(OUTDIR)\bolt.a
TEST_SOURCES=PE_test.c tests.c PE_dump_imports.c PE_imports_test.c PE_add_import.c
TEST_EXECS=$(addprefix $(OUTDIR)/,$(TEST_SOURCES:.c=.exe))

all: $(OUTDIR) $(LIBRARY)($(OBJECTS)) $(TEST_EXECS)
	$(OUTDIR)\tests.exe

$(OUTDIR):
	mkdir $(OUTDIR)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(DEPFILES)
	$(RM) $(LIBRARY)
	$(RM) $(TEST_EXECS)

-include $(OBJECTS:.o=.d)

$(OUTDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@

# for tests:

%.exe: %.o $(LIBRARY)
	$(CC) $< $(CPPFLAGS_ADD) $(LIBRARY) $(OCTOTHORPE_LIBRARY_PATH)/lisp.o $(OCTOTHORPE_LIBRARY) $(PORG_LIBRARY) $(X86_DISASM_LIBRARY) -lpsapi -ldbghelp -limagehlp -o $@
