rem MSVC 2012
cl sample_DLL1.c /LD /link /DEF:sample_DLL1.def
cl sample_DLL2.c /LD /link /DEF:sample_DLL2.def
cl use_DLL.c
