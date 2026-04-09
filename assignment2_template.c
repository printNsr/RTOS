/***********************************************************************************/
//***********************************************************************************
//            *************NOTE**************
// This is a template for the subject of RTOS in University of Technology Sydney(UTS)
// Please complete the code based on the assignment requirement.

//***********************************************************************************
/***********************************************************************************/

/*
  To compile assign2_template-v3.c ensure that gcc is installed and run 
  the following command:

  gcc your_program.c -o your_ass-2 -lpthread -lrt -Wall
*/

#include  <pthread.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <sys/types.h>
#include  <fcntl.h>
#include  <string.h>
#include  <sys/stat.h>
#include  <semaphore.h>
#include  <sys/time.h>
#include  <sys/mman.h> 

/* to be used for your memory allocation, write/read. man mmsp */
#define SHARED_MEM_NAME "/my_shared_memory"
#define SHARED_MEM_SIZE 1024

// line size and end of file token for controlled threat shutdown
#define LINE_SIZE 255
#define EOF_TOKEN "__EOF__"

/* --- Structs --- */
typedef struct ThreadParams {
  int pipeFile[2]; // [0] for read and [1] for write. use pipe for data transfer from thread A to thread B
  sem_t sem_A, sem_B, sem_C; // the semphore
  char message[255];
  char inputFile[100]; // input file name
  char outputFile[100]; // output file name
} ThreadParams;

typedef struct SharedMemoryData {
  int eof;
  char line[LINE_SIZE];
}
SharedMemoryData;

pthread_attr_t attr;

int shm_fd;// use shared memory for data transfer from thread B to Thread C 

static SharedMemoryData *shareddata = NULL;

/* --- Prototypes --- */

/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

/* This thread reads data from data.txt and writes each line to a pipe */
void* ThreadA(void *params);

/* This thread reads data from pipe used in ThreadA and writes it to a shared variable */
void* ThreadB(void *params);

/* This thread reads from shared variable and outputs non-header text to src.txt */
void* ThreadC(void *params);

static double sum_numbers(const char*line) {
  double total = 0.0;
  double value = 0.0;
  int consumed = 0;
  const char *p = line;

  while (sscanf(p, "%lf%n", &value, &consumed) == 1) {
    total += value;
    p += consumed;
  }
  return total;
}

/* --- Main Code --- */
int main(int argc, char const *argv[]) {

  pthread_t tid[3]; // three threads
  ThreadParams params;
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
    return 1;
  }

  // Initialization
  initializeData(&params);

  snprintf(params.inputFile, sizeof(params.inputFile), "%s", argv[1]);
  snprintf(params.outputFile, sizeof(params.outputFile), "%s", argv[2]);

  // Create Threads
  // TODO: add your code

  if (pthread_create(&(tid[0]), &attr, &ThreadA, (void *)(&params)) != 0) {
    perror("pthread_create ThreadA");
    return 1;
  }

  // TODO: add your code
  if (pthread_create(&(tid[1]), &attr, &ThreadB, (void *)(&params)) != 0) {
    perror("pthread_create ThreadB");
    return 1;
  }

  if (pthread_create(&(tid[2]), &attr, &ThreadC, (void *)(&params)) != 0) {
    perror("pthread_create ThreadC");
    return 1; 
  }

  // Wait on threads to finish
  pthread_join(tid[0], NULL);

  // TODO: add your code
  pthread_join(tid[1], NULL);
  pthread_join(tid[2], NULL);

  sem_destroy(&(params.sem_A));
  sem_destroy(&(params.sem_B));
  sem_destroy(&(params.sem_C));
  pthread_attr_destroy(&attr);

  close(params.pipeFile[0]);
  close(params.pipeFile[1]);

  if (sharedData != NULL) {
    munmap(sharedData, SHARED_MEM_SIZE);
    sharedData = NULL;
  }

  if (shm_fd >= 0) {
    close(shm_fd);
    shm_unlink(SHARED_MEM_NAME);
  }

  return 0;
}

void initializeData(ThreadParams *params) {
  // Initialize Sempahores
  if(sem_init(&(params->sem_A), 0, 1) != 0) { // Set up Sem for thread A
    perror("error for init threa A");
    exit(1);
  }
if(sem_init(&(params->sem_B), 0, 0) != 0) { // Set up Sem for thread B
    perror("error for init threa B");
    exit(1);
  }
  if(sem_init(&(params->sem_C), 0, 0) != 0) { // Set up Sem for thread C
    perror("error for init threa C");
    exit(1);
  } 

// Initialize thread attributes 
  pthread_attr_init(&attr);
  //TODO: add your code
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  memset(params->message, 0, sizeof(params->message));
  memset(params->inputFile, 0, sizeof(params->inputFile));
  memset(params->outputFile, 0, sizeof(params->outputFile));

  if (pipe(params->pipeFile) == -1) {
    perror("pipe");
    exit(1);
  }

  shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  if (ftruncate(shm_fd, SHARED_MEM_SIZE) == -1) {
    perror("ftruncate");
    exit(1);
  }

  sharedData = (SharedMemoryData*)mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (sharedData == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  sharedData->eof = 0;
  memset(sharedData->line, 0, sizeof(sharedData->line));

  return;
}

void* ThreadA(void *params) {
  //TODO: add your code
  
printf("Thread A: sum = %d\n", sum);
}

void* ThreadB(void *params) {
  //TODO: add your code

  printf("Thread B: sum = %d\n", sum);
}

void* ThreadC(void *params) {
  //TODO: add your code

 printf("Thread C: Final sum = %d\n", sum);
}