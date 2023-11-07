#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

struct message {
    long mtype;       
    int client_id;    
    int command;      
    char filename[256];
};


#define CLIENT_MSG_TYPE 1
#define SERVER_MSG_TYPE 2

int main() {
    int msg_queue = msgget(ftok(".", 'A'), 0666);
    if (msg_queue == -1) {
        perror("msgget");
        exit(1);
    }

    while (1) {
        char choice;
        printf("Do you want the server to terminate? Press Y for Yes and N for No: ");
        scanf(" %c", &choice);

        if (choice == 'Y' || choice == 'y') {
            struct message terminate_msg;
            terminate_msg.mtype = CLIENT_MSG_TYPE;
            terminate_msg.client_id = 0;  
            terminate_msg.command = 4;
            if (msgsnd(msg_queue, &terminate_msg, sizeof(terminate_msg) - sizeof(long), 0) == -1) {
                perror("msgsnd");
                exit(1);
            }
            exit(0);
        } 
        
        else if (choice == 'N' || choice == 'n') {
        } 
        
        else {
            printf("Invalid choice. Please enter Y or N.\n");
        }
    }

    return 0;
}
