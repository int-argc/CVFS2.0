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

int callback(void *notUsed, int argc, char **argv, char **colname){

	int i;
	string comm = "";

	char *ptr1;
	ptr1 = strtok(ls_out, " ");

	while (ptr1 != NULL){
		sprintf(comm, "mv /mnt/CVFSTemp/%s %s", ptr1, argv[1]);
		printf("File %s redirected to %s", ptr1, argv[1]);
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
