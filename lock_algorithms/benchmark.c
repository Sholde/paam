// Include
#include <stdio.h>     // printf
#include <stdlib.h>    // atoll / allocation
#include <string.h>    // string
#include <pthread.h>   // pthread
#define _GNU_SOURCE    // gettid
#include <unistd.h>    // gettid
#include <stdatomic.h> // atomic operatin

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

typedef struct ticket_s
{
  u64 _Atomic ticket;
  u64 _Atomic screen;
} ticket_t;

// Enum
enum
  {
    FREE,
    BUSY
  };

// Global variable
void *(*benchmark)(void *) = NULL; // pointer to benchmark function of given
                                   // lock algorithm
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
u64 _Atomic spin = FREE;
ticket_t ticket;

// Waiting time nanosecond
void nanowait(double time)
{
  struct timespec clock;
  clock_gettime(CLOCK_MONOTONIC, &clock);

  double before = clock.tv_sec + clock.tv_nsec * 1e-9;
  double after = 0.0;

  do
    {
      clock_gettime(CLOCK_MONOTONIC, &clock);
      after = clock.tv_sec + clock.tv_nsec * 1e-9;
    }
  while ((after - before) - time > 0.0);
}

// Posix benchmark
void *posix_benchmark(void *arg)
{
  // Get argument
  work_t *work = (work_t *)arg;

  // Structure to monitoring lock and unlock
  struct timespec clock;
  double before_lock;
  double after_lock;
  double before_unlock;
  double after_unlock;

  double time_lock = 0;
  double time_unlock = 0;

  // loop
  for (u64 i = 0; i < work->number_of_iteration; ++i)
    {
      // Take time before the lock
      clock_gettime(CLOCK_MONOTONIC, &clock);
      before_lock = clock.tv_sec + clock.tv_nsec * 1e-9;;

      // Take the lock
      pthread_mutex_lock(&mut);
      {
        // Take time after the lock
        clock_gettime(CLOCK_MONOTONIC, &clock);
        after_lock = clock.tv_sec + clock.tv_nsec * 1e-9;

        // Critical section
        nanowait(work->critical_section_delay);

        // Take time before the unlock
        clock_gettime(CLOCK_MONOTONIC, &clock);
        before_unlock = clock.tv_sec + clock.tv_nsec * 1e-9;
      }
      pthread_mutex_unlock(&mut);

      // Take time after the unlock
      clock_gettime(CLOCK_MONOTONIC, &clock);
      after_unlock = clock.tv_sec + clock.tv_nsec * 1e-9;

      // Compute
      nanowait(work->compute_delay);

      // Add time
      time_lock += (after_lock - before_lock);
      time_unlock += (after_unlock - before_unlock);
    }

  printf("hello from thread %u, lock %f, unlock %f\n",
         gettid(), time_lock, time_unlock);
  return NULL;
}

// Spin lock
void spin_lock(u64 _Atomic *spin)
{
  while (atomic_exchange(spin, BUSY) != FREE)
    {
      ;
    }
}

// Spin unlock
void spin_unlock(u64 _Atomic *spin)
{
  atomic_store(spin, FREE);
}

// Spinlock benchmark
void *spin_benchmark(void *arg)
{
  // Get argument
  work_t *work = (work_t *)arg;

  // Structure to monitoring lock and unlock
  struct timespec clock;
  double before_lock;
  double after_lock;
  double before_unlock;
  double after_unlock;

  double time_lock = 0;
  double time_unlock = 0;

  // loop
  for (u64 i = 0; i < work->number_of_iteration; ++i)
    {
      // Take time before the lock
      clock_gettime(CLOCK_MONOTONIC, &clock);
      before_lock = clock.tv_sec + clock.tv_nsec * 1e-9;;

      // Take the lock
      spin_lock(&spin);
      {
        // Take time after the lock
        clock_gettime(CLOCK_MONOTONIC, &clock);
        after_lock = clock.tv_sec + clock.tv_nsec * 1e-9;

        // Critical section
        nanowait(work->critical_section_delay);

        // Take time before the unlock
        clock_gettime(CLOCK_MONOTONIC, &clock);
        before_unlock = clock.tv_sec + clock.tv_nsec * 1e-9;
      }
      spin_unlock(&spin);

      // Take time after the unlock
      clock_gettime(CLOCK_MONOTONIC, &clock);
      after_unlock = clock.tv_sec + clock.tv_nsec * 1e-9;

      // Compute
      nanowait(work->compute_delay);

      // Add time
      time_lock += (after_lock - before_lock);
      time_unlock += (after_unlock - before_unlock);
    }

  printf("hello from thread %u, lock %f, unlock %f\n",
         gettid(), time_lock, time_unlock);
  return NULL;
}

// Ticket lock
void ticket_lock(ticket_t *ticket)
{
  u64 my_ticket = atomic_fetch_add(&(ticket->ticket), 1);

  while (atomic_load(&(ticket->ticket)) < my_ticket)
    {
      ;
    }
}

// Ticket unlock
void ticket_unlock(ticket_t *ticket)
{
  atomic_fetch_add(&(ticket->screen), 1);
}

// Ticket benchmark
void *ticket_benchmark(void *arg)
{
  // Get argument
  work_t *work = (work_t *)arg;

  // Structure to monitoring lock and unlock
  struct timespec clock;
  double before_lock;
  double after_lock;
  double before_unlock;
  double after_unlock;

  double time_lock = 0;
  double time_unlock = 0;

  // Init ticket
  ticket.ticket = 0;
  ticket.screen = 0;

  // loop
  for (u64 i = 0; i < work->number_of_iteration; ++i)
    {
      // Take time before the lock
      clock_gettime(CLOCK_MONOTONIC, &clock);
      before_lock = clock.tv_sec + clock.tv_nsec * 1e-9;;

      // Take the lock
      ticket_lock(&ticket);
      {
        // Take time after the lock
        clock_gettime(CLOCK_MONOTONIC, &clock);
        after_lock = clock.tv_sec + clock.tv_nsec * 1e-9;

        // Critical section
        nanowait(work->critical_section_delay);

        // Take time before the unlock
        clock_gettime(CLOCK_MONOTONIC, &clock);
        before_unlock = clock.tv_sec + clock.tv_nsec * 1e-9;
      }
      ticket_unlock(&ticket);

      // Take time after the unlock
      clock_gettime(CLOCK_MONOTONIC, &clock);
      after_unlock = clock.tv_sec + clock.tv_nsec * 1e-9;

      // Compute
      nanowait(work->compute_delay);

      // Add time
      time_lock += (after_lock - before_lock);
      time_unlock += (after_unlock - before_unlock);
    }

  printf("hello from thread %u, lock %f, unlock %f\n",
         gettid(), time_lock, time_unlock);
  return NULL;
}

int main(int argc, char **argv)
{
  // Init error code
  int error_code = 0;

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
  else if (strcmp(work.lock_algorithm, "spin") == 0)
    {
      benchmark = spin_benchmark;
    }
  else if (strcmp(work.lock_algorithm, "ticket") == 0)
    {
      benchmark = ticket_benchmark;
    }
  else
    {
      printf("Error: %s lock algorithm is not available !\n",
             work.lock_algorithm);
      printf("       Please choose one of: none, posix, spin\n");

      // Exit program
      error_code = 1;
      goto label_exit_program;
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
  return error_code;
}
