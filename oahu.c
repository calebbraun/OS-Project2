/*
Caleb Braun and Reilly Hallstrom
Project 2
4/27/16

A program using threads, semaphores, mutexes, and condition
variables to imitate the classic one-boat problem.  The restrictions for our
problem are that only one adult, one child, or two children are allowed in
the boat at a time.  All people must cross from Oahu to Molokai.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

// Method delcarations
void* child(void*);
void* adult(void*);
void initSynch();

// Semaphores used by main method to signal start and end of threads
sem_t* crossing_signal_sem;
sem_t* signal_start_sem;

// Condition variables for the threads
pthread_cond_t signal_adult_embark;
pthread_cond_t boat_available_O;
pthread_cond_t boat_available_M;
pthread_cond_t boat_full_O;

// Locks for the global variables
pthread_mutex_t boat_lock;
pthread_mutex_t child_lock;
pthread_mutex_t adult_lock;

// Last thread to cross signals this bool to allow all threads to exit
int everyoneAcross = 0;

int boat = 0;           // 0 for empty, 1 for one child, 2 for full
int boat_location = 0;  // 0 for Oahu, 1 for Molokai
int children_O;
int adults_O;

int main(int argc, char const* argv[]) {
  // Error check number of arguments
  if (argc < 3) {
    printf("\nYou must include both the number of children and adults.\n\n");
    exit(1);
  }

  const int numchildren = atoi(argv[1]);
  const int numadults = atoi(argv[2]);
  const int total = numchildren + numadults;

  // Error check if there are enough children to solve problem
  if (numchildren < 2) {
    printf("\nThere must be at least two children.\n\n");
    exit(1);
  }

  initSynch();

  // Create threads for each person
  pthread_t people[total];
  for (int i = 0; i < numchildren; i++) {
    pthread_create(&people[i], NULL, child, NULL);
  }
  for (int i = numchildren; i < total; i++) {
    pthread_create(&people[i], NULL, adult, NULL);
  }

  // Wait until all threads have started
  for (size_t j = 0; j < total; j++) {
    sem_wait(crossing_signal_sem);
  }
  printf("Everyone reported ready, signaling to start.\n");
  fflush(stdout);

  // Signal that the threads can start crossing
  sem_post(signal_start_sem);

  // Wait until all threads have finished crossing
  sem_wait(crossing_signal_sem);

  return 0;
}

void* child(void* x) {
  // Increase number of children on starting island and give them unique ID
  pthread_mutex_lock(&child_lock);
  int name = children_O++;
  pthread_mutex_unlock(&child_lock);

  int island = 0;  // 0 for Oahu, 1 for Molokai

  // Signal to main() that the child thread is ready
  sem_post(crossing_signal_sem);
  printf("Child #%d – Ready.\n", name);
  fflush(stdout);

  // Wait for main() to signal that it is okay to start
  sem_wait(signal_start_sem);

  sem_post(signal_start_sem);
  printf("Child #%d – showing up on Oahu.\n", name);
  fflush(stdout);

  // Master loop – child should always be waiting to help until finished
  while (children_O + adults_O > 0) {
    while (island == 0) {

      // Case 1: The child and the empty boat are on the same island
      pthread_mutex_lock(&boat_lock);
      if (boat_location == 0 && boat == 0) {
        pthread_mutex_lock(&child_lock);
        if (children_O > 1) {
          boat++;
          pthread_mutex_unlock(&boat_lock);

          // Get on boat and wait for another thread to indicate it's full
          pthread_cond_wait(&boat_full_O, &child_lock);

          printf("Child #%d – Getting into boat on Oahu.\n", name);
          fflush(stdout);
          printf(
              "\nOAHU _._._._._._ --> \\__C%d__/ --> _._._._._._ MOLOKAI\n\n",
              name);
          printf("Child #%d – Getting out of boat on Molokai.\n", name);
          fflush(stdout);

          // Update state variables
          pthread_mutex_lock(&boat_lock);
          boat_location = 1;
          island = 1;
          children_O--;
          pthread_mutex_unlock(&boat_lock);
          pthread_mutex_unlock(&child_lock);
        } else {
          // If there is only one child on the island we don't need the boat
          pthread_mutex_unlock(&child_lock);
          pthread_mutex_unlock(&boat_lock);

          pthread_mutex_lock(&adult_lock);
          if (adults_O > 0) {
            pthread_mutex_unlock(&adult_lock);
            pthread_cond_signal(&signal_adult_embark);
            pthread_cond_wait(&boat_available_O, &child_lock);
            pthread_mutex_unlock(&child_lock);
          } else {
            pthread_mutex_unlock(&adult_lock);
            printf("Child #%d – Getting into boat on Oahu.\n", name);
            printf(
                "\nOAHU _._._._._._ --> \\__C%d__/ --> _._._._._._ MOLOKAI\n\n",
                name);
            printf("Child #%d – Getting out of boat on Molokai.\n", name);
            printf("Woohoo! Everyone is across!\n");
            fflush(stdout);
            everyoneAcross = 1;
            pthread_cond_broadcast(&boat_available_M);
            sem_post(crossing_signal_sem);
            return (void*)0;
          }
        }
        // Case 2: The child and the half-full boat are on the same island
      } else if (boat_location == 0 && boat == 1) {
        boat++;
        pthread_mutex_unlock(&boat_lock);  // locked before if statement

        printf("Child #%d – Getting into boat on Oahu.\n", name);
        fflush(stdout);
        pthread_cond_broadcast(&boat_full_O);
        printf("\nOAHU _._._._._._ --> \\__C%d__/ --> _._._._._._ MOLOKAI\n\n",
               name);
        printf("Child #%d – Getting out of boat on Molokai.\n", name);
        fflush(stdout);

        // Update state variables
        pthread_mutex_lock(&boat_lock);
        boat_location = 1;
        pthread_mutex_unlock(&boat_lock);
        pthread_mutex_lock(&child_lock);
        island = 1;
        children_O--;
        pthread_mutex_unlock(&child_lock);

        // Case 3: The child is on the start island and cannot get on the boat.
      } else {
        pthread_mutex_unlock(&boat_lock);
        pthread_cond_wait(&boat_available_O, &child_lock);
        pthread_mutex_unlock(&child_lock);
      }
    }

    pthread_mutex_lock(&child_lock);
    while (island == 1) {
      // Case 4: The child is on Molokai with the empty boat
      pthread_mutex_lock(&boat_lock);
      boat--;
      if (boat_location == 1 && boat == 0) {
        pthread_mutex_unlock(&boat_lock);
        pthread_cond_signal(&boat_available_M);
      } else {
        pthread_mutex_unlock(&boat_lock);
      }

      // Wait on Molokai unless signaled to bring boat back
      pthread_cond_wait(&boat_available_M, &child_lock);

      if (everyoneAcross == 1) {
        pthread_mutex_unlock(&child_lock);
        return (void*)0;
      }

      printf("Child #%d – Getting into boat on Molokai.\n", name);
      printf("\nOAHU _._._._._._ <-- \\__C%d__/ <-- _._._._._._ MOLOKAI\n\n",
             name);
      printf("Child #%d – Getting out boat on Oahu.\n", name);
      fflush(stdout);

      // Update state variables
      pthread_mutex_lock(&boat_lock);
      boat_location = 0;
      pthread_mutex_unlock(&boat_lock);
      island = 0;
      children_O++;
      pthread_cond_signal(&boat_available_O);
    }
    pthread_mutex_unlock(&child_lock);
  }
  return (void*)0;
}

void* adult(void* x) {
  sem_post(crossing_signal_sem);
  printf("Adult – Ready.\n");
  fflush(stdout);

  sem_wait(signal_start_sem);
  sem_post(signal_start_sem);
  printf("Adult – showing up on Oahu.\n");
  fflush(stdout);

  int island = 0;

  pthread_mutex_lock(&adult_lock);
  adults_O++;

  while (island == 0) {
    pthread_cond_wait(&signal_adult_embark, &adult_lock);

    pthread_mutex_lock(&boat_lock);
    printf("Adult – Getting into boat on Oahu.\n");
    fflush(stdout);
    printf("\nOAHU _._._._._._ --> \\__A__/ --> _._._._._._ MOLOKAI\n\n");
    fflush(stdout);
    printf("Adult – Getting out of boat on Molokai.\n");
    fflush(stdout);
    island = 1;
    boat_location = 1;
    adults_O--;
    pthread_mutex_unlock(&boat_lock);
  }
  pthread_mutex_unlock(&adult_lock);

  // Once across, signal the boat is available and then quit
  pthread_cond_signal(&boat_available_M);
  return (void*)0;
}

void initSynch() {

  pthread_cond_init(&signal_adult_embark, NULL);
  pthread_cond_init(&boat_available_O, NULL);
  pthread_cond_init(&boat_full_O, NULL);
  pthread_cond_init(&boat_available_M, NULL);

  pthread_mutex_init(&adult_lock, NULL);
  pthread_mutex_init(&child_lock, NULL);
  pthread_mutex_init(&boat_lock, NULL);

  crossing_signal_sem = sem_open("startsem", O_CREAT | O_EXCL, 0466, 0);
  while (crossing_signal_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore startsem already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("startsem");
      crossing_signal_sem = sem_open("startsem", O_CREAT | O_EXCL, 0466, 0);
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
