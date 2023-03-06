mkdir -p binary_out
cd listings

for i in *.asm; do
	asm_file="$i"
	out=${asm_file%????}
    nasm "$i" -o "../binary_out/$out.out"
done

cd ..

clang main.c -o simulator.out

mkdir -p output
cd binary_out

for i in *.out; do
	out_file="$i"
	out=${out_file%????}
	../simulator.out "$i" > "../output/$out.asm"
done

cd ..
cd output

for i in *.asm; do
	asm_file="$i"
	out=${asm_file%????}
    nasm "$i" -o "../binary_out/generated_$out"
done

cd ..
cd binary_out
for i in *.out; do
	out="$i"
	gen="generated_${out%????}"
	diff "$i" "$gen"
done