// Project 4 cpmFS - A simple file system 
// by Sajith Muralidhar 

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// importing local files

#include "cpmfsys.h" 
#include "diskSimulator.h"

bool freelist[255]; // declaring global structure 
#define MAX 255

// function to allocate memory for DirStructType and populates it, given a pointer to a buffer of memory holding the contents of disk block (0)
// an integer index which tells which extent from block 0 to use to make the DirStructType value to return 

DirStructType *mkDirStruct(int index, uint8_t *e)
{
index = index * 32; 
int extent_loop = 0;
DirStructType *extent_ptr = malloc(sizeof(DirStructType)); //rather than modifying pointer, point to a variable and return pointer
DirStructType extent;
extent_ptr = &extent;
while(extent_loop < 32){
if(extent_loop == 0)
{
extent.status = e[index];
}
else if(extent_loop <= 8)
{
if(e[index + extent_loop] == ' ')
{
extent.name[extent_loop-1] = '\0';
}
else
{
extent.name[extent_loop-1] = e[index + extent_loop];
}
}
else if(extent_loop <= 11)
{  
extent.extension[extent_loop-9] = '\0';
extent.extension[extent_loop-9] = e[index + extent_loop];
extent.extension[3] = '\0';}
else if (extent_loop > 15) // to build blocks
{
extent.blocks[extent_loop-16] = e[index + extent_loop];
}
switch( extent_loop )
{
case(12):
extent.XL = e[index + extent_loop];
break;
case(13):
extent.BC = e[index + extent_loop];
break;
case(14):
extent.XH = e[index + extent_loop];
break;
case(15):
extent.RC = e[index + extent_loop];
break;
}
extent_loop++;
}
extent.name[8] = '\0';
extent.extension[3] = '\0';
return extent_ptr; // returns pointer value to location 
}

// function to write contents of a DirStructType struct back to the specified index of the extent 
// in block of memory (disk block 0) pointed to by e
void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e)
{
index = index * 32; 
int extent_loop = 0;
for(extent_loop = 0; extent_loop < 32; extent_loop++)
{
if(extent_loop == 0)
{
e[index] = d->status;
}
else if(extent_loop <= 8) //to check name 
{
e[index + extent_loop] =  d->name[extent_loop-1]; 
}
else if(extent_loop <= 11)
{
e[index + extent_loop] = d->extension[extent_loop-9];          
}
else if (extent_loop > 15) // builds blocks
{
e[index + extent_loop] = d->blocks[extent_loop-16]; 
}
switch( extent_loop )
{
case(12):
e[index + extent_loop] = d->XL; 
break;
case(13):
e[index + extent_loop] = d->BC;
break;
case(14):
e[index + extent_loop] = d->XH; 
break;
case(15):
e[index + extent_loop] = d->RC; 
break;
}
}
d->name[8] = '\0';
d->extension[3] = '\0';
          
}

// function to fill FreeList global data structure.
// freeList[i] == true means block[i] is free
// freeList[i] == false means block[i] is in use
void makeFreeList()
{
memset(freelist, true, sizeof freelist); // to make the array free for file contents, declaring freeList as true
uint8_t block0[BLOCK_SIZE];
blockRead(block0,0); // this should be the only block read
DirStructType *extent;
int count = 0;
int i = 0;
while(count<32)
{
extent = mkDirStruct(count,block0);
freelist[0] = false; // block 0 is always in use.
while(i<16)
{
if (extent->blocks[i] != 0 ) // checks if the block is empty
{
freelist[extent->blocks[i]] = false;
}
i++;
}
count++;
}
} 
// prints out the content of freeList in 16 rows of 16 

void printFreeList()
{
printf("\nFREE BLOCK LIST: (* Means in-use)\n");
int i = 0;
while(i<=MAX)
{
if (i % 16 == 0) 
{ 
printf("%4x: ",i); 
}
if (freelist[i] == true)
{
printf(" .");
}
else
{
printf(" *");
}
if (i % 16 == 15) { 
printf("\n"); 
}
i++;
}
printf("\n"); 
}

// function to print file directory to stdout

