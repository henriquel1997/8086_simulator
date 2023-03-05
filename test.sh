nasm listing_0037_single_register_mov.asm
nasm listing_0038_many_register_mov.asm
nasm listing_0039_more_movs.asm
nasm listing_0040_challenge_movs.asm

clang main.c -o simulator.out

./simulator.out listing_0037_single_register_mov > 37.asm
./simulator.out listing_0038_many_register_mov > 38.asm
./simulator.out listing_0039_more_movs > 39.asm
./simulator.out listing_0040_challenge_movs > 40.asm

nasm 37.asm
nasm 38.asm
nasm 39.asm
nasm 40.asm

diff 37 listing_0037_single_register_mov
diff 38 listing_0038_many_register_mov
diff 39 listing_0039_more_movs
diff 40 listing_0040_challenge_movs