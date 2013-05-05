OCTOTHORPE=../octothorpe/
OCTOTHORPE_LIBRARY=$(OCTOTHORPE)octothorped.a
X86_DISASM=../x86_disasm/
X86_DISASM_LIBRARY=$(X86_DISASM)x86_disasmd.a
CPPFLAGS=-D_DEBUG -I$(OCTOTHORPE) -I$(X86_DISASM)
CFLAGS=-c -Wall -g
SOURCES=CONTEXT_utils.c disas_utils.c memorycache.c X86_register_helpers.c
OBJECTS=$(SOURCES:.c=.o)
DEPFILES=$(SOURCES:.c=.d)
LIBRARY=boltd.a

all: $(LIBRARY)($(OBJECTS))

clean:
	$(RM) $(OBJECTS)
	$(RM) $(DEPFILES)
	$(RM) $(LIBRARY)

-include $(OBJECTS:.o=.d)

%.d: %.c
	$(CC) -MM $(CFLAGS) $(CPPFLAGS) $< > $@
