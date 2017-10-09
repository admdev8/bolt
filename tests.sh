#!/bin/bash

./X86_emu_test.exe

cp PE_remove_import_test.exe tmp.exe
./PE_remove_import.exe tmp.exe kernel32.dll GetTickCount 1234 > /dev/null
echo must be 1234:
./tmp.exe

rm tmp.exe
