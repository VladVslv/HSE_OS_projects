#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>
#include <semaphore.h>
#include <string.h>
#include <sys/stat.h>
#include "buffer.h"

const char shm_name[] = "shm"; // name of shared object
int shm_descriptor;            // shared memory descriptor
text *shared_buffer;           // buffer for writing encoding results

const char start_main_sem_name[] = "start_main_sem";
sem_t *start_main_sem; // semaphore to start reading result
const char end_main_sem_name[] = "end_main_sem";
sem_t *end_main_sem; // ending main semaphore

// function that called when finishing program
void end_program()
{
    // close semaphore
    if (sem_close(start_main_sem) == -1)
    {
        perror("Can\'t close semaphore ");
        exit(1);
    };

    // remove named semaphore
    if (sem_unlink(start_main_sem_name) == -1)
    {
        perror("Can\'t unlink semaphore ");
        exit(1);
    };

    // remove shared memory segment
    shm_unlink(shm_name);

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
    signal(SIGINT, sig_end);
    signal(SIGTERM, sig_end);

    printf("Main program started.\n\n");

    // open end semaphore
    if ((end_main_sem = sem_open(end_main_sem_name, O_CREAT, 0666, 0)) == 0)
    {
        perror("Can\'t open semaphore ");
        exit(1);
    };

    // open start semaphore
    if ((start_main_sem = sem_open(start_main_sem_name, O_CREAT, 0666, 0)) == 0)
    {
        perror("Can\'t open semaphore ");
        exit(1);
    };

    printf("Waiting for decoders to finish.\n\n");

    // wait for decoders to finish
    if (sem_wait(start_main_sem) == -1)
    {
        perror("Can't wait for semaphor ");
        exit(1);
    };

    // open shared memory
    if ((shm_descriptor = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR)) == -1)
    {

        perror("Can\'t open shared memory ");
        exit(1);
    }
    printf("Start semaphore name = \"%s\"\n", start_main_sem_name);
    printf("End semaphore name = \"%s\"\n", end_main_sem_name);
    printf("Shared memory name = \"%s\", descriptor = %d\n\n", shm_name, shm_descriptor);
    printf("Reading result.\n\n");

    shared_buffer = mmap(0, sizeof(text), PROT_WRITE | PROT_READ, MAP_SHARED, shm_descriptor, 0);
    if (shared_buffer == (text *)-1)
    {
        perror("Can\'t acess shared memory ");
        exit(1);
    }

    // print result of decoding
    printf("Main pid: %d\nDecoded text result: \n\"", getpid());
    for (size_t i = 0; i < BUFFER_SIZE; i++)
    {
        printf("%*.*s", LINE_LENGTH, LINE_LENGTH, shared_buffer->lines[i].content);
    }

    printf("\"\n\n");
    close(shm_descriptor);

    // end decoders program
    if (sem_post(end_main_sem) == -1)
    {
        perror("Can't wait for semaphor ");
        exit(1);
    };

    end_program();

    return 0;
}
