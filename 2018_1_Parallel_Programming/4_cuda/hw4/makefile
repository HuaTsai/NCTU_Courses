all:
	nvcc cuda_wave.cu -o cuda_wave
	gcc sserial_wave.c -o sserial_wave -lm -O3

test1:
	g++ check.cc -std=c++11
	./sserial_wave 100000 1000000 > 1.out
	./cuda_wave 100000 1000000 > 2.out
	@./a.out
	rm a.out

test2:
	g++ check.cc -std=c++11
	./sserial_wave 50000 50000 > 1.out
	./cuda_wave 50000 50000 > 2.out
	@./a.out
	rm a.out

test3:
	g++ check.cc -std=c++11
	./sserial_wave 10000 500000 > 1.out
	./cuda_wave 10000 500000 > 2.out
	@./a.out
	rm a.out

