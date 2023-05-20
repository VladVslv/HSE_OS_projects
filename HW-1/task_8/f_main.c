#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

const int BUFFER_SIZE = 5000;                   // size of buffer
const char *pipe_fs_name = "pipe_f_s_sep.fifo"; // first pipe fifo name
const char *pipe_sf_name = "pipe_s_f_sep.fifo"; // second pipe fifo name

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    perror("Not enough arguments (argv[1] - path to input file; argv[2] - path to output file");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  int input, output, str_size;
  int pipe_fs; // pipe between first and second process
  int pipe_sf; // pipe between second and first process
  char buffer[BUFFER_SIZE];

  unlink(pipe_fs_name);
  unlink(pipe_sf_name);
  // create pipes
  if ((mknod(pipe_fs_name, S_IFIFO | 0666, 0)) < 0)
  {
    perror("First process: can\'t create pipe between first and second processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }
  if ((mknod(pipe_sf_name, S_IFIFO | 0666, 0)) < 0)
  {
    perror("First process: can\'t create pipe between second and first processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // open input file
  if ((input = open(argv[1], O_RDONLY, 0666)) < 0)
  {
    perror("First process: can\'t open input file\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // buffer zeroing
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    buffer[i] = ' ';
  }

  // write to buffer from the input file
  str_size = read(input, buffer, BUFFER_SIZE);
  if (close(input) < 0)
  {
    perror("First process: can\'t close input file\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // open first pipe (write)
  if ((pipe_fs = open(pipe_fs_name, O_WRONLY)) < 0)
  {
    perror("First process: can\'t open pipe between first and second processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // write to the pipe from buffer
  str_size = write(pipe_fs, buffer, BUFFER_SIZE);
  if (str_size != BUFFER_SIZE)
  {
    printf("First process: can\'t write to the pipe\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }
  if (close(pipe_fs) < 0)
  {
    perror("First process: can\'t close writing side of the pipe\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // open second pipe (read)
  if ((pipe_sf = open(pipe_sf_name, O_RDONLY)) < 0)
  {
    perror("First process: can\'t open pipe between second and first processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // write to buffer from the pipe
  str_size = read(pipe_sf, buffer, BUFFER_SIZE);
  if (str_size < 0)
  {
    perror("First process: can\'t read from the pipe\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }
  if (close(pipe_sf) < 0)
  {
    perror("First process: can\'t close reading side of the pipe\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // open the output file
  if ((output = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0)
  {
    perror("First process: can\'t open output file\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // write to output file from the buffer
  str_size = write(output, buffer, BUFFER_SIZE);
  if (str_size != BUFFER_SIZE)
  {
    perror("First process: can\'t write all strings to the output file\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }
  if (close(output) < 0)
  {
    perror("First process: can\'t close output file\n");

    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  unlink(pipe_fs_name);
  unlink(pipe_sf_name);
  return 0;
}