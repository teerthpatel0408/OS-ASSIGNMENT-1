#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


void send_message(int msg_queue, int client_id, int command, const char* filename) {
    struct message msg;
    msg.mtype = CLIENT_MSG_TYPE;
    msg.client_id = client_id;
    msg.command = command;
    if (filename != NULL) {
        strncpy(msg.filename, filename, sizeof(msg.filename));
    }

    if (msgsnd(msg_queue, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
}


void receive_message(int msg_queue, struct message* msg) {
    if (msgrcv(msg_queue, msg, sizeof(*msg) - sizeof(long), SERVER_MSG_TYPE, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
}

int main() {
    int client_id;
    printf("Enter Client-ID: ");
    scanf("%d", &client_id);
    int msg_queue = msgget(ftok(".", 'A'), 0666 | IPC_CREAT);
    while (1){
    	struct message msg;
    	if(msgrcv(msg_queue, &msg,  sizeof(struct message) - sizeof(long), 0,IPC_NOWAIT) == -1){
    		break;
    	}
    }
    if (msg_queue == -1) {
        perror("msgget");
        exit(1);
    }
    
    printf("Client ID: %d\n", client_id);
    loop:
    while (1) {
        printf("\nMenu:\n");
        printf("1. Contact the Ping Server\n");
        printf("2. Contact the File Search Server\n");
        printf("3. Contact the File Word Count Server\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");

        int choice;
        scanf("%d", &choice);
        
        char filename[256];

        switch (choice) {
            case 1:
                send_message(msg_queue, client_id, choice, NULL);
                break;
                
            case 2:
                printf("Enter filename: ");
                scanf("%s", filename);
                send_message(msg_queue, client_id, choice, filename);
                break;
                
            case 3:
                printf("Enter filename: ");
                scanf("%s", filename);
                send_message(msg_queue, client_id, choice, filename);
                break;
                
            case 4:
                exit(0);
                
            default:
                printf("Invalid choice\n");
                goto loop;
        }

        struct message response;
        receive_message(msg_queue, &response);
        printf("Server Response: %s\n", response.filename);
    }
    
    return 0;
}
