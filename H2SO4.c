/*
 * Project 2
 * Authors: Caleb Braun and Reilly Hallstrom
 * 4/27/2016
 * A C program that builds H2SO4 molecules using separate threads
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include "H2SO4.h"

// Semaphores for allowing only certain number of atoms to combine as H2SO4
sem_t* mol_oxy;
sem_t* mol_hydro;
sem_t* mol_sulf;

// Semaphores to make the threads leave in correct order
sem_t* hydrogen_left;
sem_t* sulfur_left;
sem_t* sulfur_lock;

// Lock to prevent atoms from leaving
sem_t* mol_sem;

// Lock on our global counter
sem_t* sum_lock;

// Global counter to keep track of whether we have enough atoms
int mol_sum = 0;

// Function representing one oxygen atom
void* oxygen(void* x) {
  printf("oxygen produced\n");
  fflush(stdout);

  int builtMolecule = 0;  // False
  int molNum = 0;

  // Allows the first four oxygens to go through
  sem_wait(mol_oxy);

  // =================== Aquires sum_lock ===================
  sem_wait(sum_lock);
  mol_sum++;
  molNum = mol_sum;
  // Check if we can build the molecule
  if (mol_sum == 7) {
    sem_post(mol_sem);  // Allow atoms to leave
    printf("\n*** Made H2SO4 Molecule! ***\n\n");
    fflush(stdout);
    builtMolecule++;
    mol_sum = 0;
  }
  sem_post(sum_lock);
  // =================== Returns sum_lock ===================

  // Waits until the molecule has 7 atoms
  sem_wait(mol_sem);

  // Increments to allow other atoms to leave
  if (builtMolecule != 1) {
    sem_post(mol_sem);
  }

  // If it built the molecule refill the buffers allowing
  // other atoms to be make an molecule
  if (builtMolecule == 1) {
    sem_post(mol_hydro);
    sem_post(mol_hydro);
    sem_post(mol_sulf);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
  }

  // wait until one sulfur has left before leaving
  sem_wait(sulfur_left);

  // Leave and return
  printf("oxygen %d leaving \n", molNum);
  fflush(stdout);
  return (void*)0;
}

// Function representing one hydrogen atom
void* hydrogen(void* x) {
  int builtMolecule = 0;
  int molNum = 0;

  printf("hydrogen produced\n");
  fflush(stdout);

  sem_wait(mol_hydro);  // 2 down to 0

  // =================== Aquires sum_lock ===================
  sem_wait(sum_lock);
  mol_sum++;
  molNum = mol_sum;
  // Check if we can build the molecule
  if (mol_sum == 7) {
    sem_post(mol_sem);  // Allow atoms to leave
    printf("\n*** Made H2SO4 Molecule! ***\n\n");
    fflush(stdout);
    builtMolecule++;
    mol_sum = 0;
  }
  sem_post(sum_lock);
  // =================== Returns sum_lock ===================

  sem_wait(mol_sem);  // Waits for last atom in molecule

  // Increments to allow other atoms to leave
  if (builtMolecule != 1) {
    sem_post(mol_sem);
  }

  // if this atom built a molecule then refill the buffers
  if (builtMolecule == 1) {
    sem_post(mol_hydro);
    sem_post(mol_hydro);
    sem_post(mol_sulf);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
  }

  // Let sulfur know a hydrogen has left
  sem_post(hydrogen_left);

  // Leave and return
  printf("hydrogen %d leaving \n", molNum);
  fflush(stdout);
  return (void*)0;
}

// Function representing one sulfur atom
void* sulfur(void* x) {
  int builtMolecule = 0;
  int molNum = 0;

  printf("sulfur produced\n");
  fflush(stdout);

  sem_wait(mol_sulf);  // 1 down to 0

  // =================== Aquires sum_lock ===================
  sem_wait(sum_lock);
  mol_sum++;
  molNum = mol_sum;
  // Check if we can build the molecule
  if (mol_sum == 7) {
    sem_post(mol_sem);  // Allow atoms to leave
    printf("\n*** Made H2SO4 Molecule! ***\n\n");
    fflush(stdout);
    builtMolecule++;
    mol_sum = 0;
  }
  sem_post(sum_lock);
  // =================== Returns sum_lock ===================

  sem_wait(mol_sem);  // Waits for last atom in molecule

  // Increments to allow other atoms to leave
  if (builtMolecule != 1) {
    sem_post(mol_sem);
  }

  // Let oxygen know that a sulfur has left
  sem_post(sulfur_left);

  // if this atom built a molecule then refill the buffers
  if (builtMolecule == 1) {
    sem_post(mol_hydro);
    sem_post(mol_hydro);
    sem_post(mol_sulf);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
    sem_post(mol_oxy);
  }

  // wait for two hydrogen atoms to leave
  sem_wait(hydrogen_left);
  sem_wait(hydrogen_left);

  // let 4 oxygen atoms leave
  sem_post(sulfur_left);
  sem_post(sulfur_left);
  sem_post(sulfur_left);
  sem_post(sulfur_left);

  // leave and return
  printf("sulfur %d leaving \n", molNum);
  fflush(stdout);
  return (void*)0;
}

void openSems() {
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

  sulfur_lock = sem_open("sulfurlock", O_CREAT | O_EXCL, 0466, 1);
  while (sulfur_lock == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore sumlock already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("sulfurlock");
      sulfur_lock = sem_open("sulfurlock", O_CREAT | O_EXCL, 0466, 1);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  hydrogen_left = sem_open("hybye", O_CREAT | O_EXCL, 0466, 0);
  while (hydrogen_left == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore hybye already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("hybye");
      hydrogen_left = sem_open("hybye", O_CREAT | O_EXCL, 0466, 0);
    } else {
      printf("semaphore could not be opened, error # %d\n", errno);
      fflush(stdout);
      exit(1);
    }
  }

  sulfur_left = sem_open("sybye", O_CREAT | O_EXCL, 0466, 0);
  while (sulfur_left == SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore sybye already exists, unlinking and reopening\n");
      fflush(stdout);
      sem_unlink("sybye");
      sulfur_left = sem_open("sybye", O_CREAT | O_EXCL, 0466, 0);
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

  sem_close(sulfur_lock);
  sem_unlink("sulfurlock\n");

  sem_close(hydrogen_left);
  sem_unlink("hybye\n");

  sem_close(sulfur_left);
  sem_unlink("sybye\n");
}
