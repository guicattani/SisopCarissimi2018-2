#include <string.h>

char* subString (const char* input, char* destination, int offset, int length)
{
  int inputLength = strlen (input);

  if (offset + length > inputLength)
  {
     return NULL;
  }

  strncpy (destination, input + offset, length);

  if(destination[length] != '\0')
    destination[length] = '\0';

  return destination;
}

char* rstrstr(char* s1, char* s2) {
  size_t  s1len = strlen(s1);
  size_t  s2len = strlen(s2);
  char *s;

  if (s2len > s1len)
    return NULL;
  for (s = s1 + s1len - s2len; s >= s1; --s)
    if (strncmp(s, s2, s2len) == 0)
      return s;
  return NULL;
}

