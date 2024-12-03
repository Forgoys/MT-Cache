include make.conf
# CACHE_PATH:=/thfs3/home/xjtu_cx/lhj/mt3000-cache/define_version/
MY_INCLUDE_PATH:=/thfs3/home/xjtu_cx/lhj/mt3000-cache/cache_test/cache_single_test/include
HTHREADS_INCLUDE_PATH:=${HTHREADS_ROOT_PATH}/include

HTHREADS_LIB_PATH:=${HTHREADS_ROOT_PATH}/lib
# #
HOS_CC:=gcc
HOS_LDFLAGS:=-L${HTHREADS_LIB_PATH} -lhthread_host -lm -lpthread
HOS_CFLAGS:= -Wall -I${HTHREADS_INCLUDE_PATH} -I${MY_INCLUDE_PATH}

# #
DEV_CC_BIN_PATH:=${DEV_CC_ROOT_PATH}/bin
DEV_CC:=${DEV_CC_BIN_PATH}/MT-3000-gcc
DEV_LD:=${DEV_CC_BIN_PATH}/MT-3000-ld 
DEV_MAKEDAT:=${DEV_CC_BIN_PATH}/MT-3000-makedat
DEV_CC_INCLUDE_PATH:=${DEV_CC_ROOT_PATH}/include
DEV_CC_LIB_PATH:=${DEV_CC_ROOT_PATH}/lib
# CACHE_PATH:=/thfs3/home/xjtu_cx/lhj/mt3000-cache/define_version/
DEV_INCLUDE_PATH:=${DEV_CC_INCLUDE_PATH} ${HTHREADS_INCLUDE_PATH} ${MY_INCLUDE_PATH}
DEV_INCLUDE_FLAGS:=$(foreach i, ${DEV_INCLUDE_PATH}, -I${i})
DEV_WALLFLAGS:=-Wall -Wno-attributes -Wno-unused-function
DEV_CFLAGS:= $(DEV_WALLFLAGS) -O2 -fenable-m3000 -ffunction-sections \
			-flax-vector-conversions ${DEV_INCLUDE_FLAGS}

DEV_LIBRARY_PATH:=${DEV_CC_LIB_PATH} ${HTHREADS_LIB_PATH}
DEV_LDFLAGS:=$(foreach i, ${DEV_LIBRARY_PATH}, -L${i})
DEV_LDFLAGS+= --gc-sections -T$(HTHREADS_LIB_PATH)/dsp.lds -lhthread_device \
			${DEV_CC_LIB_PATH}/slib3000.a

export LD_LIBRARY_PATH=/thfs3/software/programming_env/mt3000_programming_env/third-party-lib:$LD_LIBRARY_PATH


###############################################################################
.PHONY: ALL
ALL: gemm_test_tuning

%.dev.o:%.dev.c
	$(DEV_CC) -c $(DEV_CFLAGS) $^ -o $@

%.dev.out:%.dev.o
	$(DEV_LD) $^ $(DEV_LDFLAGS) -o $@ 

%.dev.dat:%.dev.out
	$(DEV_MAKEDAT) -J $^

%.hos:%.hos.c
	${HOS_CC} -o $@ -fopenmp $^ ${HOS_CFLAGS} ${HOS_LDFLAGS}

gemm_test_tuning: gemm_test_tuning.hos gemm_test_tuning.dev.dat

.PHONY: clean
clean:
	rm -rf *.out *.dat *.asm *.o
	rm -rf gemm_test_tuning.hos
