all:
	mpicc prime.c -lm -O3 -o prime
	mpicc integrate.c -lm -O3 -o integrate

task1: SHELL:=/bin/bash
task1:
	@echo -e 'process 4'
	time mpiexec -n 4 ./prime 6000000
	@echo -e '\nprocess 2'
	time mpiexec -n 2 ./prime 6000000
	@echo -e '\nprocess 1'
	time mpiexec -n 1 ./prime 6000000

task2: SHELL:=/bin/bash
task2:
	@echo -e 'process 4'
	time mpiexec -n 4 ./integrate 100000000
	@echo -e '\nprocess 2'
	time mpiexec -n 2 ./integrate 100000000
	@echo -e '\nprocess 1'
	time mpiexec -n 1 ./integrate 100000000

