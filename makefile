CC = gcc
ARGS = -Wall
# Compiling all the dependencies
all: hangman_client hangman_server
# Replace <"your_program"> with the name of your specififc program.
# For example, the next line may look something like this: 'server_c_udp:
# server_c_udp.c' without quotes.
hangman_server:
	${CC} ${ARGS} -o hangman_server hangman_server.c
hangman_client:
	${CC} ${ARGS} -o hangman_client hangman_client.c 

clean:
	rm -f *.o hangman_server
	rm -f *.o hangman_client
