OCTOTHORPE=C:\Users\Administrator\Projects\octothorpe
OCTOTHORPE_LIB=$(OCTOTHORPE)\octothorped.lib
DISASM=C:\Users\Administrator\Projects\x86_disasm
DISASM_LIB=$(DISASM)\x86_disasmd.lib

CL_OPTIONS=/c /D_DEBUG /Zi /I$(OCTOTHORPE) /I$(DISASM)

X86_register_helpers.obj: X86_register_helpers.c X86_register_helpers.h
	cl.exe X86_register_helpers.c $(CL_OPTIONS)

memorycache.obj: memorycache.c memorycache.h
	cl.exe memorycache.c $(CL_OPTIONS)

CONTEXT_utils.obj: CONTEXT_utils.c CONTEXT_utils.h
	cl.exe CONTEXT_utils.c $(CL_OPTIONS)

bolt.lib: X86_register_helpers.obj memorycache.obj CONTEXT_utils.obj
	lib.exe X86_register_helpers.obj memorycache.obj CONTEXT_utils.obj /OUT:bolt.lib

all: bolt.lib

clean:
	del *.lib *.exe *.obj *.asm
