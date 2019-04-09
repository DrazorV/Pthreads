#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


void *PrintHello(void *threadId) ;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("ERROR: the program should take one argument, the number of threads to create!\n");
        exit(-1);
    }

    int maxNumberOfThreads = atoi(argv[1]);

    /*elegxoume oti to plithos twn threads pou edwse o xristis einai thetiko
    arithmos.*/
    if (maxNumberOfThreads < 0) {
        printf("ERROR: the number of threads to run should be a positive number. Current number given %d.\n",maxNumberOfThreads);
        exit(-1);
    }

    printf("Main: We will create %d threads that will print hello world.\n", maxNumberOfThreads);

    pthread_t *threads;

    threads = malloc(maxNumberOfThreads * sizeof(pthread_t));
    if (threads == NULL) {
        printf("NOT ENOUGH MEMORY!\n");
        return -1;
    }

    int rc;
    int threadCount;
    int countArray[maxNumberOfThreads];
    for (threadCount = 0; threadCount < maxNumberOfThreads; threadCount++) {
        printf("Main: creating thread %d\n", threadCount);
        countArray[threadCount] = threadCount + 1;
        /*dimiourgia tou thread*/
        rc = pthread_create(&threads[threadCount], NULL, PrintHello, &countArray[threadCount]);

        /*elegxos oti to thread dimiourgithike swsta.*/
        if (rc != 0) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    void *status;
    /*Aparaitito gia na stamatisei to thread, an den to orisete yparxei
    periptwsi na teleiwsei o pateras prin ta threads kai ara na min exoume
    to epithymito apotelesma*/
    for (threadCount = 0; threadCount < maxNumberOfThreads; threadCount++) {
        rc = pthread_join(threads[threadCount], &status);

        if (rc != 0) {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

        printf("Main: Thread %d returned %d as status code.\n", countArray[threadCount], (*(int *) status));
    }

    free(threads);
    return 1;
}



void *PrintHello(void *threadId) {
    int *tid;
    tid = (int *)threadId;
    printf("Thread: Hello World from thread %d!\n", *tid);
    /*aparaitito gia na gnwrizei o pateras oti to thread termatise swsta,
    douleuei swsta kai an kanete return kapoia timi.*/
    pthread_exit(tid);
}