include ../common/common.mk

sources = solution_omp.c ../common/io.c ../common/iteration.c
lib = -lm

out/solution.o:
	gcc -o out/solution.o -fopenmp ${sources} ${lib}

out/speedtest.o:
	gcc -o out/speedtest.o -D SPEED_TEST=1 -fopenmp ${sources} ${lib}

run: out/solution.o
	@./out/solution.o ${N}

rebuild: clean out/solution.o out/speedtest.o

test: out/solution.o
	cat ${test_data} | ./out/solution.o | ${tester} 

speedtest: out/speedtest.o
	cat ${exp1} | ${runner} ./out/speedtest.o {NP} 
