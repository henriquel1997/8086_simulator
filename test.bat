@echo off
if not exist binary_out mkdir binary_out
cd listings

for %%i in (*.asm) do (
    nasm "%%~ni.asm" -o "../binary_out/%%~ni.out"
)

cd ..

clang main.c -D_CRT_SECURE_NO_WARNINGS -o simulator.exe

if not exist output mkdir output
cd binary_out

for %%i in (*.out) do (
    ..\simulator "%%~ni.out" > "../output/%%~ni.asm"
)

cd ..
cd output

for %%i in (*.asm) do (
    nasm "%%~ni.asm" -o "../binary_out/generated_%%~ni"
)

cd ..
cd binary_out

for %%i in (*.out) do (
    fc "%%~ni.out" "generated_%%~ni"
)

cd ..