void cpmDir()
{
int index = 0;
uint8_t block0[BLOCK_SIZE];
blockRead(block0,0);
DirStructType *extent;
printf("\nDIRECTORY LISTING\n");
while(index<32)
{
extent = mkDirStruct(index,block0);
if( extent->status != 229) // 0xe5 is 299 in dec
{   
int b_Count = 0;
if(index == 0 ){b_Count++;} // adding block 0
for(int i = 0; i < 15; i++)
{
if (extent->blocks[i] != 0 ) // counts the non empty blocks
{
b_Count++;
} }
int s = (b_Count-1)*1024+extent->RC*128+extent->BC;
printf("%s.%s %d\n", extent->name, extent->extension,s);
}
index++;
}}

// function to check name of the file
// false = illegal name || true == legal name

bool checkLegalName(char *name)
{
int i = 0;
bool v_file = true;
int name_size = 0;
if (strlen(name) > 12){ v_file = false;}
if(v_file == 1)
{
if((isalpha(name[0]) == 0) && (isdigit(name[0]) == 0)) // to check if file has contents or not
{
v_file = false;
}
while(i<9)
{
if ((name[i] == '.') && (v_file == true))
{
name_size = i+1;
}  
i++;
}
if((strlen(name) - name_size) > 3)
{
v_file = false; 
}
if((isalpha(name[strlen(name) - name_size]) == 0) && (isdigit(name[strlen(name) - name_size]) == 0)) // to check if file exists
{
v_file = false;
}
}
return v_file;
}

// function if it return -1 for illegal name or name not found 
// otherwise returns extent number 0-31

int findExtentWithName(char *name, uint8_t *block0)
{
int index = 0;
int i = 0;
DirStructType *extent;
char file_name[9];
char file_extension[4];
// to build the name in the same format as the same name in extent, if match found return the index 
while(i<9)
{
if(name[i] == '.')
{
file_name[i] = '\0';
}
else if(name[i] == ' ')
{
file_name[i] = '\0';
}
else
{
file_name[i] = name[i];
}
i++;
}
for( index = 0; index < 32; index++)
{
extent = mkDirStruct(index,block0);
if(strcmp(extent->name, file_name) == 0){
return index;
}
}
return -1; // -1 because filename is not found 
}

// The function deletes the file named 'name', and frees its disk blocks in the freeList

int  cpmDelete(char * name)
{  
uint8_t block0[BLOCK_SIZE];
blockRead(block0,0);
DirStructType *extent;
int index = 0;
bool legalname = true;
index = findExtentWithName(name, block0);
legalname = checkLegalName(name);
if(legalname == false)
{
return -2; //filename does not exist
}
if(index < 0)
{
return -1;
}
if(index > 0) //if filename is found
{
extent = mkDirStruct(index,block0);
int rm_block = index*32; 
int extent_loop = 0;
block0[rm_block] = 229; // sets status to 0xe5 
for(extent_loop = 0; extent_loop < 32; extent_loop++)
{
if(extent_loop < 16)
{
if (extent->blocks[extent_loop] != 0)
{ 
freelist[extent->blocks[extent_loop]] = true;
}
}
if(extent_loop > 0){ block0[rm_block+extent_loop] = 0; } //prevents setting status as 0 which means there is content in file
}
blockWrite(block0,0); 
}
else
{
return index;
}
}

// function to read directory block, modifies the extent for file named 'oldname' with 'newname'

int cpmRename(char *oldName, char * newName)
{ 
char file_name[9];
char file_extension[4];
int i = 0;
bool EOFname = false;
int ex_index = 1; 
uint8_t block0[BLOCK_SIZE];
blockRead(block0,0);
DirStructType *extent;
int index = 0;
bool legalname = true;
index = findExtentWithName(oldName, block0);
legalname = checkLegalName(newName);
if(legalname == false) 
{
return -2;
}
if(index < 0) 
{
return -1;
}
if(index >= 0)
{
extent = mkDirStruct(index,block0);
for (i = 0; i < 9; i++)
{
if(EOFname == true)
{
extent->name[i] = '\0';
}
else if(newName[i] == '.')
{
extent->name[i] = '\0';
EOFname = true; 
ex_index += i; 
}
else if(newName[i] == ' ')
{
extent->name[i] = '\0'; 
}
else
{
extent->name[i] = newName[i];
}
}
for ( i=0; i <= 3; i++){
if(newName[i+ex_index] == '.')
{
extent->extension[i] = '\0';
}
else if(newName[i+ex_index] == ' ')
{
extent->extension[i] = '\0';
}
else
{
extent->extension[i] = newName[ex_index+i];
}
}
writeDirStruct(extent, index, block0);
blockWrite(block0,0); 
}
}