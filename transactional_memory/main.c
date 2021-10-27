// Include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdatomic.h>
#include <limits.h>

// Define
#define ALIGNED 64
#define N_CELLS 65536
#define SIZE 1024

// Struct
typedef unsigned long long u64;

typedef struct cell_s
{
  u64 value;
  u64 counter;
} cell_t;

typedef struct memory_s
{
  struct cell_s *cells[N_CELLS];
  u64 clock;
} memory_t;

typedef struct TX_s
{
  u64 read_set[N_CELLS];
  u64 write_set[2][N_CELLS];
  u64 clock;
} TX_t;

// Global variable
void *(*func)(void *) = NULL;
memory_t mem;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
__thread TX_t tx;

// Function
void init_memory(u64 n)
{
  for (u64 i = 0; i < N_CELLS; i++)
    {
      mem.cells[i] = aligned_alloc(ALIGNED, sizeof(cell_t));
      mem.cells[i]->value = 0;
      mem.cells[i]->counter = 0;
    }

  mem.clock = 1;
}

void free_memory()
{
  for (u64 i = 0; i < N_CELLS; i++)
    free(mem.cells[i]);
}

void *func_atomic(void *arg)
{
  struct timespec clock;

  clock_gettime(CLOCK_MONOTONIC, &clock);
  double before = clock.tv_sec + clock.tv_nsec * 1e-9;

  for (u64 i = 0; i < SIZE; i++)
    atomic_fetch_add(&(mem.cells[0]->value), 1);

  clock_gettime(CLOCK_MONOTONIC, &clock);
  double after = clock.tv_sec + clock.tv_nsec * 1e-9;

  printf("Hello from %lu, time: %lf\n", pthread_self(), after - before);

  return NULL;
}

void *func_mutex(void *arg)
{
  struct timespec clock;

  clock_gettime(CLOCK_MONOTONIC, &clock);
  double before = clock.tv_sec + clock.tv_nsec * 1e-9;

  for (u64 i = 0; i < SIZE; i++)
    {
      pthread_mutex_lock(&mut);
      {
        mem.cells[0]->value += 1;
      }
      pthread_mutex_unlock(&mut);
    }

  clock_gettime(CLOCK_MONOTONIC, &clock);
  double after = clock.tv_sec + clock.tv_nsec * 1e-9;

  printf("Hello from %lu, time: %lf\n", pthread_self(), after - before);

  return NULL;
}

void startTX(void)
{
  tx.clock = mem.clock;

  for (u64 i = 0; i < N_CELLS; i++)
    {
      tx.read_set[i] = 0;
      tx.write_set[0][i] = 0;
      tx.write_set[1][i] = 0;
    }
}

u64 read(u64 n)
{
  if (tx.write_set[0][n])
    return tx.write_set[1][n];

  cell_t *cell = mem.cells[n];

  if (cell->counter >= tx.clock)
    return INT_MAX;

  tx.read_set[n] = 1;

  return cell->value;
}

void write(u64 n, u64 value)
{
  tx.write_set[0][n] = 1;
  tx.write_set[1][n] = value;
}

u64 commitTX()
{
  pthread_mutex_lock(&mut);
  {
    for (u64 i = 0; i < N_CELLS; i++)
      {
        if (!(tx.read_set[i]))
          continue;

        if (mem.cells[i]->counter >= tx.clock)
          {
            // Release mutex
            pthread_mutex_unlock(&mut);
            return 0;
          }
      }

    // ok, commit !
    for (u64 i = 0; i < N_CELLS; i++)
      {
        if (!(tx.write_set[0][i]))
          continue;

        cell_t *cell = aligned_alloc(ALIGNED, sizeof(cell_t));
        cell->value = tx.write_set[1][i];
        cell->counter = mem.clock;
        mem.cells[i] = cell;
      }

    mem.clock += 1;
  }
  pthread_mutex_unlock(&mut);

  return 1;
}

void *func_sft(void *arg)
{
  struct timespec clock;

  clock_gettime(CLOCK_MONOTONIC, &clock);
  double before = clock.tv_sec + clock.tv_nsec * 1e-9;

  for (u64 i = 0; i < SIZE; i++)
    {
      // Stuff to do
    restart:
      startTX();
      u64 value = read(0); // read x

      if (value == INT_MAX)
        goto restart;

      write(0, value + 1); // write x + 1

      if (!commitTX()) // try to commit
        goto restart;  // in case of abort, restart
    }

  clock_gettime(CLOCK_MONOTONIC, &clock);
  double after = clock.tv_sec + clock.tv_nsec * 1e-9;

  printf("Hello from %lu, time: %lf\n", pthread_self(), after - before);

  return NULL;
}

int main(int argc, char **argv)
{
  if (argc != 3)
    return 1;

  u64 n = atoll(argv[1]);

  pthread_t *tid = aligned_alloc(ALIGNED, sizeof(pthread_t) * n);

  init_memory(n);

  if (strcmp(argv[2], "atomic") == 0)
    func = func_atomic;
  else if (strcmp(argv[2], "mutex") == 0)
    func = func_mutex;
  else if (strcmp(argv[2], "sft") == 0)
    func = func_sft;
  else
    exit(3);

  for (u64 i = 0; i < n; i ++)
    pthread_create(tid + i, NULL, func, NULL);

  for (u64 i = 0; i < n; i ++)
    pthread_join(tid[i], NULL);

  printf("x: %llu\n", mem.cells[0]->value);

  free_memory();

  return 0;
}
