#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define SIZE_IN_BYTES 104857600
#define CHUNK_SIZE 1024
long Endtime;
long Starttime;

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
    int ChSu, sum = 0;
    for (int i = 0; i < strlen(file); i++)
        sum += file[i];
    ChSu=~sum;    //1's complement of sum
    return ChSu;
}

int main() {
    //open the file
    int fd = open("file.txt", O_RDONLY);
    if (fd == -1) {
        perror("Error opening input file");
        return 1;
    }
    int ch1, ch2;
    //process 1 checksum
    char *buff = malloc(SIZE_IN_BYTES);
    read(fd,buff, SIZE_IN_BYTES);
    ch1 = getCheckSum(buff);
    free(buff);    //check if the 2 checksums are the same and print according to the orders

    // Create the pipe
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        // Error creating pipe
        printf("Can't create a pipe");
        return 1;
    }

    // pipefd[0] is the read end of the pipe
    // pipefd[1] is the write end of the pipe
    int pid = fork();
    if (pid == -1)
        return 2;

    if (pid == 0) {
        //we are in child process

        int sum = 0, i;
        char buffer[CHUNK_SIZE + 1];
        int bytes_read;
        int sumChunksize = 0;
        while (sumChunksize != SIZE_IN_BYTES) {
            if ((bytes_read = read(pipefd[0], &buffer, CHUNK_SIZE)) == -1) {
                return 3;
            }
            sumChunksize += bytes_read;
            i = 0;
            buffer[CHUNK_SIZE + 1] = '\0';
            while (buffer[i] != '\0') {
                sum += buffer[i++];
            }
        }
        printf("Recived file\n");

        //here we will check the time
        Endtime = ReturnTimeNs();

        //we will checksum for process 1 and 2
        //process 2 checksum
        ch2 = ~sum;
        if (ch1 == ch2) {
            //print time
            printf("PIPE Socket - Start: %ld\n", Starttime);
            printf("PIPE Socket - End: %ld\n", Endtime);
        } else {
            printf("the checksums are not identical, \n -1\n");
        }
    }

    else {
        //we are in parent process aka process 1
        //check time
        Starttime = ReturnTimeNs();
        //we will write the file by chunks to the child process
        char buffer[CHUNK_SIZE];
        ssize_t bytes_read;
        while ((bytes_read = read(fd, buffer, CHUNK_SIZE)) > 0) {
            if (write(pipefd[1], buffer, bytes_read) != bytes_read) {
                perror("Error writing to pipe");
                return 1;
            }
        }
        if (bytes_read == -1) {
            perror("Error reading input file");
            return 1;
        }
        //wait(NULL);
    }

    close(pipefd[0]);
    close(pipefd[1]);
}