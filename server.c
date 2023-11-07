#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>


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
    msg.mtype = SERVER_MSG_TYPE;
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
    if (msgrcv(msg_queue, msg, sizeof(*msg) - sizeof(long), CLIENT_MSG_TYPE, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
}


void handle_ping(int msg_queue, int client_id) {
    send_message(msg_queue, client_id, 1, "Hello");
}


void handle_file_search(int msg_queue, int client_id, const char* filename) {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(1);
    }

    if (child_pid == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        execlp("find", "find", ".", "-name", filename, NULL);

        perror("execlp");
        exit(1);
    } 
    
    else {
        close(pipe_fd[1]);

        int status;
        waitpid(child_pid, &status, 0);

        char result[1024];
        ssize_t bytes_read = read(pipe_fd[0], result, sizeof(result));

        close(pipe_fd[0]);

        if (bytes_read == -1) {
            perror("read");
            exit(1);
        }

        if (bytes_read > 0) {
            result[bytes_read] = '\0';
            send_message(msg_queue, client_id, 2, "File found"); 
        } 
        
        else {
            send_message(msg_queue, client_id, 2, "File not found."); 
        }
    }
    
}


void handle_word_count(int msg_queue, int client_id, const char* filename) {
    int pipe_fd[2]; 
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) { 
        close(pipe_fd[0]); 
        dup2(pipe_fd[1], STDOUT_FILENO); 
        close(pipe_fd[1]); 

        execlp("wc", "wc", "-w",filename, NULL);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } 
    
    else { 
    	
    	wait(NULL); 
        close(pipe_fd[1]); 

        char buffer[1024];
        ssize_t bytes_read;
        bytes_read = read(pipe_fd[0], buffer, sizeof(buffer));
      	
        close(pipe_fd[0]); 
        if(bytes_read == -1) {
        	perror("Read error");
        	exit(1);
        }
        char word_count[256];
        int i,j;
        for(i= 0,j=0;i<bytes_read;i++) {
        	if(buffer[i] == ' '){
        		break;
        	}
        	word_count[j++] = buffer[i];
        }
        word_count[j] = '\0';
        
        send_message(msg_queue, client_id, 3, word_count);
    }
    
}

int main() {
    int msg_queue = msgget(ftok(".", 'A'), 0666 | IPC_CREAT);
    if (msg_queue == -1) {
        perror("msgget"); 
        exit(1);
    }
    
    
    while (1) {
        struct message request;
        receive_message(msg_queue, &request);
	
	if(request.client_id == 0){
		if (msgctl(msg_queue, IPC_RMID, NULL) == -1) {
		        exit(1);
		}

		exit(0);
	}
	
        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("fork");
            exit(1);
        }

        if (child_pid == 0) {
            switch (request.command) {
                case 1:
                    handle_ping(msg_queue, request.client_id);
                    break;
                    
                case 2:
                    handle_file_search(msg_queue, request.client_id, request.filename);
                    break;
                    
                case 3:
                    handle_word_count(msg_queue, request.client_id, request.filename);
                    break;
                    
                case 4:
		    while (wait(NULL) > 0) {
		    }
		    exit(0);
                    
                default:
                    exit(1);
            }
        } 
        
        else {
	    while (wait(NULL) > 0) {
	    }
	    
	    if (msgctl(msg_queue, IPC_RMID, NULL) == -1) {
		 exit(1);
	    }

            exit(0);
        }
    }

    msgctl(msg_queue, IPC_RMID, NULL);
    
    return 0;
}
