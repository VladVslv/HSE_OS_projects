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
  int str_size;
  int pipe_fs; // pipe between first and second process
  int pipe_sf; // pipe between second and first process
  char buffer[BUFFER_SIZE];
  char result[BUFFER_SIZE];
  
  // buffer zeroing
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    buffer[i] = ' ';
    result[i] = ' ';
  }

  int result_size = 0;

  // open first pipe (read)
  if ((pipe_fs = open(pipe_fs_name, O_RDONLY)) < 0)
  {
    perror("Second process: can\'t open pipe between first and second processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // write to buffer from the pipe
  str_size = read(pipe_fs, buffer, BUFFER_SIZE);
  if (close(pipe_fs) < 0)
  {
    perror("Second process: can\'t close reading side of the pipe\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }
  if (str_size < 0)
  {
    perror("Second process: can\'t read from the pipe\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // search for the required line in buffer
  str_size = strlen(buffer);
  for (int i = 0; i < str_size; ++i)
  {
    if (buffer[i] >= '0' && buffer[i] <= '9')
    {
      if (result_size > 0 && (buffer[i - 1] > '9' || buffer[i - 1] < '0'))
      {
        result[result_size++] = ' ';
        result[result_size++] = '+';
        result[result_size++] = ' ';
        result[result_size++] = buffer[i];
      }
      else if (result_size >= 2 && result[result_size - 1] == '0' && result[result_size - 2] == ' ')
      {
        if (buffer[i] != '0')
        {
          result[result_size - 1] = buffer[i];
        }
      }
      else if (result_size == 1 && result[0] == '0')
      {
        if (buffer[i] != '0')
        {
          result[result_size - 1] = buffer[i];
        }
      }
      else
      {
        result[result_size++] = buffer[i];
      }
    }
  }

  // open second pipe (write)
  if ((pipe_sf = open(pipe_sf_name, O_WRONLY)) < 0)
  {
    perror("Second process: can\'t open pipe between second and first processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }
  // write result to the pipe
  str_size = write(pipe_sf, result, result_size);
  if (str_size != result_size)
  {
    perror("Second process: can\'t write to the pipe");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }
  if (close(pipe_sf) < 0)
  {
    perror("Second process: can\'t close writing side of the pipe\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }
  return 0;
}