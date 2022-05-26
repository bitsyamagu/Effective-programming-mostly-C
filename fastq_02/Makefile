all:
	clang -std=c11 -g -o fastq2 main2.c
	clang -std=c11 -g -o fastq3 main3.c
	clang -std=c11 -g -o fastq2_bulk main2_bulk.c

test:
	./fastq2 test.fastq
	./fastq3 test.fastq
