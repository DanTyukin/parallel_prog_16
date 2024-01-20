include ../common/common.mk

rebuild:
	echo Reduilding stub

run: 
	@mpiexec -n $(N) python3 -m mpi4py solution_mpi.py

test: 
	cat ${test_data} | mpiexec -n 4 python3 -m mpi4py solution_mpi.py | ${tester}

speedtest:
	cat ${exp1} | ${runner} mpiexec -n {NP} python3 -m mpi4py solution_mpi.py SPEED_TEST

speedtest2:
	cat ${exp2} | ${runner} mpiexec -n {NP} python3 -m mpi4py solution_mpi.py SPEED_TEST

speedtest5:
	cat ${exp5} | ${runner} mpiexec -n {NP} python3 -m mpi4py solution_mpi.py SPEED_TEST
