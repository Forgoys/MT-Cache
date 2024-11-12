ENV_ROOT=/thfs3/software/programming_env/mt3000_programming_env/hthreads
SRC=host.c
EXE=host
ALL:
	gcc -O2 -g3 -fopenmp -ggdb ${SRC} -std=c99 -I./ -I$(ENV_ROOT)/include $(ENV_ROOT)/lib/libhthread_host.a -lpthread -o ${EXE}
	cp ${EXE} ../bin/
	
simu:
	gcc -O0 -g3 -ggdb ${SRC} -std=c99 -I./ -I$(ENV_ROOT)/include $(ENV_ROOT)/lib/libmt_host_simu.a -lpthread -o ${EXE}

clean:
	rm ${EXE}
