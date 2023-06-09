#include <stdio.h>  /* for perror() */
#include <stdlib.h> /* for exit() */

void DieWithError(char *errorMessage) {
  perror(errorMessage);
  exit(1);
}
char *inputString(FILE *fp, size_t size) {
  // The size is extended by the input with the value of the provisional
  char *str;
  int ch;
  size_t len = 0;
  str = realloc(NULL, sizeof(*str) * size); // size is start size
  if (!str)
    return str;
  while (EOF != (ch = fgetc(fp)) && ch != '\n') {
    str[len++] = ch;
    if (len == size) {
      str = realloc(str, sizeof(*str) * (size += 16));
      if (!str)
        return str;
    }
  }
  str[len++] = '\0';

  return realloc(str, sizeof(*str) * len);
}