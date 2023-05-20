#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

const int BUFFER_SIZE = 5000;              // size of buffer
const char *pipe_fs_name = "pipe_fs.fifo"; // first pipe fifo name
const char *pipe_st_name = "pipe_st.fifo"; // second pipe fifo name

int main(int argc, char **argv)
{
  // start of the first process program

  if (argc < 3)
  {
    perror("Not enough arguments (argv[1] - path to input file; argv[2] - path to output file");
    unlink(pipe_fs_name);
    unlink(pipe_st_name);
    exit(-1);
  }

  int input, output, str_size;
  int pipe_fs; // pipe between first and second process
  int pipe_st; // pipe between second and third process
  char buffer[BUFFER_SIZE];
  char result[BUFFER_SIZE];

  // buffer zeroing
  for (size_t i = 0; i < BUFFER_SIZE; i++)
  {
    buffer[i] = ' ';
    result[i] = ' ';
  }

  int result_size = 0;

  // create pipes
  if ((mknod(pipe_fs_name, S_IFIFO | 0666, 0)) < 0)
  {
    perror("First process: can\'t create pipe between first and second processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_st_name);
    exit(-1);
  }
  if ((mknod(pipe_st_name, S_IFIFO | 0666, 0)) < 0)
  {
    perror("First process: can\'t create pipe between second and third processes\n");
    unlink(pipe_fs_name);
    unlink(pipe_st_name);
    exit(-1);
  }

  pid_t second_pid = fork(); // id of the second process

  if (second_pid < 0)
  {
    perror("First process: can\'t create second process");
    unlink(pipe_fs_name);
    unlink(pipe_st_name);
    exit(-1);
  }
  else if (second_pid > 0)
  {
    // countinuation of the first process program

    // open input file
    if ((input = open(argv[1], O_RDONLY, 0666)) < 0)
    {
      perror("First process: can\'t open input file\n");
      unlink(pipe_fs_name);
      unlink(pipe_st_name);
      exit(-1);
    }

    // write to buffer from the input file
    str_size = read(input, buffer, BUFFER_SIZE);
    if (close(input) < 0)
    {
      perror("First process: can\'t close input file\n");
      unlink(pipe_fs_name);
      unlink(pipe_st_name);
      exit(-1);
    }

    // open first pipe (write)
    if ((pipe_fs = open(pipe_fs_name, O_WRONLY)) < 0)
    {
      perror("First process: can\'t open pipe between first and second processes\n");
      unlink(pipe_fs_name);
      unlink(pipe_st_name);
      exit(-1);
    }

    // write to the pipe from buffer
    str_size = write(pipe_fs, buffer, BUFFER_SIZE);
    if (str_size != BUFFER_SIZE)
    {
      printf("First process: can\'t write to the pipe\n");
      unlink(pipe_fs_name);
      unlink(pipe_st_name);
      exit(-1);
    }
    if (close(pipe_fs) < 0)
    {
      perror("First process: can\'t close writing side of the pipe\n");
      unlink(pipe_fs_name);
      unlink(pipe_st_name);
      exit(-1);
    }

    // end of the first process program
  }
  else
  {
    // start of the second process programm

    pid_t third_pid = fork(); // id of the second process
    if (third_pid < 0)
    {
      perror("Second process: can\'t create third process\n");
      unlink(pipe_fs_name);
      unlink(pipe_st_name);
      exit(-1);
    }
    else if (third_pid > 0)
    {
      // continuation of the second process program

      // open first pipe (read)
      if ((pipe_fs = open(pipe_fs_name, O_RDONLY)) < 0)
      {
        perror("Second process: can\'t open pipe between first and second processes\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }

      // write to buffer from the pipe
      str_size = read(pipe_fs, buffer, BUFFER_SIZE);
      if (close(pipe_fs) < 0)
      {
        perror("Second process: can\'t close reading side of the pipe\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }
      if (str_size < 0)
      {
        perror("Second process: can\'t read from the pipe\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
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
      if ((pipe_st = open(pipe_st_name, O_WRONLY)) < 0)
      {
        perror("Second process: can\'t open pipe between second and third processes\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }
      // write result to the pipe
      str_size = write(pipe_st, result, result_size);
      if (str_size != result_size)
      {
        perror("Second process: can\'t write to the pipe");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }
      if (close(pipe_st) < 0)
      {
        perror("Second process: can\'t close writing side of the pipe\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }

      // end of the second process program
    }
    else
    {
      // start of the third process program

      // open second pipe (read)
      if ((pipe_st = open(pipe_st_name, O_RDONLY)) < 0)
      {
        perror("Third process: can\'t open pipe between second and third processes\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }

      // write to buffer from the pipe
      str_size = read(pipe_st, buffer, BUFFER_SIZE);
      if (str_size < 0)
      {
        perror("Third process: can\'t read from the pipe\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }
      if (close(pipe_st) < 0)
      {
        perror("Third process: can\'t close reading side of the pipe\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }

      // open the output file
      if ((output = open(argv[2], O_WRONLY | O_CREAT, 0666)) < 0)
      {
        perror("Third process: can\'t open output file\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }

      // write to output file from the buffer
      str_size = write(output, buffer, BUFFER_SIZE);
      if (str_size != BUFFER_SIZE)
      {
        perror("Third process: can\'t write all strings to the output file\n");
        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }
      if (close(output) < 0)
      {
        perror("Third process: can\'t close output file\n");

        unlink(pipe_fs_name);
        unlink(pipe_st_name);
        exit(-1);
      }

      unlink(pipe_fs_name);
      unlink(pipe_st_name);

      // end of the third process program
    }
  }
  return 0;
}