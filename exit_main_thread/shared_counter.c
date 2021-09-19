#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef unsigned long long u64;

u64 counter = 0;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void *hello_thread(void *arg)
{
  u64 i = *(int *)(arg);

  u64 my_tid = pthread_self();

  u64 _counter = 0;

  pthread_mutex_lock(&mut);
  {
    _counter = counter;
    counter++;
    pthread_cond_wait(&cond, &mut);
  }
  pthread_mutex_unlock(&mut);

  printf("%llu Hello, World %llu with counter %llu !\n", my_tid, i, _counter);

  return NULL;
}

int main(int argc, char **argv)
{
  // Check comment line argument
  if (argc != 2)
    {
      printf("Need the NUMBER of thread on argument !\n");
      return 1;
    }

  // Declare variable
  u64 N = atoll(argv[1]);
  pthread_t *tid = malloc(sizeof(pthread_t) * N);
  u64 *to_share = malloc(sizeof(u64) * N);

  // Init table
  for (u64 i = 0; i < N; i++)
    {
      tid[i] = 0;
      to_share[i] = 0;
    }

  // Create thread
  for (u64 i = 0; i < N; i++)
    {
      to_share[i] = i;
      pthread_create(&(tid[i]), NULL, hello_thread, (void *)(&to_share[i]));
    }

  // Main finish is work
  int tmp = 0;
  while (counter != N) { tmp++; }

  pthread_mutex_lock(&mut);
  {
    pthread_cond_broadcast(&cond);
  }
  pthread_mutex_unlock(&mut);

  printf("counter: %llu\n", counter);
  printf("Main quitting\n");

  // Free memory
  free(tid);
  free(to_share);

  // Exit main thread
  pthread_exit(NULL);

  return 0;
}
