#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

sem_t* start_crossing_sem;
sem_t* signal_start_sem;

pthread_cond_t oneChild;
pthread_cond_t boat_available_O;
pthread_cond_t boat_available_M;
pthread_cond_t boat_full_O;

pthread_mutex_t adult_lock;
pthread_mutex_t child_lock;

int boat = 0;           // 0 for empty, 1 for one child, 2 for full
int boat_location = 0;  // 0 for Oahu, 1 for Molokai
int children_O;
int children_M;
int adults_O;
int adults;

void* child(void*);
void* adult(void*);
void initSynch();

int main(int argc, char const* argv[]) {
  if (argc < 3) {
    printf("\nmust include num number of Children and adults\n\n");
    exit(1);
  }

  // set # to create of each atom (atoi converts a string to an int)
  const int numchildren = atoi(argv[1]);
  const int numadults = atoi(argv[2]);
  const int total = numchildren + numadults;

  initSynch();

  // for testing
  adults = numadults;

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
  }
  printf("Everyone reported ready, signaling to start.\n");
  fflush(stdout);
  sem_post(signal_start_sem);

  //  Wait until everyone has crossed before exiting
  // ^ Use semaphore

  // join all threads before letting main exit
  for (int i = 0; i < total; i++) {
    pthread_join(people[i], NULL);
  }

  return 0;
}

void* child(void* x) {
  int id = children_O++;

  printf("Child #%d – reporting in.\n", id);
  fflush(stdout);
  sem_post(start_crossing_sem);

  sem_wait(signal_start_sem);
  sem_post(signal_start_sem);
  printf("Child #%d – showing up on Oahu.\n", id);
  fflush(stdout);

  int island = 0;  // 0 for Oahu, 1 for Molokai

  sleep(1);
  printf("\n\n");

  while (children_O + adults_O > 0) {

    pthread_mutex_lock(&child_lock);
    while (island == 0) {
      if (boat_location == 0 && boat == 0) {
        if (children_O > 1) {
          boat++;
          printf("Child #%d – First one on the boat! Waiting until full...\n",
                 id);
          fflush(stdout);
          pthread_cond_wait(&boat_full_O, &child_lock);
          printf(
              "\nOAHU _._._._._._ --> \\__C%d__/ --> _._._._._._ MOLOKAI\n\n",
              id);
          printf("Child #%d – getting off boat.\n", id);
          fflush(stdout);
          island = 1;
          children_O--;
          children_M++;
        } else {
          printf("Child #%d – Signal Adult to get on boat.\n", id);
          fflush(stdout);
          pthread_cond_signal(&boat_available_O);
          printf("Child #%d – waiting to be picked up...\n", id);
          fflush(stdout);
          pthread_cond_wait(&boat_available_M, &child_lock);
          pthread_cond_wait(&boat_available_O, &child_lock);
          printf("Child #%d – picked up?\n", id);
          fflush(stdout);

          pthread_mutex_unlock(&child_lock);
        }
      } else if (boat_location == 0 && boat == 1) {
        printf("Child #%d – Last one on the boat! Broadcast full.\n", id);
        fflush(stdout);
        boat++;
        pthread_cond_broadcast(&boat_full_O);
        printf("\nOAHU _._._._._._ --> \\__C%d__/ --> _._._._._._ MOLOKAI\n\n",
               id);
        printf("Child #%d – getting off boat.\n", id);
        fflush(stdout);
        boat = 0;
        boat_location = 1;
        island = 1;
        children_O--;
        children_M++;
      } else {
        printf("Child #%d – ????.\n", id);
        fflush(stdout);
        pthread_cond_wait(&boat_available_O, &child_lock);
      }
    }
    pthread_mutex_unlock(&child_lock);

    pthread_mutex_lock(&child_lock);
    while (island == 1) {
      if (boat_location == 1 && boat == 0) {
        printf("Child #%d – telling everyone the boat is empty.\n", id);
        fflush(stdout);
        pthread_cond_broadcast(&boat_available_M);
      }
      printf("Child #%d – waiting to go back...\n", id);
      fflush(stdout);
      pthread_cond_wait(&boat_available_M, &child_lock);
      printf("Child #%d – Getting back on boat!\n", id);
      printf("\nOAHU _._._._._._ <-- \\__C%d__/ <-- _._._._._._ MOLOKAI\n\n",
             id);
      printf("Child #%d – getting off boat.\n", id);
      fflush(stdout);
      island = 0;
      boat_location = 0;
      children_M--;
      children_O++;
      // pthread_cond_signal(&boat_available_O);
    }
    pthread_mutex_unlock(&child_lock);
  }

  return (void*)0;
}

void* adult(void* x) {
  printf("Adult – reporting in.\n");
  fflush(stdout);
  sem_post(start_crossing_sem);

  sem_wait(signal_start_sem);
  sem_post(signal_start_sem);
  printf("Adult – showing up on Oahu.\n");
  fflush(stdout);

  int island = 0;  // 0 for Oahu, 1 for Molokai
  adults_O++;

  pthread_mutex_lock(&adult_lock);
  while (island == 0) {
    printf("Adult – Waiting for boat!\n");
    fflush(stdout);
    pthread_cond_wait(&boat_available_O, &adult_lock);

    printf("Adult – Getting on boat!\n");
    fflush(stdout);
    boat += 2;
    pthread_cond_broadcast(&boat_full_O);

    printf("\nOAHU _._._._._._ --> \\__A__/ --> _._._._._._ MOLOKAI\n\n");
    fflush(stdout);
    island = 1;
    boat -= 2;
    boat_location = 1;
    adults_O--;
  }
  pthread_mutex_unlock(&adult_lock);

  printf("Adult #%d – getting off boat.\n", adults - adults_O);
  printf("Adult #%d – Signal child to get on boat. #chill\n",
         adults - adults_O);
  fflush(stdout);
  pthread_cond_signal(&boat_available_M);
  pthread_cond_broadcast(&boat_available_M);
  return (void*)0;
}

void initSynch() {

  pthread_cond_init(&oneChild, NULL);
  pthread_cond_init(&boat_available_O, NULL);
  pthread_cond_init(&boat_full_O, NULL);
  pthread_cond_init(&boat_available_M, NULL);

  pthread_mutex_init(&adult_lock, NULL);
  pthread_mutex_init(&child_lock, NULL);

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
