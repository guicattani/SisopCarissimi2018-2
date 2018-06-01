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

