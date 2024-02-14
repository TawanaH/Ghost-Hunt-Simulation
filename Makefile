TARGETS = main.o helpers.o house.o room.o ghost.o hunters.o logger.o utils.o

all:	${TARGETS}
			gcc -Wextra -Wall -Werror -o finalProject ${TARGETS}

main.o:		main.c defs.h
			gcc -g -c main.c

helpers.o:	helpers.c defs.h
			gcc -pthread -g -c helpers.c

house.o:	house.c defs.h
			gcc -g -c house.c

room.o:		room.c defs.h
			gcc -g -c room.c

ghost.o:	ghost.c defs.h
			gcc -g -c ghost.c

hunters.o:	hunters.c defs.h
			gcc -g -c hunters.c

logger.o:	logger.c defs.h
			gcc -g -c logger.c

utils.o:	utils.c defs.h
			gcc -g -c utils.c

clean:
			rm -f ${TARGETS} finalProject