GCCROOT=/thfs3/software/programming_env/mt3000_programming_env/dsp_compiler
ENV_ROOT=/thfs3/software/programming_env/mt3000_programming_env/hthreads
CC=MT-3000-gcc
AR=MT-3000-ar
LD=MT-3000-ld

export LD_LIBRARY_PATH=/thfs3/software/programming_env/mt3000_programming_env/third-party-lib:$LD_LIBRARY_PATH

CFLAGS=-c -O2 -g -gdwarf-2 -fenable-m3000 -ffunction-sections -flax-vector-conversions -I./ -I$(ENV_ROOT)/include -I$(GCCROOT)/include/ -I/thfs3/home/xjtu_cx/lhj/mt3000-cache/define_version/
LDFLAGS= -L$(ENV_ROOT)/lib --gc-sections -Tdsp.lds
SRC=kernel.c
# SRC=kernel_preprocessed.c
OBJ=kernel.o
EXE=kernel.out
DAT=kernel.dat

# 如果定义了DEBUG变量，则添加-DDEBUG标志
ifeq ($(DEBUG), 1)
    CFLAGS += -DDEBUG
endif

# ifeq ($(DDR),1)
#     CFLAGS += -DDR
# endif

# ifeq ($(DMA),1)
#     CFLAGS += -DMA
# endif

ALL: $(EXE)
	$(GCCROOT)/bin/MT-3000-makedat -J $(EXE)
	cp $(DAT) ../bin

$(OBJ): $(SRC)
	$(GCCROOT)/bin/$(CC) $(CFLAGS) $(SRC) -o $(OBJ) 
# $(GCCROOT)/bin/$(CC) $(CFLAGS) -E $(SRC) -o kernel_preprocessed.c
# $(GCCROOT)/bin/$(CC) $(CFLAGS) kernel_preprocessed.c -o $(OBJ) 

$(EXE): $(OBJ)
	$(GCCROOT)/bin/$(LD) $(LDFLAGS) $(OBJ) $(ENV_ROOT)/lib/libhthread_device.a $(GCCROOT)/lib/vlib3000.a $(GCCROOT)/lib/slib3000.a -o $(EXE) 

clean:
	rm $(EXE) $(OBJ) $(DAT)
