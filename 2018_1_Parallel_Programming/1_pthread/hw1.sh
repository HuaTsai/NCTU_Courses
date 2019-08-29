gcc pi.c -pthread
echo 1 core
time taskset -c 1 ./a.out 1 100000000
echo 2 cores
time taskset -c 1,2 ./a.out 2 100000000
echo 4 cores
time taskset -c 1,2,3,4 ./a.out 4 100000000
rm a.out
