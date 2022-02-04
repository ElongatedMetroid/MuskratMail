CC=gcc
SRC_SRV=Server/src/*.c
SRC_CLI=Client/src/*.c
FLGS= -O2
DEBFLGS = -Wall -g -DDEBON

all:
	$(CC) $(FLGS) $(SRC_SRV) -o bin/MUSKRAT-MAIL-SERVER_GNU-Linux.out
	$(CC) $(DEBFLGS) $(SRC_SRV) -o bin/DEBMUSKRAT-MAIL-SERVER_GNU-Linux.out
	$(CC) $(FLGS) $(SRC_CLI) -o bin/MUSKRAT-MAIL-CLIENT_GNU-Linux.out
	$(CC) $(DEBFLGS) $(SRC_CLI) -o bin/DEBMUSKRAT-MAIL-CLIENT_GNU-Linux.out