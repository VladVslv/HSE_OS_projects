
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <semaphore.h>

#define BUFFER_SIZE 10 // number of child processes
#define LINE_LENGTH 50 // length of code for each child process

// result of each child process decoding
struct line
{
    char content[LINE_LENGTH];
};

// decoding result
typedef struct
{
    struct line lines[BUFFER_SIZE];
    sem_t main_sem; // semaphore
} text;

// function to get symbol from code aka encoding table
char get_char(int code)
{
    return (char)code;
}

const char shm_name[] = "shm"; // name of shared object
int shm_descriptor;            // shared memory descriptor
text *shared_buffer;           // buffer for writing encoding results

// function that called when finishing program
void end_program()
{

    shared_buffer = mmap(0, sizeof(text), PROT_WRITE | PROT_READ, MAP_SHARED, shm_descriptor, 0);

    if (shared_buffer == (text *)-1)
    {
        perror("Can\'t acess shared memory ");
        exit(1);
    }

    // destroy semaphore
    if (sem_destroy(&shared_buffer->main_sem) == -1)
    {
        perror("Can\'t close semaphore ");
        exit(-1);
    };
    close(shm_descriptor);

    // remove shared memory segment
    shm_unlink(shm_name);

    printf("Program finished.\n");
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

    printf("Program started.\n");

    shm_unlink(shm_name);

    // open shared memory
    if ((shm_descriptor = shm_open(shm_name, O_CREAT | O_RDWR, 0666)) == -1)
    {

        perror("Can\'t open shared memory ");
        exit(1);
    }

    printf("Shared memory name = \"%s\", descriptor = %d\n\n", shm_name, shm_descriptor);

    // truncate
    if (ftruncate(shm_descriptor, sizeof(text)) == -1)
    {
        perror("Can\'t truncate ");
        exit(1);
    }

    shared_buffer = mmap(0, sizeof(text), PROT_WRITE | PROT_READ, MAP_SHARED, shm_descriptor, 0);

    if (shared_buffer == (text *)-1)
    {
        perror("Can\'t acess shared memory ");
        exit(1);
    }

    // open semaphore
    if ((sem_init(&shared_buffer->main_sem, shm_descriptor, 1)) == -1)
    {
        perror("Can\'t create semaphore ");
        exit(1);
    };

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
            shared_buffer = mmap(0, sizeof(text), PROT_WRITE | PROT_READ, MAP_SHARED, shm_descriptor, 0);
            if (shared_buffer == (text *)-1)
            {
                perror("Can\'t acess shared memory ");
                exit(1);
            }

            // wait for semaphore to open
            if (sem_wait(&shared_buffer->main_sem) == -1)
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
            if (sem_post(&shared_buffer->main_sem) == -1)
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

    end_program();
    return 0;
}
