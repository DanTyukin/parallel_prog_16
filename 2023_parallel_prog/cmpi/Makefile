include ../common/common.mk

sources = solution_mpi.c ../common/io.c ../common/iteration.c
lib = -lm

out/solution.o:
	mpicc -o out/solution.o ${sources} ${lib} 

run: out/solution.o
	@mpiexec -np $(N) ./out/solution.o

rebuild: clean out/solution.o out/speedtest.o

test: out/solution.o
	cat ${test_data} | mpirun -np 8 ./out/solution.o | ${tester} 

out/speedtest.o:
	mpicc -o out/speedtest.o -D SPEED_TEST=1 ${sources} ${lib}

speedtest: out/speedtest.o
	cat ${exp1} | ${runner} mpirun -np {NP} ./out/speedtest.o 

speedtest2: out/speedtest.o 
	cat ${exp2} | ${runner} mpirun -np {NP} ./out/speedtest.o 

speedtest5: out/speedtest.o 
	cat ${exp5} | ${runner} mpirun -np {NP} ./out/speedtest.o 
