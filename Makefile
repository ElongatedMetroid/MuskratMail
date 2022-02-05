CC=gcc
SRC_SRV=Server/*.c include/*.c
SRC_CLI=Client/*.c Client/src/*.c include/*.c
FLGS= -O2 
LIBS= -lpthread
DEBFLGS = -Wall -g -DDEBON

all:
	$(CC) $(FLGS) $(LIBS) $(SRC_SRV) -o bin/MUSKRAT-MAIL-SERVER_GNU-Linux.out
	$(CC) $(DEBFLGS) $(LIBS) $(SRC_SRV) -o bin/DEBMUSKRAT-MAIL-SERVER_GNU-Linux.out
	$(CC) $(FLGS) $(LIBS) $(SRC_CLI) -o bin/MUSKRAT-MAIL-CLIENT_GNU-Linux.out
	$(CC) $(DEBFLGS) $(LIBS) $(SRC_CLI) -o bin/DEBMUSKRAT-MAIL-CLIENT_GNU-Linux.out