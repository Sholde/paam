/* Include */

#include <stdio.h>   // printf
#include <stdlib.h>  // gettid, malloc, exit...
#include <pthread.h> // pthread

/* Define */

#define N_PARAMETER 3
#define ALIGNED 64

/* Stucture and typedef */

typedef unsigned long long u64;

typedef struct node_s
{
  struct node_s *next;
  u64 value;
} node_t;

typedef struct queue_s
{
  struct node_s *head;
  struct node_s *tail;
} queue_t;

typedef struct share_s
{
  struct queue_s *queue;
  u64 k;
} share_t;

/* Global variable */

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

/* Functions */

void enqueue_mutex(queue_t *queue, u64 value)
{
  node_t *node = aligned_alloc(ALIGNED, sizeof(node_t));
  node->next = NULL;
  node->value = value;

  // Critical section
  pthread_mutex_lock(&mut);
  {
    if (queue->tail == NULL)
      {

        queue->head = node;
        queue->tail = node;
      }
    else
      {
        queue->tail->next = node;
        queue->tail = node;
      }
  }
  pthread_mutex_unlock(&mut);
}

u64 dequeue_mutex(queue_t *queue)
{
  u64 value = 0;

  // Critical section
  pthread_mutex_lock(&mut);
  {
    if (queue->head == NULL)
      {
        printf("Error: dequeue when queue is empty !\n");
        exit(1);
      }
    else
      {
        value = queue->head->value;
        node_t *tmp = queue->head;
        queue->head = queue->head->next;
        free(tmp);

        if (queue->head == NULL)
          queue->tail = NULL;
      }
  }
  pthread_mutex_unlock(&mut);

  return value;
}

void *func_mutex(void *arg)
{
  share_t share = *(share_t *)arg;
  queue_t queue = *(share.queue);
  u64 k = share.k;

  for (u64 i = 0; i < k; ++i)
    {
      enqueue_mutex(&queue, i);
      u64 value = dequeue_mutex(&queue);

      printf("thread: %d, value: %lld\n", gettid(), value);
    }

  return NULL;
}

int main(int argc, char **argv)
{
  if (argc != N_PARAMETER)
    {
      printf("Need the number of thread and the number of iteration in argument\n");
      return 1;
    }

  // Init
  u64 n_thread = atoll(argv[1]);
  pthread_t *tid = aligned_alloc(ALIGNED, sizeof(pthread_t) * n_thread);
  u64 k = atoll(argv[2]);

  // Init structure shared by threads
  share_t share;
  share.queue = aligned_alloc(ALIGNED, sizeof(queue_t));
  share.queue->head = NULL;
  share.queue->tail = NULL;
  share.k = k;

  // Create thread
  for (u64 i = 0; i < n_thread; ++i)
    pthread_create(tid + i, NULL, func_mutex, &share);

  // Wait thread
  for (u64 i = 0; i < n_thread; ++i)
    pthread_join(tid[i], NULL);

  // Release memory
  free(share.queue);
  free(tid);

  return 0;
}
