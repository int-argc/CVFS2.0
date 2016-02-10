/*
    Redirecting files to Mounted Volumes
    @author: Aidz
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "global_defs.h"
#include "cmd_exec.h"

#define DBNAME "cvfs_db"

string ls_out = "";

//striping files method
void stripeFile(FILE file, char filename){

    FILE *wfile;
    int fcnt = 1;
    string fpartn = "", floc = "";
    char buffer[536870912]; //buffer of 512MB
    while(fread(buffer, 1, 536870912, file) != NULL){ //read 512MB at a time
          sprintf(fpartn,"%s.part%d",filename,fcnt); //rename the name as e.g. thesis.part1, thesis.part2, thesis.part3
          /**this is where the sorting should come in, specifying which folder to write to **/
          /**instead of /mnt/CVFSTemp, change it to mountpt folder**/
          sprintf(floc,"/mnt/CVFSTemp/%s",fpartn); //filelocation of parts
          /**********************************************************************************/
          wfile = fopen(floc,"wb"); //create a file e.g. thesis.part1 at /mnt/CVFSTemp 
          fwrite(buffer, 1, sizeof(buffer), wfile); //write read 512MB on file
          fcnt++; //increment part count
          fclose(wfile);
    }
    fclose(file);

}

int callback(void *notUsed, int argc, char **argv, char **colname){

	int i;
	string comm = "", ls_file_size = "", ls_file_size_out = "", fileloc = "";
    FILE *fptr;
    unsigned int buffer;
  
	char *ptr1;
	ptr1 = strtok(ls_out, " ");

	while (ptr1 != NULL){
        //get file size of file
        sprintf(ls_file_size, "ls -l /mnt/CVFSTemp/ | grep %s | awk '{print $5}'", ptr1);
		runCommand(ls_file_size,ls_file_size_out);
		
		//if current file size > 512MB go to stripeFile()
		if (ls_file_size_out > 536870912){
               strcpy(fileloc,"/mnt/CVFSTemp/");
               strcat(fileloc,ptr1);//store file location to  fileloc
               fptr = fopen(fileloc,"rb"); //open file
               stripeFile(fptr,ptr1); //pass to file pointer and filename to stripe method
        } else {		
               sprintf(comm, "mv /mnt/CVFSTemp/%s %s", ptr1, argv[1]);
		       printf("File %s redirected to %s", ptr1, argv[1]);
        }

		ptr1 = strtok(NULL, " ");
	}

	printf("\n");
	return 0;
}

int main(){
	sqlite3 *db;
	char *errmsg = 0;
	int rc;
	string ls = "", cp = "", sql = "";

	//get current contents of Temp directory
	strcpy(ls,"ls /mnt/CVFSTemp");
	//store contents of temp directory to ls_out	
	runCommand(ls,ls_out);

	//open database
	rc = sqlite3_open(DBNAME,&db);
	if (rc){
		fprintf(stderr, "Cant open database %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	strcpy(sql,  "SELECT MAX(avspace), mountpt FROM Target;");
	
	rc = sqlite3_exec(db,sql, callback, 0, &errmsg);

	if (rc != SQLITE_OK){
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
	}
	sqlite3_close(db);
	return 0;
}
