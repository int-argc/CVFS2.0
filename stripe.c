#include <stdio.h>
#include <stdlib.h>
#include "global_defs.h"
#include "stripe.h"
#include "sort.h"

void stripe(string filename)
{
   FILE *fptr;
   string fileloc = "", filepart = "", partname = "";
   size_t size = 536870912;
   size_t result;
   size_t filesize;
   size_t totalRead = 0;
   char *buffer;  
   int counter = 1;

   printf("%s will be striped.\n",filename);
   sprintf(fileloc, "/mnt/CVFSTemp/%s",filename);
   fptr = fopen(fileloc,"rb");

   if (fptr == NULL){
      fputs("Unable to open file\n",stderr);
      exit(1);
   }
   fseek(fptr,0,SEEK_END);
   filesize = ftell(fptr); //get file size
   fseek(fptr,0,SEEK_SET);
 
   //printf("Size: %L",filesize);
   buffer = (char*) malloc (sizeof(char)*size);
   if (buffer == NULL){
     fputs("Unable to allocated memory\n",stderr);
     exit(1);
   }
  
   while( (result =  fread(buffer,sizeof(char),size,fptr)) > 0 ){
   
      sprintf(filepart,"/mnt/CVFSTemp/%s.part%d",filename,counter);
      sprintf(partname,"%s.part%d",filename,counter);
      FILE *fptr1 = fopen(filepart,"wb");
      fwrite(buffer,1,result,fptr1);
      printf("%s written!\n",filepart);
      sort(filepart);
      fclose(fptr1);
      counter++;
      totalRead += result;
      
      if (filesize - totalRead < 536870912){ //less than 512MB to read
          sprintf(filepart, "/mnt/CVFSTemp/%s.part%d",filename, counter);
          sprintf(partname,"%s.part%d",filename,counter);
          FILE *fptr2 = fopen(filepart,"wb");
          fwrite(buffer,1,filesize-totalRead,fptr2);
          printf("%s written!\n", filepart);
          sort(filepart);
          fclose(fptr2);
          free(buffer);
      }
   }
    
   fclose(fptr);
}
