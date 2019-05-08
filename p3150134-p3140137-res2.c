#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "p3150134-p3140137-res2.h"

int * seats , totInc , totW8 , totServ , operator , sCounter , transcount , RandomSeed;
void *Reservation(void *threadId);

pthread_mutex_t Moperator , MtotInc , MtotW8 , MtotServ , Mtrans , Mprint , Mtest;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[]) {
    //THREAD_ARGS threadArgs;
    totInc = 0, transcount = 0, totW8 = 0, totServ = 0, sCounter = 0;
    operator = Ntel;

    if (argc != 3) {
        printf("ERROR: the program should take two arguments, the number of threads to create and the random seed!\n");
        exit(-1);
    }

    char *ptr;
    int Ncust = strtol(argv[1], &ptr, 0);
    RandomSeed = strtol(argv[2], &ptr, 0);

    if (Ncust < 0) {
        printf("ERROR: the Ncust should be a positive number. Current number given %d.\n", Ncust);
        exit(-1);
    }

    printf("Customers: %d, Seed: %d.\n", Ncust, RandomSeed);

    seats = (int *) malloc(sizeof (int) * Nseat);

    if (seats == NULL) {
        printf("NOT ENOUGH MEMORY!\n");
        return -1;
    }

    for (int i = 0; i < Nseat; i++) seats[i] = -1;

    pthread_t *threads = malloc(Ncust * sizeof(pthread_t));

    if (threads == NULL) {
        printf("NOT ENOUGH MEMORY!\n");
        return -1;
    }

    int i;
    int countArray[Ncust];

    for (i = 0; i < Ncust; i++) {
        countArray[i] = i + 1;
        pthread_create(&threads[i], NULL, Reservation, &countArray[i]);
    }

    for (i = 0; i < Ncust; i++) {
        pthread_join(threads[i], NULL);
    }

    for (i= 0; i < Nseat; i++) {
        if(seats[i] == -1) printf("Seat %d is empty\n", i + 1);
        else printf("Seat %d is taken from Customer %d\n", i + 1, *(seats + i));
    }

    printf("Total Income: %d euros.\n", totInc);
    printf("Average waiting time: %f.\n", (double)totW8/(double)Ncust);
    printf("Average service time: %f.\n\n", (double)totServ/(double)Ncust);


    pthread_mutex_destroy(&Moperator);
    pthread_mutex_destroy(&MtotInc);
    pthread_mutex_destroy(&MtotW8);
    pthread_mutex_destroy(&MtotServ);
    pthread_mutex_destroy(&Mtrans);
    pthread_mutex_destroy(&Mprint);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&Mtest);

    free(seats);
    free(threads);
    return 0;
}

void *Reservation(void *threadId) {
    int *thread = (int *)threadId;
    struct timespec start, mid, end;
    time_t start_time , mid_time , end_time;
    clock_gettime(CLOCK_REALTIME, &start);
    start_time = start.tv_sec;

    int n = rand_r(RandomSeed ) * (*thread) * (int) time(NULL);
    unsigned int randSeats = (unsigned int) n % (Nseathigh - Nseatlow + 1) + Nseatlow;
    unsigned int randTime = (unsigned int) n % (tseathigh - tseatlow + 1) + tseatlow;
    int  randSuccess = n % 100;

    pthread_mutex_lock(&Moperator);
    while (operator == 0) pthread_cond_wait(&cond, &Moperator);

    clock_gettime(CLOCK_REALTIME,&mid);
    mid_time = mid.tv_sec;

    operator--;
    pthread_mutex_unlock(&Moperator);


    pthread_mutex_lock(&MtotW8);
    totW8 += mid_time - start_time;
    pthread_mutex_unlock(&MtotW8);

    sleep(randTime);

    clock_gettime(CLOCK_REALTIME,&end);
    end_time = end.tv_sec;


    pthread_mutex_lock(&MtotServ);
    totServ += end_time - start_time;
    pthread_mutex_unlock(&MtotServ);

    pthread_mutex_lock(&Mtest);
    bool flag = false;

    if (sCounter == Nseat){
        printf("The reservation for customer %d could not be accepted because the theater is full.\n", *thread);
        flag = true;
    }else if (sCounter + randSeats > Nseat){
        printf("The reservation for customer %d could not be accepted because there are not available seats.\n", *thread);
        flag = true;
    }else if (randSuccess > Pcardsuccess * 100) {
        printf("The reservation for customer %d could not be accepted because the credit card transaction was not approved.\n",*thread);
        flag = true;
    }

    if(flag){
        pthread_mutex_unlock(&Mtest);
        pthread_mutex_lock(&Moperator);
        operator++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&Moperator);
        pthread_exit(thread);
    }

    for (int i = sCounter; i < sCounter + randSeats ; i++) seats[i] = *thread;

    int temp = sCounter + 1;
    sCounter += randSeats;
    int temp2 = sCounter;

    pthread_mutex_unlock(&Mtest);

    pthread_mutex_lock(&MtotInc);
    totInc += randSeats * Cseat;
    pthread_mutex_unlock(&MtotInc);

    pthread_mutex_lock(&Mtrans);
    int TransId = transcount;
    transcount++;
    pthread_mutex_unlock(&Mtrans);

    pthread_mutex_lock(&Mprint);
    pthread_mutex_lock(&Mtest);
    printf("The reservation was successful for customer %d with id %d, your seats are [%d", *thread , TransId , temp);
    for(int i = temp + 1; i <= temp2; i++) printf(",%d", i);
    printf("] and the transaction cost is %d euros.\n", randSeats * Cseat);
    pthread_mutex_unlock(&Mprint);
    pthread_mutex_unlock(&Mtest);
    pthread_mutex_lock(&Moperator);
    operator++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&Moperator);
    pthread_exit(thread);
}