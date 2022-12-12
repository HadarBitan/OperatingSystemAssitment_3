#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

//this is the size in bytes of 100MB
#define SIZE_IN_BYTES 104857600

int process1(char * portNum, char * ipAddr, char * Stime, char * file) {
    int port = atoi(portNum);//convert the string of port from user to int
    int socket_desc;
    struct sockaddr_in server_addr;

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Unable to create socket\n");
        return -1;
    }

    printf("Socket created successfully\n");
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ipAddr);

    // Send connection request to server:
    if (connect(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");

    //checksum put in ChSu1
    int ChSu, sum = 0;
    for (int i = 0; i < SIZE_IN_BYTES; i++)
        sum += file[i];
    ChSu = ~sum;    //1's complement of sum

    //check time
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    strftime(Stime, sizeof(Stime), "%Y-%m-%d, time is %H:%M", &tm_now);

    //send the data to process2
    if (send(socket_desc, file, strlen(file), 0) < 0) {
        printf("Unable to send message\n");
        return -1;
        return ChSu;
    }
}

int process2(char * portNum, char * Etime)
{
    int port = atoi(portNum);//convert the string of port from user to int
    int socket_desc, client_sock, client_size;
    struct sockaddr_in server_addr, client_addr;
    char dataRecived[SIZE_IN_BYTES];

    // Clean buffer:
    memset(dataRecived, '\0', sizeof(dataRecived));

    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    // Listen for clients:
    if (listen(socket_desc, 1) < 0) {
        printf("Error while listening\n");
        return -1;
    }
    printf("\nListening for incoming connections.....\n");

    // Accept an incoming connection:
    client_size = sizeof(client_addr);
    client_sock = accept(socket_desc, (struct sockaddr *) &client_addr, &client_size);

    if (client_sock < 0) {
        printf("Can't accept\n");
        return -1;
    }

    //get the data
    if (recv(client_sock, dataRecived, sizeof(dataRecived), 0) < 0) {
        printf("Couldn't receive\n");
        return -1;
    }
    //check end time
    time_t now = time(NULL);
    struct tm tm_now;
    localtime_r(&now, &tm_now);
    strftime(Etime, sizeof(Etime), "%Y-%m-%d, time is %H:%M", &tm_now);

    //checksum put in ChSu2
    int ChSu, sum = 0;
    for (int i = 0; i < SIZE_IN_BYTES; i++)
        sum += dataRecived[i];
    ChSu = ~sum;    //1's complement of sum
    return ChSu;
}

int main(int argc, char * argv[])
{
    //create a 100MB data, for the creation of the file we used https://www.web-workers.ch/index.php/2017/06/23/how-to-create-a-1gb-100mb-10mb-file-for-testing/
    //we created a regular 100MB file in linux and open, read in here.
    char * file = malloc(SIZE_IN_BYTES);
    int fd = open("/home/hadarb/Desktop/OS3/file.txt", O_RDONLY);
    if(fd == -1)
    {
        printf("couldn't open the file");
        return 0;
    }
    read(fd, file, SIZE_IN_BYTES);
    //create times for start and end
    char * startTime, * endTime;
    startTime = malloc(100);
    endTime = malloc(100);
    int ChSu1, ChSu2;
    //call process1(client) and send him the data, IP address and port
    ChSu1 = process1(argv[1], argv[2], startTime, file);
    ChSu2 = process2(argv[1], endTime);
//    if(strcmp(argv[1], "process1"))
//    {
//    }
//    //call process2(server) and send him port
//    if(strcmp(argv[1], "process2"))
//    {
//    }
    //check if the 2 checksums are the same and print according to the orders
    if(ChSu1 == ChSu2)
    {
        //print time
        printf("TCP/IPv4 Socket - Start: %s\n", startTime);
        printf("TCP/IPv4 Socket - End: %s\n", endTime);
    }
    else
    {
        printf("the checksums are not identical, \n -1");
    }
    free(startTime);
    free(endTime);
    return 1;
}