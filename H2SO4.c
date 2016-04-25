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
#include <time.h>
#include "H2SO4.h"

sem_t* oxy_sem;
sem_t* hydro_sem;
sem_t* sulf_sem;
sem_t* mol_sem;

void* oxygen(void* x) {
  printf("oxygen produced\n");
  fflush(stdout);

  // Tell oxy_sem that there's one more O atom available
  sem_post(oxy_sem);

  if (*oxy_sem < 5) {
    sem_post(mol_sem);
  }

  if (*mol_sem == 7) {
    sem_wait(oxy_sem);
    printf("oxygen leaving \n");
    fflush(stdout);
  }

  // printf("oxygen leaving \n");
  // fflush(stdout);
  return (void*)0;
}

void* hydrogen(void* x) {
  printf("hydrogen produced\n");
  fflush(stdout);

  sem_post(hydro_sem);
  if (*hydro_sem < 3) {
    sem_post(mol_sem);
  }

  printf("hydrogen leaving \n");
  fflush(stdout);
  return (void*)0;
}

void* sulfur(void* x) {
  printf("sulfur produced\n");
  fflush(stdout);

  sem_post(sulf_sem);
  if (*sulf_sem < 2) {
    sem_post(mol_sem);
  }

  // produce a water molecule
  printf("\n*** Made H2SO4 Molecule! ***\n\n");
  fflush(stdout);

  printf("sulfur leaving \n");
  fflush(stdout);
  return (void*)0;
}

void openSems() {
  printf("open\n");

  oxy_sem = sem_open("oxysem", O_CREAT | O_EXCL, 0466, 0);
  while (oxy_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore oxysem already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("oxysem");
      oxy_sem = sem_open("oxysem", O_CREAT | O_EXCL, 0466, 0);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  hydro_sem = sem_open("hydrosem", O_CREAT | O_EXCL, 0466, 0);
  while (hydro_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore hydrosem already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("hydrosem");
      hydro_sem = sem_open("hydrosem", O_CREAT | O_EXCL, 0466, 0);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  sulf_sem = sem_open("sulfsem", O_CREAT | O_EXCL, 0466, 0);
  while (sulf_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore sulfsem already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("sulfsem");
      sulf_sem = sem_open("sulfsem", O_CREAT | O_EXCL, 0466, 0);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  mol_sem = sem_open("molsem", O_CREAT | O_EXCL, 0466, 7);
  while (mol_sem == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore molsem already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("molsem");
      mol_sem = sem_open("molsem", O_CREAT | O_EXCL, 0466, 7);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }
}

void closeSems() {
  printf("close semaphores\n");
  sem_close(hydro_sem);
  sem_unlink("hydrosem\n");
  sem_close(oxy_sem);
  sem_unlink("oxysem\n");
  sem_close(sulf_sem);
  sem_unlink("sulfsem\n");
  sem_close(mol_sem);
  sem_unlink("molsem\n");
}
