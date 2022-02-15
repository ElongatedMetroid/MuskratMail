CC=gcc
SRC_SRV=Server/*.c Server/src/*.c include/*.c
SRC_CLI=Client/*.c include/*.c
FLGS= -O2 
LIBS= -lpthread
DEBFLGS = -Wall -g -DDEBON

all:
	$(CC) $(FLGS) $(SRC_SRV) -o bin/MUSKRAT-MAIL-SERVER_GNU-Linux.out $(LIBS) 
	$(CC) $(DEBFLGS) $(SRC_SRV) -o bin/DEBMUSKRAT-MAIL-SERVER_GNU-Linux.out $(LIBS) 
	$(CC) $(FLGS) $(SRC_CLI) -o bin/MUSKRAT-MAIL-CLIENT_GNU-Linux.out $(LIBS) 
	$(CC) $(DEBFLGS) $(SRC_CLI) -o bin/DEBMUSKRAT-MAIL-CLIENT_GNU-Linux.out $(LIBS)  