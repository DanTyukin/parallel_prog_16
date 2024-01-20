include ../common/common.mk

sources = solution_pthreads.c solution_join.c solution_barrier.c ../common/io.c ../common/iteration.c
lib = -lpthread

out/join.o:
	gcc -o out/join.o ${lib} -DJOIN ${sources} -lm

out/speedtest.join.o:
	gcc -o out/speedtest.join.o ${lib} -DJOIN -DSPEED_TEST=1 ${sources} -lm

out/barrier.o:
	gcc -o out/barrier.o ${lib} ${sources} -lm

out/speedtest.barrier.o:
	gcc -o out/speedtest.barrier.o ${lib} -DSPEED_TEST=1 ${sources} -lm

test_join: out/join.o
	cat ${test_data} | ./out/join.o | ${tester}

speed_join: out/speedtest.join.o
	cat ${exp1} | ${runner} ./out/speedtest.join.o {NP} 

run_join: out/join.o
	@./join.o ${N}

test_barrier: out/barrier.o
	cat ${test_data} | ./out/barrier.o | ${tester}

speed_barrier: out/speedtest.barrier.o
	cat ${exp1} | ${runner} ./out/speedtest.barrier.o {NP} > res/barrier.csv

run_barrier: out/barrier.o
	@./out/barrier.o ${N}

rebuild: clean out/join.o out/barrier.o out/speedtest.join.o out/speedtest.barrier.o

speedtest: speed_barrier

test: test_barrier

run: run_barrier