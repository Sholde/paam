// Include
#include <stdio.h>   // printf
#include <stdlib.h>  // atoll / allocation
#include <string.h>  // string
#include <pthread.h> // pthread
#define _GNU_SOURCE
#include <unistd.h>  // gettid

// Define
#define ALIGNED 64

// Struct / Typedef
typedef unsigned long long u64;

typedef struct work_s
{
  u64 number_of_threads;
  u64 number_of_iteration;
  u64 critical_section_delay;
  u64 compute_delay;
  char *lock_algorithm;
} work_t;

// Global variable
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
void *(*benchmark)(void *) = NULL; // pointer to benchmark function of given lock algorithm

// Function
void nanowait(u64 time)
{
  struct timespec before;
  clock_gettime(CLOCK_MONOTONIC, &before);
  struct timespec after;

  do
    {
      clock_gettime(CLOCK_MONOTONIC, &after);
    }
  while ((after.tv_nsec - before.tv_nsec) < time);
}

void *posix_benchmark(void *arg)
{
  // Get argument
  work_t *work = (work_t *)arg;

  // Structure to monitoring lock and unlock
  struct timespec before_lock;
  struct timespec after_lock;
  struct timespec before_unlock;
  struct timespec after_unlock;

  u64 time_lock = 0;
  u64 time_unlock = 0;

  // loop
  for (u64 i = 0; i < work->number_of_iteration; ++i)
    {
      // Take time before the lock
      clock_gettime(CLOCK_MONOTONIC, &before_lock);

      // Take the lock
      pthread_mutex_lock(&mut);
      {
        // Take time after the lock
        clock_gettime(CLOCK_MONOTONIC, &after_lock);

        // Critical section
        nanowait(work->critical_section_delay);

        // Take time before the unlock
        clock_gettime(CLOCK_MONOTONIC, &before_unlock);
      }
      pthread_mutex_unlock(&mut);

      // Take time after the unlock
      clock_gettime(CLOCK_MONOTONIC, &after_unlock);

      // Compute
      nanowait(work->compute_delay);

      // Add time
      time_lock += (after_lock.tv_nsec - before_lock.tv_nsec);
      time_unlock += (after_unlock.tv_nsec - before_unlock.tv_nsec);
    }

  printf("hello from thread %u, lock %llu, unlock %llu\n",
         gettid(), time_lock, time_unlock);
  return NULL;
}

int main(int argc, char **argv)
{
  // Check argument
  if (argc != 6)
    {
      printf("Need 5 arguments !\n");
      printf("  - number of threads\n");
      printf("  - number of iteration\n");
      printf("  - critical section delay\n");
      printf("  - compute delay\n");
      printf("  - lock algorithm\n");
      return 1;
    }

  // Init variable
  work_t work;
  work.number_of_threads = atoll(argv[1]);
  work.number_of_iteration = atoll(argv[2]);
  work.critical_section_delay = atoll(argv[3]);
  work.compute_delay = atoll(argv[4]);
  work.lock_algorithm = aligned_alloc(ALIGNED, sizeof(char) * strlen(argv[5]));
  strcpy(work.lock_algorithm, argv[5]);

  // Check algorithm
  if (strcmp(work.lock_algorithm, "none") == 0)
    {
      // Print values
      printf("number of threads: %llu\n", work.number_of_threads);
      printf("number of iteration: %llu\n", work.number_of_iteration);
      printf("critical section delay: %llu\n", work.critical_section_delay);
      printf("compute delay: %llu\n", work.compute_delay);
      printf("lock algorithm: %s\n", work.lock_algorithm);

      // Exit program
      goto label_exit_program;
    }
  else if (strcmp(work.lock_algorithm, "posix") == 0)
    {
      benchmark = posix_benchmark;
    }
  else
    {
      printf("Error: %s lock algorithm is not available !\n",
             work.lock_algorithm);
      printf("       Please choose one of: none, posix\n");
      free(work.lock_algorithm);
      return 1;
    }

  // Create threads
  pthread_t *tid = aligned_alloc(ALIGNED,
                                 sizeof(pthread_t) * work.number_of_threads);
  for (u64 i = 0; i < work.number_of_threads; ++i)
    {
      pthread_create(tid + i, NULL, benchmark, &work);
    }

  // Wait threads
  for (u64 i = 0; i < work.number_of_threads; ++i)
    {
      pthread_join(tid[i], NULL);
    }

 label_exit_program:
  // Release memory
  free(work.lock_algorithm);

  // Exit program
  return 0;
}
