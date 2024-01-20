tester = ../common/tester.sh 
test_data = ../common/test.inputs.txt
runner = python3 ../common/runner.py

exp_folder = ../common/experiment

exp1 = $(exp_folder)/1node.txt
exp2 = $(exp_folder)/2node.txt 
exp5 = $(exp_folder)/5node.txt 

N = $(if $(NP), $(NP), 4) # default value for number of processes

clean:
	rm -f ./out/*.o