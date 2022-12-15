#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#define CHUNK_SIZE 1024
#define SIZE_IN_BYTES 104857600
#define FILENAME "file.txt"
#define RECEIVER_PORT 2001

long Etime;
long Stime;


// The function was adapted from: https://www.educba.com/clock_gettime/
long ReturnTimeNs() {
    struct timespec currTime;

    if (clock_gettime(CLOCK_REALTIME, &currTime) == -1) {
        perror("clock gettime");
        return EXIT_FAILURE;
    }
    return currTime.tv_nsec;
}

int getCheckSum(char * file)
{
    int ChSu = 0, sum = 0;
    for (int i = 0; i < strlen(file); i++)
        sum += file[i];
    ChSu=~sum;    //1's complement of sum
    return ChSu;
}

//void send_file(FILE * fp, int sockfd, struct sockaddr_in6 servaddr){
//    // Loop until the end of the file
//    while (!feof(fp)) {
//        // Read a chunk of data from the file
//        char buffer[1024];
//        size_t bytes_read = fread(buffer, 1, 1024, fp);
//
//        // Send the chunk of data to the server
//        //sendto(sockfd, buffer, bytes_read, 0);
//
//        sendto(sockfd, buffer, bytes_read,
//               0, (const struct sockaddr *) &servaddr,
//               sizeof(servaddr));
//
//    }
//    // Close the file
//    fclose(fp);
//}

////client
//int process1(char * portNum, char * ipAddr, FILE * fp)
//{
//    int sockfd;
////    char buffer[SIZE_IN_BYTES];
//    struct sockaddr_in6 servaddr;
//    int port = atoi(portNum);//convert the string of port from user to int
//
//    // Creating socket file descriptor
//    if ( (sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0 ) {
//        perror("socket creation failed");
//        exit(EXIT_FAILURE);
//    }
//
//    memset(&servaddr, 0, sizeof(servaddr));
//
//    // Filling server information
//    servaddr.sin6_family = AF_INET6;
//    servaddr.sin6_port = htons(port);
////    servaddr.sin6_addr= inet_addr(ipAddr);
//    memcpy((char *)&servaddr.sin6_addr, ipAddr, strlen(ipAddr));
//    //servaddr.sin6_addr.s6_addr = inet_addr(ipAddr);
//    // Set up the receiver's address
//    inet_pton(AF_INET6,  "::1", &servaddr.sin6_addr);
//
//
//
//    // int n, len;
//    Stime = ReturnTimeNs();//getTime();
//    send_file(fp, sockfd, servaddr);
//    printf("file sent.\n");
//    close(sockfd);
//    printf("\n");
//    return 0;
//
//}
//
////server
//int process2(char * portNum, FILE * fp)
//{
//    int port = atoi(portNum);//convert the string of port from user to int
//    int socket_desc, client_sock, client_address_size;
//    struct sockaddr_in6 server_addr, client_addr;
//    char * client_message;
//
//
//    client_message = malloc(SIZE_IN_BYTES);
//
//    // Create socket:
//    socket_desc = socket(AF_INET6, SOCK_DGRAM, 0);
//
//    if(socket_desc < 0){
//        printf("Error while creating socket\n");
//        return -1;
//    }
//    printf("Socket created successfully\n");
//
//    // Set port and IP:
//    server_addr.sin6_family = AF_INET6;
//    server_addr.sin6_port = htons(port);
//    server_addr.sin6_addr= in6addr_any;//inet_addr("0.0.0.0");
//
//    // Bind to the set port and IP:
//    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
//        printf("Couldn't bind to the port\n");
//        return -1;
//    }
//    printf("Done with binding\n");
//    client_address_size = sizeof(client_addr);
//
//
//    // Accept an incoming connection:
//    client_address_size = sizeof(client_addr);
//
//    char * fullMassage = malloc(SIZE_IN_BYTES);
//    //int fd = open("recivedFileUDP.txt", O_WRONLY | O_APPEND | O_CREAT);
//    long i = 0, lenGot;
//    while(i != SIZE_IN_BYTES){
//        // Receive client's message:
//        if((lenGot = recvfrom(socket_desc, client_message, 1024, 0, (struct sockaddr *) &client_addr,
//                    &client_address_size)) <0)
//        {
//            perror("recvfrom()");
//            exit(4);
//        }
//        strcat(fullMassage, client_message);
////        write(fd, client_message, SIZE_IN_BYTES);
//        i += lenGot;
//    }
//
//
//    Etime = ReturnTimeNs();//getTime();
//
//    //get checksum for process1
//    int ch1;
//    char * buff = malloc(SIZE_IN_BYTES);
//    fread(buff, 1, SIZE_IN_BYTES, fp);
//    ch1 = getCheckSum(buff);
//    free(buff);
//
//    printf("ch1 = %d\n", ch1);
//    //get checksum for process2
//    int ch2 = getCheckSum(fullMassage);
//    printf("ch2 = %d\n", ch2);
////    char * fileRecived = malloc(SIZE_IN_BYTES);
////    read(fd, fileRecived, SIZE_IN_BYTES);
////    int ch2 = getCheckSum(fileRecived);
////    free(fileRecived);
//
//    free(client_message);
////    free(fullMassage);
//    close(socket_desc);
//    if(ch1 == ch2)
//    {
//        return 1;
//    }
//    else
//        return 0;
//}

