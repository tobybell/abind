#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef void (*thunk_t)();

unsigned char* pool = 0;
unsigned char* next = 0;

thunk_t bind(void* fn, void* aux) {
  unsigned char* thunk = next;
  next += 24;
  thunk[0] = 0x48;
  thunk[1] = 0xbf;
  *(void**) &thunk[2] = aux;
  thunk[10] = 0x48;
  thunk[11] = 0xb8;
  *(void**) &thunk[12] = fn;
  thunk[20] = 0xff;
  thunk[21] = 0xe0;
  return (thunk_t) thunk;
}

typedef struct person {
  const char* name;
  int age;
} person_t;

void greet(person_t* self) {
  printf("Hi, my name's %s and I'm %d.\n", self->name, self->age);
}

int main() {
  // Just initializes a block of memory we can use.
  pool = mmap(0, 4096, PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  next = pool;

  person_t toby = {"Toby", 24};
  person_t xran = {"Xin Ran", 19};
  thunk_t greet1 = bind(&greet, &toby);
  thunk_t greet2 = bind(&greet, &xran);
  greet1();
  greet2();
  greet2();
  greet1();

  // Cleanup.
  munmap(pool, 4096);
  return 0;
}
