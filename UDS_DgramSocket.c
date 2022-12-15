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
#include <errno.h>

#define SERVER_PATH_DGRAM "tpf_unix_sock.server_Dgram"

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

void UDS_Dgram_Sender() {
    int client_socket, rc;
    struct sockaddr_un remote;
    char *buf = malloc(BUFFSIZE);
    char *data = malloc(BUFFSIZE);
    int scrFile = 0;
    scrFile = open("file.txt", O_RDONLY);
    if (scrFile > 0) {
        int succe = read(scrFile, data, BUFFSIZE);
        close(scrFile);
        data = data;


        //Take time before send
        Start_time = ReturnTimeNs();
        printf("Start time of UDS-Stream: %ld\n", Start_time);
        int checksum, sum = 0, i;
        for (i = 0; i < BUFFSIZE; i++)
            sum += data[i];
        checksum = ~sum;

        memset(&remote, 0, sizeof(struct sockaddr_un));

        // Create a UNIX domain datagram socket

        client_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (client_socket == -1) {
            printf("SOCKET ERROR = %s\n", strerror(errno));
            exit(1);
        }

        // Set up the UNIX sockaddr structure
        remote.sun_family = AF_UNIX;
        strcpy(remote.sun_path, SERVER_PATH_DGRAM);


        //Copy the data to be sent to the
        //buffer and send it to the server.
        strcpy(buf, data);
        buf[BUFFSIZE] = '\0';
        char *startPointer = buf;
        for (int i = 0; i < BUFFSIZE / 1024; i++) {
            char *tempSend = malloc(1024);
            strncpy(tempSend, startPointer, 1024);
            rc = sendto(client_socket, tempSend, 1024, 0, (struct sockaddr *) &remote, sizeof(remote));
            if (rc == -1) {
                printf("SENDTO ERROR = %s\n", strerror(errno));
                close(client_socket);
                exit(1);
            } else {
                startPointer += 1024;
            }
            free(tempSend);
        }


        // Close the socket and exit

        rc = close(client_socket);
    }}

void UDS_Dgram_Receiver() {
    int server_sock, len, rc;
    int bytes_rec = 0;
    struct sockaddr_un server_sockaddr, peer_sock;
    char *buf = malloc(BUFFSIZE);
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(buf, 0, 256);


    //Create a UNIX domain datagram socket
    server_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (server_sock == -1) {
        printf("SOCKET ERROR = %s\n", strerror(errno));
        exit(1);
    }

    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SERVER_PATH_DGRAM);
    len = sizeof(server_sockaddr);
    unlink(SERVER_PATH_DGRAM);
    rc = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
    if (rc == -1) {
        printf("BIND ERROR = %s\n", strerror(errno));
        close(server_sock);
        exit(1);
    }
    char *trueData = malloc(BUFFSIZE * sizeof(char));
    memset(trueData, 0, BUFFSIZE);
    // trueData = OpenAndReadFile(trueData);
    int scrFile = 0;
    scrFile = open("file.txt", O_RDONLY);
    if (scrFile > 0) { // there are things to read from the input
        int surce = read(scrFile, trueData, BUFFSIZE);
        close(scrFile);
        trueData = trueData;

        char *data = malloc(BUFFSIZE);
        for (int i = 0; i < BUFFSIZE / 1024; i++) {
            bytes_rec = recvfrom(server_sock, buf, 1024, 0, (struct sockaddr *) &peer_sock, &len);
            if (bytes_rec == -1) {
                printf("RECVFROM ERROR = %s\n", strerror(errno));
                close(server_sock);
                exit(1);
            } else {
                strcat(data, buf);
            }
        }
        int checksum, sum = 0, i;
        for (i = 0; i < BUFFSIZE; i++)
            sum += data[i];
        checksum = ~sum;
        //Take time after receive
        End_time = ReturnTimeNs();
        int checksumReceived, sumR = 0, j;
        for (j = 0; j < BUFFSIZE; j++)
            sumR += trueData[j];
        sumR = sumR + checksum;
        checksumReceived = ~sumR;

        if (checksumReceived != 0) {
            End_time = -1;
        }
        printf("End time of UDS-Datagram: %ld\n", End_time);
        printf("The checksum difference for UDS-Datagram is: %d\n", checksumReceived);


        close(server_sock);
    }}
int main() {
    if (fork() == 0) {
        UDS_Dgram_Sender();
    } else {
        if (fork() == 0) {
            UDS_Dgram_Receiver();
        }
        else {
            wait(NULL);
        }
        wait(NULL);
    }
}