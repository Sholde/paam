/* Include */

#include <stdio.h>     // printf
#include <stdlib.h>    // gettid, malloc, exit...
#include <pthread.h>   // pthread
#include <stdatomic.h> // atomic functions

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

/* Functions */

void enqueue_lock_free(queue_t *queue, u64 value)
{
  node_t *node = aligned_alloc(ALIGNED, sizeof(node_t));
  node->next = NULL;
  node->value = value;

  node_t *old = NULL;
  node_t *nullprt = NULL;

  do
    {
      old = queue->tail;
      while (old->next != NULL)
        {
          atomic_compare_exchange_strong(&(queue->tail), &old, old->next);
          old = queue->tail;
        }
    }
  while (atomic_compare_exchange_strong(&(old->next), &nullprt, node) != 0);

  atomic_compare_exchange_strong(&(queue->tail), &old, node);
}

u64 dequeue_lock_free(queue_t *queue)
{
  node_t *res = NULL;

  do
    {
      res = queue->head;
      if (res->next == NULL)
        return 0;
    }
  while (atomic_compare_exchange_strong(&(queue->head), &res, res->next)
         != (u64)(void *)res);

  return res->next->value;
}

void *func_lock_free(void *arg)
{
  share_t share = *(share_t *)arg;
  queue_t queue = *(share.queue);
  u64 k = share.k;

  for (u64 i = 0; i < k; ++i)
    {
      enqueue_lock_free(&queue, i);
      u64 value = dequeue_lock_free(&queue);

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

  // Init fake node
  node_t *fake = aligned_alloc(ALIGNED, sizeof(node_t));
  fake->next = NULL;
  fake->value = 0;

  // Init structure shared by threads
  share_t share;
  share.queue = aligned_alloc(ALIGNED, sizeof(queue_t));
  share.queue->head = fake;
  share.queue->tail = fake;
  share.k = k;

  // Create thread
  for (u64 i = 0; i < n_thread; ++i)
    pthread_create(tid + i, NULL, func_lock_free, &share);

  // Wait thread
  for (u64 i = 0; i < n_thread; ++i)
    pthread_join(tid[i], NULL);

  // Release memory
  free(share.queue);
  free(fake);
  free(tid);

  return 0;
}
