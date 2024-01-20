#!/bin/bash

V=0
if [[ $1 == "-V" ]]; then 
    shift
    V=1
fi 

T=0
T_arg=""
T_cmd=""
if [[ $1 == "-T" ]]; then 
    shift
    T=1
    T_arg="-T"
    T_cmd="time -p"
fi

if [[ $1 == "all" ]]; then 
    shift
    for dir in cmpi pthreads openmp pympi
    do 
        ./launcher/runner.sh -V ${T_arg} $dir $@
        echo ; echo
    done 
    exit 0
fi 

if [[ $2 == "speedtest2" ]] || [[ $2 == "speedtest5" ]]; then 
    if [[ $1 != "cmpi" ]] && [[ $1 != "pympi" ]]; then 
        exit 0 
    fi
fi

if [[ $V == 1 ]]; then 
    echo Exucuting $@
    echo
fi

cd $1
make rebuild > /dev/null

if [[ $2 == "run" ]]; then 
    cat $3 | ${T_cmd} make run NP=$4
    exit 0
fi
${T_cmd} make $2