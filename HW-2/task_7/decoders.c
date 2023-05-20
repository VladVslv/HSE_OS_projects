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
#include "buffer.h"
#include <sys/stat.h>

// function to get symbol from code aka encoding table
char get_char(int code)
{
    return (char)code;
}

const char shm_name[] = "shm"; // name of shared object
int shm_descriptor;            // shared memory descriptor
text *shared_buffer;           // buffer for writing encoding results

const char start_main_sem_name[] = "start_main_sem";
sem_t *start_main_sem; // semaphore to start main
const char end_main_sem_name[] = "end_main_sem";
sem_t *end_main_sem; // ending main semaphore

const char decoder_sem_name[] = "decoder_sem";
sem_t *decoder_sem; // decoder semaphore

// function that called when finishing program
void end_program()
{
    // close semaphore
    if (sem_close(decoder_sem) == -1)
    {
        perror("Can\'t close semaphore ");
        exit(1);
    };

    // remove named semaphore
    if (sem_unlink(decoder_sem_name) == -1)
    {
        perror("Can\'t unlink semaphore ");
        exit(1);
    };

    // close semaphore
    if (sem_close(end_main_sem) == -1)
    {
        perror("Can\'t close semaphore ");
        exit(1);
    };

    // remove named semaphore
    if (sem_unlink(end_main_sem_name) == -1)
    {
        perror("Can\'t unlink semaphore ");
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

    printf("Decoders program started.\n\n");

    // open decoder semaphore
    if ((decoder_sem = sem_open(decoder_sem_name, O_CREAT, 0666, 1)) == 0)
    {
        perror("Can\'t open semaphore ");
        exit(1);
    };

    // open main semaphore
    if ((start_main_sem = sem_open(start_main_sem_name, O_CREAT, 0666, 0)) == 0)
    {
        perror("Can\'t open semaphore ");
        exit(1);
    };

    // open main semaphore
    if ((end_main_sem = sem_open(end_main_sem_name, O_CREAT, 0666, 0)) == 0)
    {
        perror("Can\'t open semaphore ");
        exit(1);
    };

    shm_unlink(shm_name);

    // open shared memory
    if ((shm_descriptor = shm_open(shm_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR)) == -1)
    {

        perror("Can\'t open shared memory ");
        exit(1);
    }
    printf("Decoder semaphor name = \"%s\"\n", decoder_sem_name);
    printf("Start main semaphor name = \"%s\"\n", start_main_sem_name);
    printf("End main semaphor name = \"%s\"\n", end_main_sem_name);
    printf("Shared memory name = \"%s\", descriptor = %d\n\n", shm_name, shm_descriptor);

    // truncate
    if (ftruncate(shm_descriptor, sizeof(text)) == -1)
    {
        perror("Can\'t truncate ");
        exit(1);
    }

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
        else if (child_pid == 0) // for children
        {
            // acess shared memory
            shared_buffer = mmap(0, sizeof(text), PROT_WRITE | PROT_READ, MAP_SHARED, shm_descriptor, 0);
            if (shared_buffer == (text *)-1)
            {
                perror("Can\'t acess shared memory ");
                exit(1);
            }

            // wait for semaphore to open
            if (sem_wait(decoder_sem) == -1)
            {
                perror("Can't wait for semaphor ");
                exit(1);
            };

            printf("Decoder number: %d, pid: %d\nDecoded text: ", i, getpid());

            // decode to shared memory
            for (size_t j = 0; j < LINE_LENGTH; j++)
            {
                shared_buffer->lines[i].content[j] = get_char(codes[i][j]);
            }
            printf("\n\"%*.*s\"\n\n", LINE_LENGTH, LINE_LENGTH, shared_buffer->lines[i].content);

            // post semaphore
            if (sem_post(decoder_sem) == -1)
            {
                perror("Can\'t post semaphore");
                exit(1);
            };
            close(shm_descriptor);
            exit(0);
        }
    }

    // wait for all child processes to finish
    while (wait(NULL) > 0)
        ;

    printf("Decoding finished.\n\n");

    // start reading from main
    if (sem_post(start_main_sem) == -1)
    {
        perror("Can\'t post semaphore");
        exit(1);
    };

    // wait for main to finish reading
    if (sem_wait(end_main_sem) == -1)
    {
        perror("Can't wait for semaphor ");
        exit(1);
    };

    end_program();

    return 0;
}
