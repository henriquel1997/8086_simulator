nasm listing_0037_single_register_mov.asm
nasm listing_0038_many_register_mov.asm

./simulator listing_0037_single_register_mov > 37.asm
./simulator listing_0038_many_register_mov > 38.asm

nasm 37.asm
nasm 38.asm

diff 37 listing_0037_single_register_mov
diff 38 listing_0038_many_register_mov