#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

sem_t* start_crossing_sem;
sem_t* signal_start_sem;

void* child(void*);
void* adult(void*);
void initSynch();

int main(int argc, char const* argv[]) {
  if (argc < 3) {
    printf("\nmust include num number of children and adults\n\n");
    exit(1);
  }

  // set # to create of each atom (atoi converts a string to an int)
  const int numchildren = atoi(argv[1]);
  const int numadults = atoi(argv[2]);
  const int total = numchildren + numadults;

  initSynch();

  // Create threads for each person
  pthread_t people[total];
  for (int i = 0; i < numchildren; i++) {
    pthread_create(&people[i], NULL, child, NULL);
  }
  for (int i = numchildren; i < total; i++) {
    pthread_create(&people[i], NULL, adult, NULL);
  }

  // Somehow wait until all threads have started and then notify threads they
  // can start crossing
  for (size_t j = 0; j < total; j++) {
    sem_wait(start_crossing_sem);
    printf("Person reported ready.\n");
    fflush(stdout);
  }
  sem_post(signal_start_sem);

  //  Wait until everyone has crossed before exiting
  // ^ Use semaphore

  return 0;
}

void* child(void* x) {
  printf("Child – reporting in.\n");
  fflush(stdout);
  sem_post(start_crossing_sem);

  sem_wait(signal_start_sem);
  sem_post(signal_start_sem);
  printf("Child – arrived on Oahu.\n");
  fflush(stdout);

  return (void*)0;
}

void* adult(void* x) {
  printf("Adult – reporting in.\n");
  fflush(stdout);
  sem_post(start_crossing_sem);

  sem_wait(signal_start_sem);
  sem_post(signal_start_sem);
  printf("Adult – arrived on Oahu.\n");
  fflush(stdout);

  return (void*)0;
}

void initSynch() {
  start_crossing_sem = sem_open("startsem", O_CREAT | O_EXCL, 0466, 0);
  while (start_crossing_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore startsem already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("startsem");
      start_crossing_sem = sem_open("startsem", O_CREAT | O_EXCL, 0466, 0);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  signal_start_sem = sem_open("signalsem", O_CREAT | O_EXCL, 0466, 0);
  while (signal_start_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore signalsem already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("signalsem");
      signal_start_sem = sem_open("signalsem", O_CREAT | O_EXCL, 0466, 0);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }
}
