#include "../includes/iutf-stream.h"
#include <string.h>
#include <stdlib.h>

IutfStream* stream_open (const char* filename)
{
  FILE* fp = fopen (filename, "r");
  if (!fp) return NULL;

  IutfStream* stream = malloc (sizeof(IutfStream));
  if (!stream) {
    fclose (fp);
    return NULL;
  }

  // читаем весь файл в память
  fseek (fp, 0, SEEK_END)
}
