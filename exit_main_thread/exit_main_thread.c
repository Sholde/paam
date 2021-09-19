#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef unsigned long long u64;

void *hello_thread(void *arg)
{
  u64 i = *(int *)(arg);

  printf("%lu Hello, World %llu !\n", pthread_self(), i);

  // Exit thread
  pthread_exit(NULL);

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
  u64 *to_share = malloc(sizeof(u64) * N);;

  // Init table
  for (u64 i = 0; i < N; i++)
      tid[i] = 0;

  // Create thread
  for (u64 i = 0; i < N; i++)
    {
      to_share[i] = i;
      pthread_create(&(tid[i]), NULL, hello_thread, (void *)(&to_share[i]));
    }

  // Free memory
  free(tid);
  /* Here, we cannot free the memory unless we add a global variable that
   * counts the number of threads that have finished
   */
  //free(to_share);

  // Main finish is work
  printf("Main quitting\n");

  // Exit main thread
  pthread_exit(NULL);

  return 0;
}
