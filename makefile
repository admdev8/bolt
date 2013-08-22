CC=gcc

ifeq ($(BUILD),debug)
bsuffix=debug
CPPFLAGS_ADD=-D_DEBUG
else
bsuffix=release
CPPFLAGS_ADD=-O3
endif

OCTOTHORPE=../octothorpe
OCTOTHORPE_LIBRARY_PATH=$(OCTOTHORPE)/$(MSYSTEM)_$(bsuffix)
OCTOTHORPE_LIBRARY=$(OCTOTHORPE_LIBRARY_PATH)/octothorpe.a
X86_DISASM=../x86_disasm
X86_DISASM_LIBRARY=$(X86_DISASM)/x86_disasm.a
PORG=../porg
PORG_LIBRARY=$(PORG)/$(MSYSTEM)_debug/porg.a
CPPFLAGS=-D_DEBUG -I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG) $(CPPFLAGS_ADD)
CFLAGS=-c -Wall -g -std=c11
#CFLAGS=-c -Wall -g -std=gnu99
SOURCES=CONTEXT_utils.c disas_utils.c memorycache.c X86_register_helpers.c PE.c X86_emu.c \
	bolt_stuff.c
DEPFILES=$(SOURCES:.c=.d)
OUTDIR=$(MSYSTEM)_$(bsuffix)
OBJECTS=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.o))
LIBRARY=$(OUTDIR)\bolt.a
TEST_SOURCES=PE_test.c
TEST_EXECS=$(addprefix $(OUTDIR)/,$(TEST_SOURCES:.c=.exe))

all: $(OUTDIR) $(LIBRARY)($(OBJECTS)) $(TEST_EXECS)

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
	$(CC) $< $(LIBRARY) $(OCTOTHORPE_LIBRARY_PATH)/lisp.o $(OCTOTHORPE_LIBRARY) $(PORG_LIBRARY) -lpsapi -ldbghelp -limagehlp -o $@
