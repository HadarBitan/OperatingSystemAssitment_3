#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/wait.h>
# include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
long End_time;
long Start_time;

#define BUFFSIZE (1024*1024*100)


long ReturnTimeNs() {
    struct timespec currTime;

    if (clock_gettime(CLOCK_REALTIME, &currTime) == -1) {
        perror("clock gettime");
        return EXIT_FAILURE;
    }
    return currTime.tv_nsec;
}

void UDS_Stream_Sender() {
    int server_sock, client_sock, len, rc;
    int bytes_rec = 0;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;
    char buf[256];
    int backlog = 10;
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(buf, 0, 256);


    //Create a UNIX domain stream socket
    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        printf("SOCKET ERROR\n");
        exit(1);
    }


    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SERVER_PATH);
    len = sizeof(server_sockaddr);

    unlink(SERVER_PATH);
    rc = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
    if (rc == -1) {
        printf("BIND ERROR:\n");
        close(server_sock);
        exit(1);
    }

    // Listen for any client sockets
    rc = listen(server_sock, backlog);
    if (rc == -1) {
        printf("LISTEN ERROR:\n");
        close(server_sock);
        exit(1);
    }


    //Accept an incoming connection
    client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, &len);
    if (client_sock == -1) {
        printf("ACCEPT ERROR:\n");
        close(server_sock);
        close(client_sock);
        exit(1);
    }

    // Get the name of the connected socket */
    len = sizeof(client_sockaddr);
    rc = getpeername(client_sock, (struct sockaddr *) &client_sockaddr, &len);
    if (rc == -1) {
        printf("GETPEERNAME ERROR:\n");
        close(server_sock);
        close(client_sock);
        exit(1);
    }


    // Send data back to the connected socket
    char *data = malloc(BUFFSIZE * sizeof(char));
    memset(data, 0, BUFFSIZE);
    //data = OpenAndReadFile(data);
    int scrFile = 0;
    // let us open the input file
    scrFile = open("file.txt", O_RDONLY);
    if (scrFile > 0) { // there are things to read from the input
        int succe = read(scrFile, data, BUFFSIZE);
        close(scrFile);
        data = data;

        //Take time before send
        Start_time = ReturnTimeNs();
        printf("Start time of UDS-Stream: %ld\n", Start_time);
//    int checksum = sender(data, BUFFSIZE);


        for (int i = 0; i < BUFFSIZE / 1024; i++) {
            rc = send(client_sock, data, 1024, 0);
            if (rc == -1) {
                printf("SEND ERROR:");
                close(server_sock);
                close(client_sock);
                exit(1);
            } else {
                data += 1024;
            }
        }


        //Close the sockets and exit
        close(server_sock);
        close(client_sock);
    }}

void UDS_Stream_Receiver() {
    int client_sock, rc, len;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;
    char *buf = malloc(BUFFSIZE * sizeof(char));
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));


    //Create a UNIX domain stream socket */

    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock == -1) {
        printf("SOCKET ERROR =\n");
        exit(1);
    }


    client_sockaddr.sun_family = AF_UNIX;
    strcpy(client_sockaddr.sun_path, CLIENT_PATH);
    len = sizeof(client_sockaddr);

    unlink(CLIENT_PATH);
    rc = bind(client_sock, (struct sockaddr *) &client_sockaddr, len);
    if (rc == -1) {
        printf("BIND ERROR:\n");
        close(client_sock);
        exit(1);
    }


    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SERVER_PATH);
    while (connect(client_sock, (struct sockaddr *) &server_sockaddr, len) < 0) {
        continue;
    }


    char *data = malloc(BUFFSIZE * sizeof(char));
    memset(data, 0, BUFFSIZE);
    //data = OpenAndReadFile(data);
    int scrFile = 0;
    scrFile = open("file.txt", O_RDONLY);
    if (scrFile > 0) { // there are things to read from the input
        int surce = read(scrFile, data, BUFFSIZE);
        close(scrFile);
        data = data;


        int checksum;
        memset(buf, 0, BUFFSIZE);

        for (int i = 0; i < BUFFSIZE / 1024; i++) {
            char *tempBuff = malloc(1024);
            rc = recv(client_sock, tempBuff, 1024, 0);
            if (rc == -1) {
                printf("RECV ERROR =\n");
                close(client_sock);
                exit(1);
            } else {
                strcat(buf, tempBuff);
                free(tempBuff);
            }
        }
        //checksum = sender(data, BUFFSIZE);
        int sum = 0, i;
        for (i = 0; i < BUFFSIZE; i++)
            sum += data[i];
        checksum = ~sum;
        //Take time after receive
        End_time = ReturnTimeNs();

        int checksumReceived;//= receiver(buf, BUFFSIZE, checksum);
        int sumR = 0, j;
        for (j = 0; j < BUFFSIZE; j++)
            sumR += buf[j];
        sumR = sumR + checksum;
        checksumReceived = ~sumR;    //1's complement of sum
        if (checksumReceived != 0) {
            End_time = -1;
        }
        printf("End time of UDS-Stream: %ld\n", End_time);
        printf("The checksum difference is: %d\n", checksumReceived);


        // Close the socket and exit.
        close(client_sock);
        free(buf);
    }
}

int main() {
    if (fork() == 0) {
        UDS_Stream_Sender();
    } else {
        if (fork() == 0) {
            UDS_Stream_Receiver();
        }
        else {
            wait(NULL);
        }
        wait(NULL);
    }
}