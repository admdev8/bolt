OCTOTHORPE=../octothorpe/
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a
X86_DISASM=../x86_disasm/
X86_DISASM_LIBRARY=$(X86_DISASM)x86_disasmd.a
PORG=../porg/
PORG_LIBRARY=$(PORG)porgd.a
CPPFLAGS=-D_DEBUG -I$(OCTOTHORPE) -I$(X86_DISASM) -I$(PORG)
CFLAGS=-c -Wall -g -std=gnu99
SOURCES=CONTEXT_utils.c disas_utils.c memorycache.c X86_register_helpers.c PE.c X86_emu.c \
	bolt_stuff.c
OBJECTS=$(SOURCES:.c=.o)
DEPFILES=$(SOURCES:.c=.d)
LIBRARY=boltd.a
TEST_SOURCES=PE_test.c
TEST_EXECS=$(TEST_SOURCES:.c=.exe)

all: $(LIBRARY)($(OBJECTS)) $(TEST_EXECS)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(DEPFILES)
	$(RM) $(LIBRARY)
	$(RM) $(TEST_EXECS)

-include $(OBJECTS:.o=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@

# for tests:

%.exe: %.o $(LIBRARY)
	$(CC) $< $(LIBRARY) $(OCTOTHORPE)/lisp.o $(OCTOTHORPE_LIBRARY) $(PORG_LIBRARY) -lpsapi -limagehlp -o $@
