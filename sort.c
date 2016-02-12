/*
    Redirecting files to Mounted Volumes
    @author: Aidz
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>
#include "global_defs.h"
#include "cmd_exec.h"
#include "makelink.h"
#define DBNAME "cvfs_db"

//Global Variables
string ls_file_out = "";

int callback(void *notUsed, int argc, char **argv, char **colname){

	int i;
	string comm = "", ls_file_size = "", ls_file_size_out = "", mv = "", sql = "", fileloc = "";  
	string filename = "";
	char *ptr1;
	ptr1 = strtok(ls_file_out, "\n");
	int rc;
	
	FILE *fp;

	while (ptr1 != NULL){
        	//get file size of file
       	 	sprintf(ls_file_size, "ls -l /mnt/CVFSTemp/ | grep '%s' | awk '{print $5}'", ptr1);
       	 	runCommand(ls_file_size,ls_file_size_out);

		if (atof(ls_file_size_out) > 536870912.00){
	//		sprintf(fileloc,"/mnt/CVFSTemp/%s",ptr1);
	//		if ((fp = fopen(fileloc,"rb")) == NULL){
	//			printf ("\nError opening file %s\n",ptr1);				
	//		}
	//		ptr1 = strtok(NULL,"\n");
			printf("\nFile: %s | File Size: %s\n", ptr1, ls_file_size_out);
		} else { 
		
	        	printf("\nFile: %s | File Size: %s\n", ptr1, ls_file_size_out);
			printf ("Folder: %s\n", argv[1]);
			sprintf(mv, "mv '/mnt/CVFSTemp/%s' %s", ptr1, argv[1]);  
      			printf("File %s will be redirected to %s\n",ptr1, argv[1]);
			system(mv);        	
			sprintf(sql,"INSERT INTO VolContent (filename,location) values ('%s','%s');", ptr1, argv[1]);				

			rc = sqlite3_exec(db,sql,0,0,0);			
			if (rc != SQLITE_OK){
				fprintf(stderr,"Cant execute command!\n",sqlite3_errmsg(db));
				sqlite3_close(db);
				exit(1);
			}			
			makelink();	
			//Update Target Size Code here//

			strcpy(ls_file_size_out,"");
			ptr1 = strtok(NULL,"\n");
		}
	}
	printf("\n");
	sqlite3_close(db);
	return 0;
}

int main(){
	char *errmsg = 0;
	int rc;
	string ls = "", cp = "", sql = "";
	//open database	
	
	rc = sqlite3_open(DBNAME,&db);
	if (rc != SQLITE_OK){
		fprintf(stderr, "Cant open database %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	
	
	while(1){
		strcpy(ls, "ls /mnt/CVFSTemp");	
		runCommand(ls,ls_file_out);
			
//		printf("\nTemp Contents: %s\n",ls_file_out);

		if (strcmp(ls_file_out,"")){
			strcpy(sql, "SELECT MAX(avspace), mountpt FROM Target;");
	
			rc = sqlite3_exec(db,sql, callback, 0, &errmsg);

			if (rc != SQLITE_OK){
				fprintf(stderr, "SQL Error: %s\n", errmsg);
				sqlite3_free(errmsg);
			}

		}
		strcpy(ls_file_out,"");
	}
	sqlite3_close(db);
	return 0;
}
