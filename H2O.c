/* Example of using semaphores - MAC compatible methods only
 * sem_open, sem_wait, sem_post
 * Author: Sherri Goings
 * Last modified: 1/30/2014
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

// functions of the 2 types of threads, one that produces oxygen and one
// hydrogen
void* Oxygen(void*);
void* Hydrogen(void*);
void delay(int);

// declare hydrogen semaphore as global variable
sem_t* hydro_sem;

int main() {

  // create the hydrogen semaphore, very important to use last 3 arguments as
  // shown here
  // first argument is simply filename for semaphore, any name is fine but must
  // be a valid path
  hydro_sem = sem_open("hydrosmphr", O_CREAT | O_EXCL, 0466, 0);

  // deal with possibility that previous run of program created semaphore with
  // same name and
  // didn't properly close & unlink for whatever reason
  while (hydro_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore hydrosmphr already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("hydrosmphr");
      hydro_sem = sem_open("hydrosmphr", O_CREAT | O_EXCL, 0466, 0);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  // testing water molecule creation, 2nd molecule should not be made until all
  // 4 hydrogen
  // and 2 oxygen atoms have been produced
  pthread_t oxy1, hydro1, oxy2, hydro2, hydro3, hydro4;
  pthread_create(&hydro1, NULL, Hydrogen, NULL);
  pthread_create(&hydro2, NULL, Hydrogen, NULL);
  pthread_create(&oxy1, NULL, Oxygen, NULL);
  pthread_create(&oxy2, NULL, Oxygen, NULL);
  pthread_create(&hydro3, NULL, Hydrogen, NULL);
  pthread_create(&hydro4, NULL, Hydrogen, NULL);

  pthread_join(hydro1, NULL);
  pthread_join(hydro2, NULL);
  pthread_join(oxy1, NULL);
  pthread_join(oxy2, NULL);
  pthread_join(hydro3, NULL);
  pthread_join(hydro4, NULL);

  sem_close(hydro_sem);
  sem_unlink("hydrosmphr");

  return 0;
}

void* Oxygen(void* args) {
  // produce an oxygen molecule - takes some (small) random amount of time
  delay(rand() % 2000);
  printf("oxygen produced\n");
  fflush(stdout);

  // oxygen waits (calls down) twice on the hydrogen semaphore
  // meaning it cannot continue until at least 2 hydrogen atoms
  // have been produced
  int err = sem_wait(hydro_sem);
  int err2 = sem_wait(hydro_sem);
  if (err == -1 || err2 == -1)
    printf("error on oxygen wait for hydro_sem, error # %d\n", errno);

  // produce a water molecule
  printf("made H20\n");
  fflush(stdout);

  // oxygen exits
  printf("oxygen leaving\n");
  fflush(stdout);

  return (void*)0;
}
void* Hydrogen(void* args) {
  // produce a hydrogen molecule - takes some (small) random amount of time
  delay(rand() % 3000);
  printf("hydrogen produced\n");
  fflush(stdout);

  // post (call up) on hydrogen semaphore to signal that a hydrogen atom
  // has been produced
  sem_post(hydro_sem);

  // hydrogen exits
  printf("hydrogen leaving\n");
  fflush(stdout);

  return (void*)0;
}

/*
 * NOP function to simply use up CPU time
 * arg limit is number of times to run each loop, so runs limit^2 total loops
 */
void delay(int limit) {
  int j, k;

  for (j = 0; j < limit; j++) {
    for (k = 0; k < limit; k++) {
    }
  }
}
