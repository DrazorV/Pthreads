#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "p3150134-p3140137-res2.h"

int * seats , totInc , totW8 , totW82, totServ , totServ2, operator, cashier , sCounter , RandomSeed, MaxSeats = (NzoneA + NzoneB + NzoneC)*Nseat;
void *Reservation(void *threadId);
void *Transaction(void *arguments);

pthread_mutex_t Moperator , MtotInc , MtotW8 , MtotServ , Mtrans , Mprint , Mtest;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t Mcashier, MtotW82, MtotServ2, Mtest2;

typedef struct arg_struct {
    char zone;
    int seats;
    int cseats;
    int transID;
    unsigned long rand;
} Args;

int main(int argc, char *argv[]) {
    //THREAD_ARGS threadArgs;
    totInc = 0, totW8 = 0,totW82 = 0, totServ = 0,totServ2 = 0, sCounter = 0;
    operator = Ntel;
    cashier = Ncash;

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

    seats = (int *) malloc(sizeof (int) * MaxSeats);

    if (seats == NULL) {
        printf("NOT ENOUGH MEMORY!\n");
        return -1;
    }

    for (int i = 0; i < MaxSeats; i++) seats[i] = -1;

    int i;
    int countArray[Ncust];
    pthread_t *threads = malloc(Ncust * sizeof(pthread_t));
    pthread_t *threads2 = malloc(Ncust * sizeof(pthread_t));

    if (threads == NULL) {
        printf("NOT ENOUGH MEMORY!\n");
        return -1;
    }

    for (i = 0; i < Ncust; i++) {
        countArray[i] = i + 1;
        pthread_create(&threads[i], NULL, Reservation, &countArray[i]);
    }

    Args *res[Ncust];
    for (i = 0; i < Ncust; i++) {
        pthread_join(threads[i], (void **) &res[i]);
        pthread_create(&threads2[i], NULL, Transaction, res[i]);
    }

    for (i = 0; i < Ncust; i++)pthread_join(threads2[i], (void **) res[i]);


    for (i = 0; i < NzoneA * 10; i++) {
        if(seats[i] == -1) printf("Seat %d is empty\n", i + 1);
        else printf("Zone %c / Seat %d / Customer %d\n", 'A', i + 1, seats[i]);
    }
    for (i = NzoneA * 10 ; i < (NzoneB + NzoneA) * 10; i++) {
        if(seats[i] == -1) printf("Seat %d is empty\n", i + 1);
        else printf("Zone %c / Seat %d / Customer %d\n", 'B', i + 1, seats[i]);
    }
    for (i = (NzoneB + NzoneA) * 10; i < (NzoneB + NzoneA + NzoneC) * 10; i++) {
        if(seats[i] == -1) printf("Seat %d is empty\n", i + 1);
        else printf("Zone %c / Seat %d / Customer %d\n", 'C', i + 1, seats[i]);
    }

    printf("Total Income: %d euros.\n", totInc);
    printf("Average service time: %f seconds.\n", ((double) totServ + (double) totServ2) /(double)Ncust);
    printf("Average waiting operator time: %f seconds.\n", (double)totW8/(double)Ncust);
    printf("Average waiting cashier time: %f seconds.\n", (double)totW82/(double)Ncust);

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

    unsigned int n = (unsigned int) (RandomSeed * (*thread) * (int) time(NULL));
    rand_r(&n);
    unsigned int randSeats = n % (Nseathigh - Nseatlow + 1) + Nseatlow;
    unsigned int randTime = n % (tseathigh - tseatlow + 1) + tseatlow;
    int randZone = n % 100;

    pthread_mutex_lock(&Moperator);
    while (operator == 0) pthread_cond_wait(&cond, &Moperator);

    clock_gettime(CLOCK_REALTIME,&mid);
    mid_time = mid.tv_sec;

    operator--;
    pthread_mutex_unlock(&Moperator);

    int Cseat;
    char zone;
    if(randZone <= PzoneA * 100){
        Cseat = CzoneA;
        zone = 'A';
    }else if (randZone <= (PzoneB + PzoneA) * 100){
        Cseat = CzoneB;
        zone = 'B';
    }else{
        Cseat = CzoneC;
        zone = 'C';
    }

    pthread_mutex_lock(&MtotW8);
    totW8 += mid_time - start_time;
    pthread_mutex_unlock(&MtotW8);

    sleep(randTime);

    clock_gettime(CLOCK_REALTIME,&end);
    end_time = end.tv_sec;

    pthread_mutex_lock(&MtotServ);
    totServ += end_time - start_time;
    pthread_mutex_unlock(&MtotServ);

    pthread_mutex_lock(&Moperator);
    operator++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&Moperator);
    Args *args = malloc(sizeof(Args));
    args->zone = zone;
    args->seats = randSeats;
    args->transID = *thread;
    args->cseats = Cseat;
    args->rand = n;
    pthread_exit(args);
}

