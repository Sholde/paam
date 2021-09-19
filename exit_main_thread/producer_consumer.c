#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_LENGTH 256
#define MAX_THREAD 16

typedef unsigned long long u64;

struct list_s
{
  struct list_s *next;
  char string[MAX_LENGTH];
};

struct share_s
{
  u64 num_thread;
  u64 id_thread;
};

typedef struct list_s list_t;
typedef struct share_s share_t;

list_t *begin = NULL;
list_t *end = NULL;
u64 read_count = 0;
u64 done[MAX_THREAD] = { 0 };

share_t to_share[MAX_THREAD];

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Consumer function lauch by all thread created by main thread
void *consume(void *arg)
{
  // Get info
  share_t to_share = *(share_t *)arg;
  u64 N = to_share.num_thread;
  u64 id = to_share.id_thread;

  // Get tid
  u64 tid = pthread_self();

  //
  for (;;)
    {
      pthread_mutex_lock(&mut);
      {
        if (begin != NULL && !done[id])
          {
            // Read first node
            printf("[%llu] %s\n", tid, begin->string);
            read_count++;
            done[id] = 1;

            // Release the first node after all thread read it
            if (read_count == N)
              {
                // Reset counter
                read_count = 0;

                // Release
                list_t *tmp = begin;
                begin = begin->next;
                free(tmp);
              }
          }

        // Waiting input from producer
        pthread_cond_wait(&cond, &mut);
        done[id] = 0;
      }
      pthread_mutex_unlock(&mut);
    }
}

void produce(void)
{
  //
  for (;;)
    {
      // Create new node
      list_t *cur = malloc(sizeof(list_t));
      cur->next = NULL;

      // Scan stdin
      scanf("%s", cur->string);

      // Check if it is "exit" word
      if (strcmp(cur->string, "exit") == 0)
        {
          // Free list before exiting
          free(cur);
          while (begin)
            {
              list_t *tmp = begin;
              begin = begin->next;
              free(tmp);
            }

          // Exit program
          exit(0);
        }

      // Adding new node to list
      pthread_mutex_lock(&mut);
      {
        if (begin == NULL || end == NULL)
          {
            begin = cur;
            end = cur;
          }
        else
          {
            end->next = cur;
            end = cur;
          }

        // Release all thread
        pthread_cond_broadcast(&cond);
      }
      pthread_mutex_unlock(&mut);
    }
}

int main(int argc, char **argv)
{
  // Check argument
  if (argc != 2)
    {
      printf("Need the NUMBER of thread on argument !\n");
      return 1;
    }

  // Init
  u64 N = atoll(argv[1]);
  if (N > MAX_THREAD)
    return 2;

  pthread_t *tid = malloc(sizeof(pthread_t) * N);

  // Create consumer thread
  for (u64 i = 0; i < N; i++)
    {
      tid[i] = i;
      to_share[i].num_thread = N;
      to_share[i].id_thread = i;
      pthread_create(&(tid[i]), NULL, consume, (void *)&(to_share[i]));
    }

  // Print instruction
  printf("Welcome in this simple producer/consumer program\n");
  printf("Type any word or phrase to produce, and all the threads will display it\n");
  printf("If you want to leave the program you can type: exit\n\n");

  // Launch producer
  produce();

  // Wait thread
  for (u64 i = 0; i < N; i++)
    pthread_join(tid[i], NULL);

  // Exit
  return 0;
}
