// Include
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <hwloc.h>

// Define
#define print_error(string) printf("==error== %s: %s\n", __func__, string)

// Sturcture / typedef
typedef unsigned long long u64;

// Global variable
_Thread_local hwloc_topology_t topology;
u64 test_pu = 0;
u64 test_node = 0;
u64 test_buffer = 0;
u64 test_worload = 0;
char noisy_config[256] = "";
u64 noisy_node = 0;

// Function
void *test(void *arg)
{
  // Setting topology of thread
  assert(hwloc_topology_init(&topology) != -1);  // initialize the topology variable
  assert(hwloc_topology_load(topology) != -1);   // load the topology
  int depth_pu = hwloc_get_type_depth(topology, HWLOC_OBJ_PU); // depth_pu is the id to retrieve processing units
  assert(depth_pu != HWLOC_TYPE_DEPTH_UNKNOWN);
  size_t nb_pus = hwloc_get_nbobjs_by_depth(topology, depth_pu); // number of processing units

  assert(test_pu < nb_pus);
  hwloc_obj_t pu = hwloc_get_obj_by_depth(topology, depth_pu, test_pu);

  assert(hwloc_set_cpubind(topology, pu->cpuset, HWLOC_CPUBIND_THREAD) != -1);

  //
  printf("test - tid: %lu, core: %llu\n", pthread_self(), test_pu);

  for (;;)
    test_pu++;

  printf("%llu\n", test_pu);

  return 0;
}

void *noisy(void *arg)
{
  u64 index_pu = 0;

  if (strcmp(noisy_config, "spread") == 0
      || strcmp(noisy_config, "overload") == 0 )
    {
      // Setting topology of thread
      assert(hwloc_topology_init(&topology) != -1);  // initialize the topology variable
      assert(hwloc_topology_load(topology) != -1);   // load the topology
      int depth_pu = hwloc_get_type_depth(topology, HWLOC_OBJ_PU); // depth_pu is the id to retrieve processing units
      assert(depth_pu != HWLOC_TYPE_DEPTH_UNKNOWN);
      size_t nb_pus = hwloc_get_nbobjs_by_depth(topology, depth_pu); // number of processing units

      srand(time(NULL) + pthread_self());

      do
        {
          index_pu = rand() % nb_pus;
        }
      while (index_pu == test_pu);

      hwloc_obj_t pu = hwloc_get_obj_by_depth(topology, depth_pu, index_pu);

      assert(hwloc_set_cpubind(topology, pu->cpuset, HWLOC_CPUBIND_THREAD)
             != -1);
    }

  printf("noisy - tid: %lu, core: %llu\n", pthread_self(), index_pu);

  return 0;
}

int main(int argc, char **argv)
{
  if (argc != 7)
    {
      print_error("Need 7 arguments: pu, node, buffer, workload, noise config, noise node");
      return 1;
    }

  // Take argument
  test_pu = atoll(argv[1]);
  test_node = atoll(argv[2]);
  test_buffer = atoll(argv[3]);
  test_worload = atoll(argv[4]);
  strcpy(noisy_config, argv[5]);
  noisy_node = atoll(argv[6]);

  // 1 test and 3 noisy
  pthread_t tid[4];

  pthread_create(tid, NULL, test, NULL);

  for (u64 i = 1; i < 4; i++)
    pthread_create(tid + i, NULL, noisy, NULL);

  for (u64 i = 1; i < 4; i++)
    pthread_join(tid[i], NULL);

  pthread_join(tid[0], NULL);

  return 0;
}
