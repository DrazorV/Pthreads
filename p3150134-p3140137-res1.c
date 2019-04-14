#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "p3150134-p3140137-res1.h"


int * seats , totInc , totW8 , totServ , operator , sCounter , transcount;
char *ptr;
void Output(int customer) ;
void *Reservation(void *threadId);

pthread_mutex_t lock , MtotInc , MtotW8 , MtotServ , Mplan , Mcounter , Mpr;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("ERROR: the program should take two arguments, the number of threads to create and the random seed!\n");
        exit(-1);
    }

    int Ncust = strtol(argv[1], &ptr, 0);
    int RandomSeed = strtol(argv[2], &ptr, 0);

    if (Ncust < 0) {
        printf("ERROR: the Ncust should be a positive number. Current number given %d.\n", Ncust);
        exit(-1);
    }

    printf("Customers: %d, Seed: %d.\n", Ncust, RandomSeed);

    srand((unsigned int) RandomSeed);

    seats = (int *) malloc(sizeof (int) * Nseat);

    if (seats == NULL) {
        printf("NOT ENOUGH MEMORY!\n");
        return -1;
    }

    for (int i = 0; i < Nseat; i++) seats[i] = -1;

    pthread_t *threads;
    threads = malloc(Ncust * sizeof(pthread_t));

    if (threads == NULL) {
        printf("NOT ENOUGH MEMORY!\n");
        return -1;
    }

    int rc;
    int threadCount;
    int countArray[Ncust];
    for (threadCount = 0; threadCount < Ncust; threadCount++) {
        printf("Main: creating thread %d\n", threadCount + 1);
        countArray[threadCount] = threadCount + 1;
        rc = pthread_create(&threads[threadCount], NULL, Reservation, &countArray[threadCount]);

        if (rc != 0) {
            printf("ERROR: return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    void *status;
    for (threadCount = 0; threadCount < Ncust; threadCount++) {
        rc = pthread_join(threads[threadCount], &status);

        if (rc != 0) {
            printf("ERROR: return code from pthread_join() is %d\n", rc);
            exit(-1);
        }

        printf("Main: Thread %d returned %d as status code.\n", countArray[threadCount], (*(int *) status));
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    Output(Ncust);

    free(seats);
    free(threads);
    return 1;
}

void Output(int cust){
    for (int i = 0; i < Nseat; i++) {
        if(seats[i]==-1)printf("Seat %d / Empty\n", i+1);
        else printf("Seat %d / Customer %d\n", i+1, *(seats + i));
    }

    printf("\nTotal Income: %d euros.\n", totInc);
    printf("\nAverage waiting time: %f.\n", (double)totW8/(double)cust);
    printf("\nAverage service time: %f.\n\n", (double)totServ/(double)cust);
}

void *Reservation(void *threadId) {
    unsigned int randSeats = (unsigned int) rand() % (Nseathigh - Nseatlow + 1) + Nseatlow;
    unsigned int randTime = (unsigned int) (rand() % (tseathigh - tseatlow + 1) + tseatlow);
    unsigned int randSuccess = (unsigned int) rand() % 100;

    struct timespec start, mid, end;
    time_t start_time , mid_time , end_time;
    clock_gettime(CLOCK_REALTIME, &start);
    start_time = start.tv_sec;

    int *tid = threadId;

    int rc = pthread_mutex_lock(&lock);

    while (operator == 0) rc = pthread_cond_wait(&cond, &lock);

    clock_gettime(CLOCK_REALTIME,&mid);
    mid_time = mid.tv_sec;
    operator--;

    rc = pthread_mutex_unlock(&lock);
    rc = pthread_mutex_lock(&MtotW8);

    totW8+= (mid_time-start_time);

    rc = pthread_mutex_unlock(&MtotW8);

    sleep(randTime);
    clock_gettime(CLOCK_REALTIME,&end);
    end_time = end.tv_sec;

    rc = pthread_mutex_lock(&MtotServ);
    totServ += (end_time-start_time);
    rc = pthread_mutex_unlock(&MtotServ);
    rc = pthread_mutex_lock(&Mplan);

    bool flag = false;

    if (sCounter == Nseat){
        printf("The reservation for customer: %d could not be accepted because the theater is full.\n", *tid);
        flag = true;
    }else if (sCounter + randSeats > Nseat){
        printf("The reservation for customer: %d could not be accepted because there are not available seats.\n", *tid);
        flag = true;
    }else if (randSuccess > Pcardsuccess - 1){
        printf("The reservation for customer: %d could not be accepted because the credit card transaction was not approved.\n", *tid);
        flag = true;
    }

    if(flag){
        rc = pthread_mutex_unlock(&Mplan);
        rc = pthread_mutex_lock(&lock);
        operator++;
        rc = pthread_cond_signal(&cond);
        rc = pthread_mutex_unlock(&lock);
        pthread_exit(tid);
    }

    for (int i = sCounter; i < sCounter + randSeats ; i++) seats[i]=*tid;
    int temp = sCounter + 1;
    sCounter += randSeats;
    int temp2 = sCounter;
    rc = pthread_mutex_unlock(&Mplan);

    rc = pthread_mutex_lock(&MtotInc);
    totInc += randSeats*Cseat;
    rc = pthread_mutex_unlock(&MtotInc);

    rc = pthread_mutex_lock(&Mcounter);
    int count = transcount;
    transcount++;
    rc = pthread_mutex_unlock(&Mcounter);

    rc = pthread_mutex_lock(&Mpr);

    printf("Πελάτης %d:\nΗ κράτηση ολοκληρώθηκε επιτυχώς.\nΟ αριθμός συναλλαγής είναι <%d>", *tid, count);
    printf(", οι θέσεις σας είναι οι <%d", temp);
    for(int i = temp + 1; i <= temp2; i++) printf(", %d",i);

    printf("> και το κόστος της συναλλαγής είναι <%d> ευρώ.\n", randSeats * Cseat);
    rc = pthread_mutex_unlock(&Mpr);


    rc = pthread_mutex_lock(&lock);

    operator++;
    rc = pthread_cond_signal(&cond);
    rc = pthread_mutex_unlock(&lock);

    pthread_exit(tid);
}