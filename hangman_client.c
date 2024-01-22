// Justin Cao, I used the starter code from linustechtips example of tcp client/server in, at the end of the section slides.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdbool.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <signal.h>


int sockfd;

void sig(int sig){
    close(sockfd);
    exit(0);
}


int main(int argc, char *argv[])
{
    signal(SIGINT,sig);
    int portno;
    struct sockaddr_in serv_addr;
    struct hostent *hp;
    char buffer[256];
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    hp = gethostbyname(argv[1]);
    bcopy((char *)hp->h_addr, 
        (char *)&serv_addr.sin_addr,
        hp->h_length);

    serv_addr.sin_port = htons(portno);
    connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
    recv(sockfd,buffer,255, 0);
    if(buffer[0] == '>'){
        printf(buffer);
        fflush(stdout);
        close(sockfd);
        return 0;
    }
    printf(">>>Ready to start game? (y/n): ");
    bzero(buffer,256);
    fgets(buffer,256,stdin);
    if(buffer[0] == 'n'){
        close(sockfd);
        return 0;
    }
    
    bzero(buffer,256);
    buffer[0] = '0';
    write(sockfd, buffer, strlen(buffer));
    while(1){
        
        bzero(buffer,256);
        recv(sockfd,buffer,255, 0);

        if(buffer[0] == '0'){
            char lengthA[2];
            lengthA[0] = buffer[1];
            lengthA[1] = '\0';
            int length = atoi(lengthA);
            char numIncA[2];
            numIncA[0] = buffer[2];
            numIncA[1] = '\0';
            int numInc = atoi(numIncA);
            char state[255];
            strncpy(state, &buffer[3], length);
            state[length] = '\0';
            char formatState[255];
            for(int i = 0; i < length; i++){
                formatState[i*2] = state[i];
                formatState[i*2 + 1] = ' ';
            }            
            formatState[2*(length-1) + 1] = '\0';
            printf(">>>");
            fflush(stdout);
            printf(formatState);
            
            printf("\n");
            fflush(stdout);
            printf(">>>Incorrect Guesses: ");
            fflush(stdout);
            strncpy(state, &buffer[3+length], numInc);
            state[numInc] = '\0';
            char incFor[255];
            for(int i = 0 ; i < strlen(state); i++){
                incFor[2*i] = state[i];
                incFor[2*i + 1] = ' ';
            }
            incFor[2 * numInc - 1] = '\0';
            printf(incFor);
            fflush(stdout);
            printf("\n>>>\n");
            fflush(stdout);
        }
        else if(buffer[0] == '9'){
            char output[256];
            for(int i = 0 ; i <= strlen(buffer); i ++){
                output[i] = buffer[i+1];
            }
            printf(output);
            break;
        } else{
            break;
        }

        printf(">>>Letter to guess: ");
        bzero(buffer,256);
        if(fgets(buffer,256,stdin) == NULL){
            close(sockfd);
            printf("\n");
            return 0;
        }
        while(!(strlen(buffer) == 2 && isalpha(buffer[0]))){
            printf(">>>Error! Please guess one letter.\n");
            printf(">>>Letter to guess: "); 
            bzero(buffer,256);
            if(fgets(buffer,256,stdin) == NULL){
                close(sockfd);
                printf("\n");
                return 0;
            }
        }
        buffer[1] = tolower(buffer[0]);
        buffer[0] = '1';
        buffer[2] = '\0';
        write(sockfd,buffer,strlen(buffer));
    }
    close(sockfd);
    return 0;
}