CC=gcc
OCTOTHORPE=../octothorpe
OCTOTHORPE_LIBRARY_PATH=$(OCTOTHORPE)/$(MSYSTEM)_debug
OCTOTHORPE_LIBRARY=$(OCTOTHORPE_LIBRARY_PATH)/octothorped.a
X86_DISASM=../x86_disasm
X86_DISASM_LIBRARY=$(X86_DISASM)/x86_disasmd.a
PORG=../porg
PORG_LIBRARY=$(PORG)/$(MSYSTEM)_debug/porgd.a
CPPFLAGS=-D_DEBUG -I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG)
CFLAGS=-c -Wall -g -std=gnu99
SOURCES=CONTEXT_utils.c disas_utils.c memorycache.c X86_register_helpers.c PE.c X86_emu.c \
	bolt_stuff.c
DEPFILES=$(SOURCES:.c=.d)
OUTDIR=$(MSYSTEM)_debug
OBJECTS=$(addprefix $(OUTDIR)/,$(SOURCES:.c=.o))
LIBRARY=$(OUTDIR)\boltd.a
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
#	$(CC) $< $(LIBRARY) $(OCTOTHORPE_LIBRARY) $(OCTOTHORPE_LIBRARY_PATH)/lisp.o $(PORG_LIBRARY) -lpsapi -limagehlp -o $@
	$(CC) $< $(LIBRARY) $(OCTOTHORPE_LIBRARY) $(OCTOTHORPE_LIBRARY_PATH)/lisp.o $(PORG_LIBRARY) -lpsapi -ldbghelp -limagehlp -o $@
