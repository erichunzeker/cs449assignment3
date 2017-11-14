#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

//include your code


//replace malloc here with the appropriate version of mymalloc
#define MALLOC my_malloc
//replace free here with the appropriate version of myfree
#define FREE my_free
//define DUMP_HEAP() to be the dump_heap() function that you write
#define DUMP_HEAP() dump_heap()

int main()
{
  char *dummy = MALLOC(23 + 24 * 5);
  FREE(dummy);
  DUMP_HEAP();

  char *this = MALLOC(5);
  char *is = MALLOC(3);
  char *a = MALLOC(2);
  char *test = MALLOC(5);
  char *program = MALLOC(8);
  DUMP_HEAP();

  strcpy(this, "this");
  strcpy(is, "is");
  strcpy(a, "a");
  strcpy(test, "test");
  strcpy(program, "program");
  printf("%s %s %s %s %s\n", this, is, a, test, program);
  DUMP_HEAP();

  FREE(is);
  FREE(test);
  printf("%s %s %s %s %s\n", this, "*", a, "*", program);
  DUMP_HEAP();

  FREE(this);
  printf("%s %s %s %s %s\n", "*", "*", a, "*", program);
  DUMP_HEAP();

  this = MALLOC(5);
  strcpy(this, "this");
  printf("%s %s %s %s %s\n", this, "*", a, "*", program);
  DUMP_HEAP();

  is = MALLOC(3);
  test = MALLOC(5);
  strcpy(is, "is");
  strcpy(test, "test");
  printf("%s %s %s %s %s\n", this, is, a, test, program);
  DUMP_HEAP();

  FREE(test);
  FREE(is);
  FREE(a);
  FREE(this);
  FREE(program);
  printf("%s %s %s %s %s\n", "*", "*", "*", "*", "*");
  DUMP_HEAP();

  return 0;
}

