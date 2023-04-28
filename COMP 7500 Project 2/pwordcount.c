// COMP 7500 Project 2 - A pipe-based WordCount tool by szm0227
// importing required libraries for this tool
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h> 
#define BUFFER 50000
#define READ_END 0
#define WRITE_END 1

// Creating error function to call when file doesn't exist or when given command is incorrect

void error(int em)
{
 //checking if given command is in correct input format

 if (em == 0)
  {
   printf("\nPlease enter a file name.\n\nUsage: ./pwordcount <filename>\n");
  }

 //checking if file exist or if the file has read privileges

 if (em == 1)
  {
   printf("\nUnable to open file.\n");
   printf("Please check if file exists and you have read privilege.");
  }
 return;
}

//function to count the number of words in a file
int cw(char* array)
{
  int w = 0;
  int i=0;
  while(i<strlen(array))
   {

// Conditions to check space in the file

   if (
       array[i] == '\t' ||
       array[i] == ' ' ||
       array[i] == '\0' ||
       array[i] == '\n')
    {
       w++;
    }i++;
   }
    return w-1;
}

// Creating pipes and running it to find the file and count the number of words in it.

int main(int argc, char *argv[])
{
 int msg;
 char word[BUFFER];
 pid_t pid;
 int fd1[2]; // Pipe 1
 int fd2[2]; // Pipe 2
 FILE * file;
 char buffer[BUFFER];
 char a;
 int c = 0;

 // Check if given input command is correct or not  

 if (argc < 2 || argc > 2)
  {
  // Printing error and exiting the program 
  error(0);
  exit(1);
    }

 // Creating pipes 

 if (pipe(fd1) == -1 || pipe(fd2) == -1)
   {
    fprintf(stderr,"One of the Pipes Failed");
    return 1;
   }

 // fork a child process

 pid = fork();

 // forking error message 

 if (pid < 0)
  {
   fprintf(stderr,"Fork Failed!");
   return 1;
  }

  // Parent process starting, read file and send to child and receive wordcount using pipes 
    
 if (pid > 0) 
  {
   printf("\nProcess 1 is reading file \"%s\" now ... \n", argv[1]);
   file = fopen(argv[1], "r");
   if (file == NULL) //to check if file is open or not
    {
     error(1);
     exit(1);
    }

   while ((a= fgetc(file)) != EOF)
    { //adding characters into an array
     buffer[c] = a;
     c++;
    }
      
  // closing the unused end of the pipe 
  close(fd2[WRITE_END]);
  close(fd1[READ_END]);

  // process 1 should pass the data from the file through the pipe (parent to child) and close the write end of the pipe. 
  
  printf("\nProcess 1 starts sending data to Process 2 ...\n");
  write(fd1[WRITE_END], buffer, BUFFER);
  close(fd1[WRITE_END]);
  int cs;
  waitpid(pid, &cs, 0);

  // reading result from pipe 2 and closing the read end of the pipe.
  read(fd2[READ_END], &msg, sizeof(msg));
  close(fd2[READ_END]);
  printf("\nProcess 1: The total number of words is %d. \n", msg); 
  }

 // child process starts and receives file information from parent using pipes 
 else
  {
    close(fd1[WRITE_END]);
    close(fd2[READ_END]);
    read(fd1[READ_END], word, BUFFER);
    close(fd1[READ_END]);
    printf("\nProcess 2 finishes receiving data from Process 1 ...\n");

    // counting words and sending to pipe 2 

    printf("\nProcess 2 is counting words now ...\n");
    msg = cw(word);
    printf("\nProcess 2 is sending the result back to Process 1 ...\n");
    write(fd2[WRITE_END], &msg, sizeof(msg));
    close(fd2[WRITE_END]);  
  }
    fclose(file);
    return 0;
}
