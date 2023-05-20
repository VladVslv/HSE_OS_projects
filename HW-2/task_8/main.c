#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include "buffer.h"

const char shm_name[] = "shm"; // name of shared object
int shm_descriptor;            // shared memory descriptor
text *shared_buffer;           // buffer for writing encoding results

char path[] = "main.c";
int start_sem_id; // semaphor to start reading result
int end_sem_id; // ending main semaphore

// keys to semaphores and shared memory
key_t end_key;
key_t start_key;
key_t shm_key;

// function that called when finishing program
void end_program()
{
    // close semaphore
    if (semctl(start_sem_id, 0, IPC_RMID) == -1)
    {
        perror("Can\'t close semaphore ");
        exit(1);
    };
    

    // delete shared memory
    if (shmctl(shm_descriptor, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(1);
    }

    printf("Main program finished.\n");
}

// correct completion of program after SIGINT or SIGTERM
void sig_end(int sig)
{
    end_program();
    exit(1);
}

int main()
{
    // create keys
    start_key = ftok(path, 's');
    end_key = ftok(path, 'e');
    shm_key = ftok(path, 'm');
    signal(SIGINT, sig_end);
    signal(SIGTERM, sig_end);

    struct sembuf wait_buf = {0, -1, SEM_UNDO};
    struct sembuf post_buf = {0, 1, SEM_UNDO};

    printf("Main program started.\n\n");

    // open semaphor
    if ((start_sem_id = semget(start_key, 1, IPC_CREAT | 0666)) == -1)
    {
        perror("Can\'t open semaphore ");
        exit(1);
    }

    // open semaphor
    if ((end_sem_id = semget(end_key, 1, IPC_CREAT | 0666)) == -1)
    {
        perror("Can\'t open semaphore ");
        exit(1);
    }

    printf("Waiting for decoders to finish.\n\n");

    // wait for semaphore to open
    if (semop(start_sem_id, &wait_buf, 1) == -1)
    {
        perror("Can't get semaphor ");
        exit(1);
    }

    // open shared memory
    if ((shm_descriptor = shmget(shm_key, sizeof(text), IPC_CREAT | 0666)) == -1)
    {
        perror("Can\'t open shared memory ");
        exit(1);
    }

    if ((shared_buffer = shmat(shm_descriptor, 0, 0)) == (text *)-1)
    {
        perror("Can\'t acess shared memory ");
        exit(1);
    }
    printf("Start semaphore id = %d\n", start_sem_id);
    printf("End semaphore id = %d\n", end_sem_id);
    printf("Shared memory name = \"%s\", descriptor = %d\n\n", shm_name, shm_descriptor);
    printf("Reading result.\n\n");

    // print result of decoding
    printf("Main pid: %d\nDecoded text result: \n\"", getpid());
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        printf("%*.*s", LINE_LENGTH, LINE_LENGTH, shared_buffer->lines[i].content);
    }

    printf("\"\n\n");

    // detach shared memory
    if (shmdt(shared_buffer) == -1)
    {
        perror("Can\' detach shared memory");
        exit(1);
    }

    // continue decoders program
    if (semop(end_sem_id, &post_buf, 1) == -1)
    {
        perror("Can't get semaphor ");
        exit(1);
    }
    end_program();

    return 0;
}
