#Both enclave and client use the same include files
INC= inc
IPPinc = IPPSGX/external/ippcp_internal/inc
IPPPath= /opt/intel/compilers_and_libraries_2018.0.128/linux/ippcp/lib/intel64_and
IPP_LIB_Client = ippcp
WOLF_DIR = /home/sgx/wolfssl  
WOLF_CRYPT = /home/sgx/wolfssl/wolfcypt
WOLF_LIB_Client = wolfssl
WOLF_D = WOLFSSL_SGX
TARGET_LIB_CLIENT = $(WOLF_LIB_Client)


client:	server
	g++  -I$(INC) -I$(IPPinc)   -I$(IPPinc)  -I$(WOLF_DIR) -I$(WOLF_CRYPT) -c client.c -o client.o
	g++  -I$(INC) -I$(IPPinc)   -I$(IPPinc)  -I$(WOLF_DIR) -I$(WOLF_CRYPT) -c clientOneByOne.c -o clientOneByOne.o
	g++  client.o -o  client  -L $(IPPPath)   -l$(TARGET_LIB_CLIENT) 
	g++ clientOneByOne.o -o clientOneByOne -L $(IPPPath)  -l$(TARGET_LIB_CLIENT)


server: clean

	g++ -m64 -O2 -fPIC -std=c++11   -c server.c -o server.o
	g++ server.o -o server

clean:
	rm -f *~ *# client clientOneByOne server *.o