int process1(char * ipAddr)
{
    // sender

    // Open a UDP socket
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set up the receiver's address
    struct sockaddr_in6 receiver_addr;
//    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin6_family = AF_INET6;
    receiver_addr.sin6_port = htons(RECEIVER_PORT);
    inet_pton(AF_INET6, ipAddr, &receiver_addr.sin6_addr);

    // Open the file to be sent
    int fd = open(FILENAME, O_RDONLY);
    if (fd < 0) {
        perror("Couldn't open file\n");
        exit(1);
    }

    // Read the file into a buffer
    char *buffer = malloc(CHUNK_SIZE);
    if (!buffer) {
        perror("Couldn't allocate memmory\n");
        exit(1);
    }

    Stime = ReturnTimeNs();//getTime();
    ssize_t bytes_read;
    ssize_t bytesTot = 0;
    // Send the file in chunks
    while (1) {

        // Read a chunk from the file
        bytes_read = read(fd, buffer, CHUNK_SIZE);

        // If we reached the end of the file, break out of the loop
        if (bytes_read == 0) {
            break;
        }

        // Send the chunk to the receiver
        ssize_t bytes_sent = sendto(sock, buffer, CHUNK_SIZE, 0,
                                    (struct sockaddr*)&receiver_addr, sizeof(receiver_addr));

        if (bytes_sent < 0) {
            perror("Unable to send\n");
            exit(1);
        }
        bytesTot += bytes_sent;
    }
    printf("bytes: %d\n", (int)bytesTot);
    printf("file sent.\n");

    // Close the socket and file
    close(sock);
    close(fd);
}

int process2()
{
    // receiver

    // Open a UDP socket
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");


// Bind the socket to the local port
    struct sockaddr_in6 local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin6_family = AF_INET6;
    local_addr.sin6_port = htons(RECEIVER_PORT);
    local_addr.sin6_addr = in6addr_any;

    if (bind(sock, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");



    char * fullMassage = malloc(SIZE_IN_BYTES);
    ssize_t totbytes = 0;
    // Receive the file in chunks
    while (1) {
        // Receive a chunk from the sender
        char buffer[CHUNK_SIZE];
        struct sockaddr_in6 sender_addr;
        socklen_t addrlen = sizeof(sender_addr);
        ssize_t bytes_received = recvfrom(sock, buffer, CHUNK_SIZE, 0,
                                          (struct sockaddr*)&sender_addr, &addrlen);

        if (bytes_received < 0) {
            perror("recvfrom");
            exit(1);
        }

        totbytes += bytes_received;
        // If we received 0 bytes, it means the sender has finished sending
        // the file and we can break out of the loop
        if (totbytes == SIZE_IN_BYTES) {
            break;
        }

        // Write the received chunk to the file
//        size_t bytes_written = fwrite(buffer, 1, bytes_received, fp);
//        if (bytes_written < 0) {
//            perror("fwrite");
//            exit(1);
//        }
        strcat(fullMassage, buffer);
    }

    printf("bytes recived: %d\n", (int)totbytes);

    printf("file recived\n");
    //take time
    Etime = ReturnTimeNs();//getTime();

    //get checksum for process2
    int ch2 = getCheckSum(fullMassage);
    printf("ch2 = %d\n", ch2);
    free(fullMassage);

// Close the socket and file
    close(sock);
    return ch2;
}

int main(int argc, char * argv[])
{
    int ch1, ch2;
    //get checksum for process1
    //open the file to be received
    int fd = open(FILENAME, O_RDONLY);
    if (fd < 0) {
        perror("Couldn't open file\n");
        exit(1);
    }

    char * buff = malloc(SIZE_IN_BYTES);
    read(fd, buff, SIZE_IN_BYTES);
    ch1 = getCheckSum(buff);
    free(buff);

    if(argc == 2)
    {
        process1(argv[1]);
    }
    if(argc == 1)
    {
        ch2 = process2();
    }
    //check if the 2 checksums are the same and print according to the orders
    printf("ch1 = %d\n", ch1);
    printf("ch2 = %d\n", ch2);

    if(ch1 == ch2)
    {
        //print time
        printf("UDP/IPv6 Socket - Start: %ld\n", Stime);
        printf("UDP/IPv6 Socket - End: %ld\n", Etime);
    }
    else
    {
        printf("the checksums are not identical, \n -1\n");
    }
    return 1;

}