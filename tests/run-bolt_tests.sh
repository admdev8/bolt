#!/bin/bash

./X86_emu_test

if [[ $?c == 1 ]]; then
	exit $?
fi

cp ${srcdir}/PE_remove_import_test.exe.original PE_remove_import_test.exe
../src/PE_remove_import.exe PE_remove_import_test.exe kernel32.dll GetTickCount 1234
output=$(./PE_remove_import_test.exe)
rm PE_remove_import_test.exe

if [[ $output != "1234" ]]; then
	exit 1
fi

# 0 is tests passed OK
exit 0
