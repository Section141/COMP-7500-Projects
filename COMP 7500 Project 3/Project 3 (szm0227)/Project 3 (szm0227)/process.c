// This program will be launched in fork_execv 
// COMP7500 part of project 3 
// referred Dr Qin's upload in canvas 
#include <stdio.h> 
#include <unistd.h>
int main(int argc, char *argv[] )
{
  int waiting;
  sscanf(argv[2], "%u", &waiting);
  sleep(waiting);
  return 0;
}