void *Transaction(void *arguments) {
    Args *args = arguments;
    struct timespec start, mid, end;
    time_t start_time, mid_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start);
    start_time = start.tv_sec;

    unsigned int randTime = args->rand % (tCashHigh - tCashLow + 1) + tCashLow;
    int randSuccess = args->rand % 100;

    pthread_mutex_lock(&Mcashier);

    while (cashier == 0) pthread_cond_wait(&cond, &Mcashier);

    clock_gettime(CLOCK_REALTIME, &mid);
    mid_time = mid.tv_sec;

    cashier--;
    pthread_mutex_unlock(&Mcashier);

    pthread_mutex_lock(&MtotW82);
    totW82 += mid_time - start_time;
    pthread_mutex_unlock(&MtotW82);

    sleep(randTime);

    clock_gettime(CLOCK_REALTIME, &end);
    end_time = end.tv_sec;


    pthread_mutex_lock(&MtotServ2);
    totServ2 += end_time - start_time;
    pthread_mutex_unlock(&MtotServ2);

    pthread_mutex_lock(&Mtest2);

    bool flag = false;

    if (randSuccess > Pcardsuccess * 100) {
        printf("The reservation for customer %d could not be accepted because the credit card transaction was not approved.\n",args->transID);
        flag = true;
    }

    if (flag) {
        pthread_mutex_unlock(&Mtest2);
        pthread_mutex_lock(&Mcashier);
        cashier++;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&Mcashier);
        pthread_exit(NULL);
    }
    int temp3 = 0;
    int temp4 = 0;
    switch (args->zone) {
        case 'A' :
            temp3 = 0;
            temp4 = NzoneA * 10 - 1;
            break;
        case 'B' :
            temp3 = NzoneA * 10;
            temp4 = (NzoneA + NzoneB) * 10 - 1;
            break;
        case 'C' :
            temp3 = (NzoneA + NzoneB) * 10;
            temp4 = (NzoneA + NzoneB + NzoneC) * 10 - 1;
            break;
    }
    int counter = 0;
    for(int k = temp3; k < temp4; k++){
        if(seats[k] == -1){
            counter++;
            if (counter == args->seats) {
                if ((k / 10) % 10 == ((k - args->seats + 1) / 10) % 10){
                    printf("The reservation was successful for customer %d with seats [", args->transID);
                    for (int l = k; l > k - args->seats; l--) {
                        printf(" (%d) ", l);
                        seats[l] = args->transID;
                    }
                    printf("] and the transaction cost is %d euros.\n", args->seats * args->cseats);
                    pthread_mutex_lock(&MtotInc);
                    totInc += args->seats * args->cseats;
                    pthread_mutex_unlock(&MtotInc);
                    break;
                } else {
                    k = (k / 10) * 10 - 1;
                    counter = 0;
                }
            }
        }else counter = 0;
        if (k == temp4 - 1) printf("The reservation for customer %d could not be accepted because Zone%c doesn't have more seats.\n",args->transID,args->zone);
    }

    pthread_mutex_unlock(&Mtest2);
    pthread_mutex_lock(&Mcashier);
    cashier++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&Mcashier);
    pthread_exit(NULL);
}