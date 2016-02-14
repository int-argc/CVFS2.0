#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "global_defs.h"
#include "cmd_exec.h"

#define DBNAME "cvfs_db"

string ls_file_out = "";

int callback(void *notUsed, int argc, char **argv, char **colname){
 
   string ls_size = "", ls_size_out = "", mv = "";
   char *ptr;

   ptr = strtok(ls_file_out,"\n");
   while (ptr != NULL){
     sprintf(ls_size,"ls -l /mnt/CVFSTemp/ | grep '%s' | awk '{print $5}'", ptr);
     runCommand(ls_size, ls_size_out);

     if (atof(ls_size_out) > 546870912.00){
        //do something here
     } else {
        printf("\nFile: %s | File Size: %s bytes\n", ptr, ls_size_out);
	printf("File redirected to %s\n", argv[1]);
	sprintf(mv,"mv '/mnt/CVFSTemp/%s' '%s/%s'",ptr,argv[1],ptr);
        system(mv);
     }
     strcpy(ls_size_out,"");
     ptr = strtok(NULL,"\n");
   }

   return 0;
}

int main()
{
   char *errmsg = 0;
   int rc;
   string sql = "", ls = "";

   rc = sqlite3_open(DBNAME,&db);
   if (rc != SQLITE_OK){
     fprintf(stderr, "Can't open database %s\n", sqlite3_errmsg(db));
     sqlite3_close(db);
     exit(1);
   }

   strcpy(ls, "ls /mnt/CVFSTemp");

   while (1){
     runCommand(ls, ls_file_out);
     
     //there are still files in CVFSTemp 
     if (strcmp(ls_file_out,"")){
       strcpy(sql,"SELECT MAX(AVSPACE), MOUNTPT FROM TARGET;");

       rc = sqlite3_exec(db, sql, callback, 0, &errmsg);
       if (rc != SQLITE_OK){
          fprintf(stderr, "SQL Error: %s\n", errmsg);
          sqlite3_free(errmsg);
          sqlite3_close(db);
       }
     }
     strcpy(ls_file_out,"");
   }

   return 0;
}

