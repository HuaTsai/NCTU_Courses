all:
	./a.out 0.1 sample2.in output.out
	diff sample2.out output.out -b
	./a.out 0.02 sample3.in output.out
	diff sample3.out output.out -b
	./a.out 0.02 test3.in output.out
	diff test3.out output.out -b
test:
	g++ fpgrowth_test.cc -lgtest -lpthread
	./a.out
	rm a.out
main:
	g++ main.cc -fopenmp -O3
