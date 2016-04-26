/*
 * Project 2
 * Authors: Caleb Braun and Reilly Hallstrom
 * 4/27/2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "H2SO4.h"

sem_t* mol_oxy;
sem_t* mol_hydro;
sem_t* mol_sulf;

sem_t* mol_sem;  // Indicates whether the atoms are allowed to leave
sem_t* sum_lock;

int mol_sum;

void* oxygen(void* x) {
  printf("oxygen produced\n");
  fflush(stdout);

  // Allows the first four oxygens to go through
  sem_wait(mol_oxy);

  // Aquires sum_lock in order to increment mol_sum
  sem_wait(sum_lock);
  mol_sum++;
  sem_post(sum_lock);

  printf("O Ready for Formation: %d\n", mol_sum);
  fflush(stdout);

  // Aquires sum_lcok in order to check it and maybe change its value
  sem_wait(sum_lock);
  if (mol_sum == 7) {
    sem_post(mol_sem);  // Either one or zero
    printf("\n*** Made H2SO4 Molecule! ***\n\n");
    fflush(stdout);
    mol_sum = 0;
  }
  sem_post(sum_lock);

  sem_wait(mol_sem);  // Waits for last atom in molecule
  sem_post(mol_sem);  // Increments to allow other atoms to leave

  sem_post(mol_oxy);
  printf("oxygen leaving \n");
  fflush(stdout);
  return (void*)0;
}

void* hydrogen(void* x) {
  printf("hydrogen produced\n");
  fflush(stdout);

  sem_wait(mol_hydro);  // 2 down to 0

  sem_wait(sum_lock);
  mol_sum++;
  sem_post(sum_lock);

  printf("H Ready for Formation: %d\n", mol_sum);
  fflush(stdout);

  sem_wait(sum_lock);
  if (mol_sum == 7) {
    sem_post(mol_sem);  // Either one or zero
    printf("\n*** Made H2SO4 Molecule! ***\n\n");
    fflush(stdout);
    mol_sum = 0;
  }
  sem_post(sum_lock);

  sem_wait(mol_sem);  // Waits for last atom in molecule
  sem_post(mol_sem);  // Increments to allow other atoms to leave

  sem_post(mol_hydro);
  printf("hydrogen leaving \n");
  fflush(stdout);
  return (void*)0;
}

void* sulfur(void* x) {
  printf("sulfur produced\n");
  fflush(stdout);

  sem_wait(mol_sulf);  // 1 down to 0

  sem_wait(sum_lock);
  mol_sum++;
  sem_post(sum_lock);

  printf("S Ready for Formation: %d\n", mol_sum);
  fflush(stdout);

  sem_wait(sum_lock);
  if (mol_sum == 7) {
    sem_post(mol_sem);  // Either one or zero
    printf("\n*** Made H2SO4 Molecule! ***\n\n");
    fflush(stdout);
    mol_sum = 0;
  }
  sem_post(sum_lock);

  sem_wait(mol_sem);  // Waits for last atom in molecule
  sem_post(mol_sem);  // Increments to allow other atoms to leave

  sem_post(mol_sulf);
  printf("sulfur leaving \n");
  fflush(stdout);
  return (void*)0;
}

void openSems() {
  printf("open\n");

  mol_oxy = sem_open("moloxy", O_CREAT | O_EXCL, 0466, 4);
  while (mol_oxy == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore moloxy already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("moloxy");
      mol_oxy = sem_open("moloxy", O_CREAT | O_EXCL, 0466, 4);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  mol_hydro = sem_open("molhydro", O_CREAT | O_EXCL, 0466, 2);
  while (mol_hydro == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore molhydro already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("molhydro");
      mol_hydro = sem_open("molhydro", O_CREAT | O_EXCL, 0466, 2);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  mol_sulf = sem_open("molsulf", O_CREAT | O_EXCL, 0466, 1);
  while (mol_sulf == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore molsulf already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("molsulf");
      mol_sulf = sem_open("molsulf", O_CREAT | O_EXCL, 0466, 1);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  mol_sem = sem_open("molsem", O_CREAT | O_EXCL, 0466, 0);
  while (mol_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore molsem already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("molsem");
      mol_sem = sem_open("molsem", O_CREAT | O_EXCL, 0466, 0);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  sum_lock = sem_open("sumlock", O_CREAT | O_EXCL, 0466, 1);
  while (sum_lock == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore sumlock already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("sumlock");
      sum_lock = sem_open("sumlock", O_CREAT | O_EXCL, 0466, 1);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }
}

void closeSems() {
  printf("close semaphores\n");

  sem_close(mol_hydro);
  sem_unlink("molhydro\n");

  sem_close(mol_oxy);
  sem_unlink("moloxy\n");

  sem_close(mol_sulf);
  sem_unlink("molsulf\n");

  sem_close(mol_sem);
  sem_unlink("molsem\n");

  sem_close(sum_lock);
  sem_unlink("sumlock\n");
}
