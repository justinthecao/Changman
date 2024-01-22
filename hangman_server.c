// Justin Cao, I used the starter code from linustechtips example of tcp client/server in C, at the end of the section slides.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

int numUsers = 0;

int totalUsers = 0;

typedef struct{
    int clientId;
    char secret[255];
    char alpha[255];
    int guesses;
} Client;

struct ThreadArgs{
    int client_socket;
    char data[256];
};

Client clientList[100];

char words[15][500];
int wordCount = 0;

int findSecret(int id){
    for(int i = 0; i < 100; i++){
        if(clientList[i].clientId == id){
            return i;
        }
    }
    return 0;
}

void update(int id, char guess){
    int index = findSecret(id);
    for(int i = 0; i < strlen(clientList[index].alpha); i++){
        if(guess == clientList[index].alpha[i]){
            return;
        }
    }
    clientList[index].alpha[strlen(clientList[index].alpha)] = guess;
    clientList[index].alpha[strlen(clientList[index].alpha) + 1] = '\0';
    clientList[index].guesses++;
}

bool inSecret(char secret[], char guess, char* state){
    bool inState = false;
    for(int i = 0 ; i < strlen(secret); i++){
        if (secret[i] == guess){
            state[i] = guess;
            inState = true;
            
        }
    }
    if(inState){ 
        return true;
    }
    return false;
}

//handles all the messages that come from a client socket 
void *handlemessage(void *args){
    struct ThreadArgs *thread_args = (struct ThreadArgs *)args;
    int client_socket = thread_args->client_socket;
    char data[255];
    char secret[255];
    printf("Socket: %d\n", client_socket);
    printf("Index: %d\n", findSecret(client_socket));
    fflush(stdout);
    strcpy(data, thread_args->data); 
    strcpy(secret, clientList[findSecret(client_socket)].secret); 
    int len = strlen(secret);    
    char reply[255];
    if(data[0] == '0'){
        reply[0] = '0';
        reply[1] = '0' + strlen(secret);
        reply[2] = '0';
        for(int i = 0; i < len; i++){
            reply[3 + i] = '_';
        }
        send(client_socket, reply, strlen(reply), 0); 
    }
    else if (data[0] == '1'){
        update(client_socket,data[1]);
        char currState[255]; //their guess currently
        char tries[255];
        strcpy(tries, clientList[findSecret(client_socket)].alpha);
        int guesses = clientList[findSecret(client_socket)].guesses;
        char wrong[255];
        wrong[0] = '\0';
        for(int i = 0; i < len; i ++){
            currState[i] = '_';
        }
        currState[len] = '\0';
        for(int i = 0; i < strlen(tries); i ++){
            if(inSecret(secret, tries[i], currState)){
                continue;
            }
            else{
                char character[2] = "";
                character[0] = tries[i];
                character[1] = '\0'; 
                strcat(wrong, character);
            }
        }
        if(strcmp(currState,secret) == 0){
            strcpy(reply,"9>>>The word was ");
            char word[255];
            for(int i = 0 ; i < len; i ++){
                word[2*i] = secret[i];
                word[2*i + 1] = ' ';
            }
            word[2*(len-1) + 1] = '\0';  
            strcat(reply,secret);
            strcat(reply,"\n>>>You Win!\n>>>Game Over!\n");
            send(client_socket, reply, strlen(reply), 0); 
            free(thread_args);
            return (void *)(intptr_t)1;
        }
        else if(strlen(wrong) == 6 && strcmp(currState,secret) != 0){
            strcpy(reply,"9>>>The word was ");
            char word[255];
            for(int i = 0 ; i < len; i ++){
                word[2*i] = secret[i];
                word[2*i + 1] = ' ';
            }
            word[2*(len-1) + 1] = '\0';  
            strcat(reply,secret);
            strcat(reply, "\n>>>You Lose!\n>>>Game Over!\n");
            send(client_socket, reply, strlen(reply), 0); 
            free(thread_args);
            return (void *)(intptr_t)1;
        } //figure out game state, # of guesses 
        else{
            reply[0] = '0';
            reply[1] = '0' + len;
            reply[2] = '0' + strlen(wrong);
            reply[3] = '\0';
            strcat(reply,currState);
            strcat(reply,wrong);

            send(client_socket, reply, strlen(reply), 0); 
        }
    }
    else{
        send(client_socket, data, strlen(data), 0); 
    }
    free(thread_args);
    return (void *)(intptr_t)0;    
}


//listener for a socket
void *listener(void *sock){
    int *new_sock_ptr = (int *) sock;
    int client_socket = *(int *) sock;
    char buffer[255];
    while(1){  
        bzero(buffer,255);
        if( recv(client_socket,buffer,255, 0) == 0){
            break;
        }
        struct ThreadArgs *thread_args = malloc(sizeof(struct ThreadArgs));
        thread_args->client_socket = client_socket;
        strcpy(thread_args->data,buffer);
        pthread_t thread;
        pthread_create(&thread, NULL, handlemessage, (void *)thread_args);
        void *result;
        pthread_join(thread, &result);
        if(result){
            break;
        }
    }
    int index = findSecret(client_socket);
    bzero(clientList[index].alpha, 255);
    bzero(clientList[index].secret, 255);
    clientList[index].clientId = -1;
    clientList[index].guesses = -1;
    close(client_socket);
    free(new_sock_ptr);
    numUsers--;
    printf("closed client");
    fflush(stdout);
    return NULL;
}


void getRandomWordFromFile() {

    // Open the file
    FILE* file = fopen("hangman_words.txt", "r");
    // Read the words from the file

    char buffer[255];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // Remove the newline character at the end
        buffer[strcspn(buffer, "\n")] = '\0';
        strcpy(words[wordCount], buffer);
        wordCount++;
        // Copy the word to the array
        // strncpy(words[wordCount], buffer, sizeof(words[wordCount]));
        // wordCount++;
    }
    fclose(file);
    

    return;  
}
int sockfd;
void sigintHandler(int sig){
    close(sockfd);
    exit(0);
}
int main(int argc, char *argv[])
{
    signal(SIGINT, sigintHandler);

     int portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
     listen(sockfd,10);
     getRandomWordFromFile();
     //waits for a new connection 
     while(1){
        clilen = sizeof(cli_addr);
        int newSock;
        newSock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);    
        if(numUsers == 3){
            // TODO: send server-overloaded
            send(newSock, ">>>server-overloaded\n", strlen(">>>server-overloaded\n"), 0); 
            close(newSock);
            continue;
            
        }
        send(newSock, "good", strlen("good"), 0);
        numUsers++;
        totalUsers++;
        Client newClient;
        newClient.clientId = newSock;
        
        int randomIndex = rand() % wordCount;
        strcpy(newClient.secret, words[randomIndex]);
        newClient.guesses = 0;
        clientList[totalUsers] = newClient;
        pthread_t thread;
        int *new_socket_ptr = malloc(sizeof(int));
        *new_socket_ptr = newSock;

        pthread_create(&thread, NULL, listener, (void *)new_socket_ptr);
        pthread_detach(thread); //detaches the thread so it can terminate independently
        fflush(stdout);
        //remember to close(newsockfd)
     }
     close(sockfd);
     return 0; 
}