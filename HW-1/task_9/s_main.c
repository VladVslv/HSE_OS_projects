#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

const int BUFFER_SIZE = 5000;
const int TEMP_BUFFER_SIZE = 5;                 // size of buffer
const char *pipe_fs_name = "pipe_f_s_sep.fifo"; // first pipe fifo name
const char *pipe_sf_name = "pipe_s_f_sep.fifo"; // second pipe fifo name

int main(int argc, char **argv)
{
  int str_size;
  int pipe_fs; // pipe between first and second process
  int pipe_sf; // pipe between second and first process
  char main_buffer[BUFFER_SIZE];

  char result[BUFFER_SIZE];
  char temp_buffer[TEMP_BUFFER_SIZE];

  // buffer zeroing
  for (int i = 0; i < BUFFER_SIZE; i++)
  {
    main_buffer[i] = ' ';
    result[i] = ' ';
  }

  int main_buffer_size = 0;
  int result_size = 0;

  // open first pipe (read)
  if ((pipe_fs = open(pipe_fs_name, O_RDONLY)) < 0)
  {
    perror("Second process: can\'t open pipe between first and second processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  do
  {
    // write to buffer from the pipe
    str_size = read(pipe_fs, temp_buffer, TEMP_BUFFER_SIZE);

    if (str_size < 0)
    {
      perror("Second process: can\'t read from the pipe\n");
      unlink(pipe_fs_name);
      unlink(pipe_sf_name);
      exit(-1);
    }

    // write new info to the main buffer
    for (int i = 0; i < str_size; i++)
    {
      main_buffer[i + main_buffer_size] = temp_buffer[i];
    }
    main_buffer_size += str_size;
  } while (str_size == TEMP_BUFFER_SIZE);

  if (close(pipe_fs) < 0)
  {
    perror("Second process: can\'t close reading side of the pipe\n");
    unlink(pipe_fs_name);
    unlink(pipe_sf_name);
    exit(-1);
  }

  // search for the required line in buffer
  str_size = main_buffer_size;
  for (int i = 0; i < str_size; ++i)
  {
    if (main_buffer[i] >= '0' && main_buffer[i] <= '9')
    {
      if (result_size > 0 && (main_buffer[i - 1] > '9' || main_buffer[i - 1] < '0'))
      {
        result[result_size++] = ' ';
        result[result_size++] = '+';
        result[result_size++] = ' ';
        result[result_size++] = main_buffer[i];
      }
      else if (result_size >= 2 && result[result_size - 1] == '0' && result[result_size - 2] == ' ')
      {
        if (main_buffer[i] != '0')
        {
          result[result_size - 1] = main_buffer[i];
        }
      }
      else if (result_size == 1 && result[0] == '0')
      {
        if (main_buffer[i] != '0')
        {
          result[result_size - 1] = main_buffer[i];
        }
      }
      else
      {
        result[result_size++] = main_buffer[i];
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

  int current_position = 0; // index of the first not written element of result
  while (current_position < result_size)
  {
    for (int i = current_position; i < current_position + TEMP_BUFFER_SIZE && i < result_size; i++)
    {
      temp_buffer[i - current_position] = result[i];
    }

    // write result to the pipe
    str_size = write(pipe_sf, temp_buffer, (result_size - current_position) > TEMP_BUFFER_SIZE ? TEMP_BUFFER_SIZE : result_size - current_position);
    if (str_size < 0)
    {
      perror("Second process: can\'t write to the pipe\n");
      unlink(pipe_fs_name);
      unlink(pipe_sf_name);
      exit(-1);
    }
    current_position = (current_position + TEMP_BUFFER_SIZE) > result_size ? result_size : current_position + TEMP_BUFFER_SIZE;
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