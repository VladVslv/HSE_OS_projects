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

// function to get symbol from code aka encoding table
char get_char(int code)
{
    return (char)code;
}

const char shm_name[] = "shm"; // name of shared object
int shm_descriptor;            // shared memory descriptor
text *shared_buffer;           // buffer for writing encoding results

char path[] = "main.c";
int decoders_sem_id;
int start_sem_id; // semaphor to start reading result
int end_sem_id; // ending main semaphore

// keys to semaphores and shared memory

key_t end_key;
key_t start_key;
key_t shm_key;
key_t decoders_key;

// function that called when finishing program
void end_program()
{
    // close semaphore
    if (semctl(end_sem_id, 0, IPC_RMID) == -1)
    {
        perror("Can\'t close semaphore ");
        exit(1);
    };

    // close semaphore
    if (semctl(decoders_sem_id, 0, IPC_RMID) == -1)
    {
        perror("Can\'t close semaphore ");
        exit(1);
    };

    printf("Decoders program finished.\n");
}

// correct completion of program after SIGINT or SIGTERM
void sig_end(int sig)
{
    end_program();
    exit(1);
}

int main()
{
    signal(SIGINT, sig_end);
    signal(SIGTERM, sig_end);
    start_key = ftok(path, 's');
    end_key = ftok(path, 'e');
    shm_key = ftok(path, 'm');
    decoders_key = ftok(path, 'd');

    printf("Decoders program started.\n\n");

    struct sembuf wait_buf = {0, -1, SEM_UNDO};
    struct sembuf post_buf = {0, 1, SEM_UNDO};

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

    // open semaphor
    if ((decoders_sem_id = semget(decoders_key, 1, IPC_CREAT | 0666)) == -1)
    {
        perror("Can\'t open semaphore ");
        exit(1);
    }

    // set semaphor value to 1
    if (semctl(decoders_sem_id, 0, SETVAL, 1) == -1)
    {
        perror("Can\'t set semaphor value");
        exit(1);
    }

    // open shared memory
    if ((shm_descriptor = shmget(shm_key, sizeof(text), IPC_CREAT | 0666)) == -1)
    {
        perror("Can\'t open shared memory ");
        exit(1);
    }

    printf("Decoder semaphor id = %d\n", decoders_sem_id);
    printf("Start main semaphor id = %d\n", start_sem_id);
    printf("End main semaphor id = %d\n", end_sem_id);
    printf("Shared memory name = \"%s\", descriptor = %d\n\n", shm_name, shm_descriptor);

    // code for each child process
    int codes[BUFFER_SIZE][LINE_LENGTH];

    // read codes from file
    FILE *input = fopen("../input.txt", "r");

    if (input == NULL)
    {
        perror("Can\'t open input file");
        exit(1);
    }
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        for (int j = 0; j < LINE_LENGTH; j++)
        {
            fscanf(input, "%d", &codes[i][j]);
        }
    }
    fclose(input);

    printf("Decoding started.\n\n");

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        pid_t child_pid = fork();

        if (child_pid == -1)
        {
            perror("Can\'t create child process");
            exit(1);
        }
        else if (child_pid == 0) 
        {

            if ((shared_buffer = shmat(shm_descriptor, 0, 0)) == (text *)-1)
            {
                perror("Can\'t acess shared memory ");
                exit(1);
            }

            // wait for semaphore to open
            if (semop(decoders_sem_id, &wait_buf, 1) == -1)
            {
                perror("Can't get semaphor ");
                exit(1);
            }

            printf("Decoder number: %d, pid: %d\nDecoded text: ", i, getpid());

            // decode to shared memory
            for (size_t j = 0; j < LINE_LENGTH; j++)
            {
                shared_buffer->lines[i].content[j] = get_char(codes[i][j]);
            }
            printf("\n\"%*.*s\"\n\n", LINE_LENGTH, LINE_LENGTH, shared_buffer->lines[i].content);

            // post seamphore
            if (semop(decoders_sem_id, &post_buf, 1) == -1)
            {
                perror("Can\'t post semaphore");
                exit(1);
            }

            // detach shared memory
            if (shmdt(shared_buffer) == -1)
            {
                perror("Can\' detach shared memory");
                exit(1);
            }

            exit(0);
        }
    }

    // wait for all child processes to finish
    while (wait(NULL) > 0)
        ;

    printf("Decoding finished.\n\n");

    // start reading from main
    if (semop(start_sem_id, &post_buf, 1) == -1)
    {
        perror("Can\'t post semaphore");
        exit(1);
    }

    // wait for main process to finish program
    if (semop(end_sem_id, &wait_buf, 1) == -1)
    {
        perror("Can\'t post semaphore");
        exit(1);
    }

    end_program();

    return 0;
}
