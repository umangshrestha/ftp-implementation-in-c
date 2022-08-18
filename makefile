BIN=./bin
SRC=./src
# src file
CLIENT_S=${SRC}/client.c
SERVER_S=${SRC}/server.c
LOG_S   =${SRC}/log.c
# executable
CLIENT_O=${BIN}/client
SERVER_O=${BIN}/server
# cmd
CC=cc
RM=rm
FLAGS=-g -Wall -lpthread
all: ${CLIENT_S} ${SERVER_S} ${LOG_S}
	${CC} -o ${CLIENT_O} ${CLIENT_S} ${LOG_S}
	${CC} ${FLAGS} -o ${SERVER_O} ${SERVER_S} ${LOG_S}
client: ${CLIENT_S} ${LOG_S}
	${CC} -o ${CLIENT_O} ${CLIENT_S} ${LOG_S}
server: ${SERVER_S} ${LOG_S}
	${CC} ${FLAGS} -o ${SERVER_O} ${SERVER_S} ${LOG_S}
clean:
	${RM} ${CLIENT_O} ${SERVER_O}
        
