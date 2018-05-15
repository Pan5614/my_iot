CROSS_COMPILE= 
CXX=${CROSS_COMPILE}g++
CC=${CROSS_COMPILE}gcc

CFLAGS = -g -Wall

 
CUR_DIR = $(shell pwd)
TOP_DIR = $(CUR_DIR)/..
SRC_DIR = $(TOP_DIR)/src
INC_DIR = $(TOP_DIR)/inc


CFLAGS := -w
CFLAGS += -I${INC_DIR}




CFLAGS += -I${TOP_DIR}/mylib/iot/include
LDFLAGS += -liot_sdk -liot_platform -Bstatic -liot_tls -lgcov -L${TOP_DIR}/mylib/iot/lib 


.PHONY:all
all: myiot install

myiot:main.o 
	$(CXX) main.o  -o myiot ${LDFLAGS}  

main.o: ${SRC_DIR}/main.cpp
	$(CXX) -c ${SRC_DIR}/main.cpp ${CFLAGS} ${LDFLAGS} 




.PHONY:install
install:
	@mkdir -p ${TOP_DIR}/out/
	@mv myiot ${TOP_DIR}/out/ -f
	@echo "\033[32m ######## make completed successfully ########\033[0m"
	@echo "\033[32m ######## `date` ########\033[0m"	

clean:
	@rm -rf *.o
	@rm ${TOP_DIR}/out/ -rf
	@echo "\033[32m ######## make clean successfully ########\033[0m"
	@echo "\033[32m ######## `date` ########\033[0m"
