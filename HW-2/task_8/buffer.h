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
} text;