#include <stdio.h>
#include<stdio.h>
#include<math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
# include <time.h>
#include <string.h>

#define BUFFSIZE (1024*1024*100)
long End_time;
long Start_time;

long ReturnTimeNs() {
    struct timespec currTime;

    if (clock_gettime(CLOCK_REALTIME, &currTime) == -1) {
        perror("clock gettime");
        return EXIT_FAILURE;
    }
    return currTime.tv_nsec;
}


void* threadFunction(char* data)
{
    //checksum for thread 1
    int checksumS, sumS = 0, i;
    for (i = 0; i < BUFFSIZE; i++)
        sumS += data[i];
    checksumS = ~sumS;

    char* dataRead = malloc(BUFFSIZE);
    strcpy(dataRead, data);
    End_time = ReturnTimeNs();
    //checsum for thread 2
    int checksumDifference, sumR = 0, j;
    for (j = 0; j < BUFFSIZE; j++)
        sumR += data[j];
    checksumDifference = ~sumR;

    if (checksumDifference == checksumS) {
        printf("\nshared memory between threads - Start: %ld\\n",Start_time);
        printf("\nshared memory between threads - End: %ld\\n",End_time);
    }
    else {
        printf("\nthe checksums are not identical,\n-1\n");
    }
}

void threadSharingMainFunction() {
    // The code was adapted from https://www.geeksforgeeks.org/multithreading-c-2/
    char *data = malloc(BUFFSIZE);
    int scrFile = 0;
    // let us open the input file
    scrFile = open("file.txt", O_RDONLY);
    if (scrFile > 0) { // there are things to read from the input
        int succe = read(scrFile, data, BUFFSIZE);
        close(scrFile);
    }

    pthread_t thread_id;
    Start_time = ReturnTimeNs();
    pthread_create(&thread_id, NULL, threadFunction, data);
    pthread_join(thread_id, NULL);
}

//int getCheckSum(char * file)
//{
//    int ChSu, sum = 0;
//    for (int i = 0; i < strlen(file); i++)
//        sum += file[i];
//    ChSu=~sum;    //1's complement of sum
//    return ChSu;
//}

int main(int argc, char * argv[]){
    threadSharingMainFunction();

}