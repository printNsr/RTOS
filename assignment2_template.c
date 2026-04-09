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

  if (shareddata != NULL) {
    munmap(shareddata, SHARED_MEM_SIZE);
    shareddata = NULL;
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
    perror("error for init thread A");
    exit(1);
  }
if(sem_init(&(params->sem_B), 0, 0) != 0) { // Set up Sem for thread B
    perror("error for init thread B");
    exit(1);
  }
  if(sem_init(&(params->sem_C), 0, 0) != 0) { // Set up Sem for thread C
    perror("error for init thread C");
    exit(1);
  } 

// Initialize thread attributes 
  pthread_attr_init(&attr);
  //TODO: add your code
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

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

  shareddata = (SharedMemoryData*)mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shareddata == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  shareddata->eof = 0;
  memset(shareddata->line, 0, sizeof(shareddata->line));

  return;
}

void* ThreadA(void *params) {
  //TODO: add your code
  ThreadParams *p = (ThreadParams *)params;
  FILE *inputFile = fopen(p->inputFile, "r");
  char line[LINE_SIZE];
  ssize_t bytes;

  if (inputFile == NULL) {
    perror("ThreadA: fopen");
    pthread_exit(NULL);
  }

  while (fgets(line, sizeof(line), inputFile) != NULL) {
    sem_wait(&p->sem_A);

    bytes = write(p->pipeFile[1], line, strlen(line) + 1);
    if (bytes == -1) {
      perror("ThreadA: write");
      sem_post(&p->sem_B);
      break;
    }

    sem_post(&p->sem_B);
  }

  sem_wait(&p->sem_A);
  bytes = write(p->pipeFile[1], EOF_TOKEN, strlen(EOF_TOKEN) + 1);
  if (bytes == -1) {
    perror("ThreadA: write EOF");
  }
  sem_post(&p->sem_B);

  fclose(inputFile);
  pthread_exit(NULL);
}

void* ThreadB(void *params) {
  //TODO: add your code
  ThreadParams *p = (ThreadParams*)params;
  char line[LINE_SIZE];

  while (1) {
    sem_wait(&(p->sem_B));

    memset(line, 0, sizeof(line));
    if (read(p->pipeFile[0], line, sizeof(line)) <= 0) {
      snprintf(line, sizeof(line), "%s", EOF_TOKEN);
    }

    strncpy(shareddata->line, line, sizeof(shareddata->line) - 1);
    shareddata->line[sizeof(shareddata->line) - 1] = '\0';
    shareddata->eof = (strcmp(shareddata->line, EOF_TOKEN) == 0) ? 1 : 0;

    sem_post(&(p->sem_C));

    if (shareddata->eof) {
      break;
    }
  }

  return NULL;
}

void* ThreadC(void *params) {
  //TODO: add your code
  ThreadParams *p = (ThreadParams*)params;
  FILE *fout = fopen(p->outputFile, "w");
  int header_done = 0;
  double sum = 0.0;
  char line[LINE_SIZE];
  int eof = 0;

  if (fout == NULL) {
    perror("Thread C fopen output");
  }

  while (1) {
    sem_wait(&(p->sem_C));

    strncpy(line, shareddata->line, sizeof(line) - 1);
    line[sizeof(line) - 1] = '\0';
    eof = shareddata->eof;

    if (eof) {
      break;
    }

    if (!header_done) {
      if (strncmp(line, "end_header", 10) == 0) {
        header_done = 1;
      }
      sem_post(&(p->sem_A));
      continue;
    }

    if (fout != NULL) {
      fputs(line, fout);
    }
    fputs(line, stdout);
    sum += sum_numbers(line);

    sem_post(&(p->sem_A));
  }

  if (fout != NULL) {
    fclose(fout);
  }

  printf("Thread C: Final sum = %.6f\n", sum);
  return NULL;
}
