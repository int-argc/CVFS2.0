/*
    Symbolic link checking and linking
    @author: Jeff
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include "global_defs.h"
#include "cmd_exec.h"
#include "makelink.h"

// make the soft links here, as of now no threading
static int callback(void *notUsed, int argc, char **argv, char **colname) {
	int i;
	string comm = "";

	if(argc != 2) {
		fprintf(stderr, "Error in database table: missing columns\n");
		exit(1);
	} else {
		sprintf(comm, "ln -s \"%s/%s\" \"%s/%s\"", argv[1], argv[0], SHARELOC, argv[0]);
		printf("\nLink Created: '/mnt/Share/%s'",argv[0]);
		printf("comm = %s\n", comm);
		system(comm);
	}

	printf("\n");
	return 0;
}

void makelink() {

	int tcount = 0;
	char *errmsg = 0;
	int rc;

	string file_list = "", comm = "", query = "", comm_out = "";

	// get current contents of share folder
	sprintf(comm, "ls %s | sed -e ':a;N;$!ba;s/\\n/\", \"/g'", SHARELOC);
	// printf("comm = %s\n", comm);
	runCommand(comm, comm_out);
	sprintf(file_list, "\"%s", comm_out);
	int end = strlen(file_list) - 1;
	file_list[end] = '\"';
	printf("File List: %s\n", file_list);
    // open database
/*	printf("Opening database %s\n", DBNAME);
	rc = sqlite3_open(DBNAME, &db);
	if(rc) {
		fprintf(stderr, "Can\'t open database %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
*/
		//check if table is empty or not
		/*strcpy(query, "SELECT count(*) from VolContent");

		rc = sqlite3_prepare_v2(db,query,1000,&res,0);

		if (rc != SQLITE_OK){
			fprintf(stderr, "Failed to fetch data",sqlite3_errmsg(db));
			sqlite3_close(db);
			exit(1);
		}

		rc = sqlite3_step(res);
		if (rc == SQLITE_ROW){
		     tcount = atoi(sqlite3_column_text(res,0));
		}

		strcpy(query,"");*/

	sprintf(query, "SELECT filename, location FROM VolContent WHERE filename NOT IN (%s);", file_list);

	rc = sqlite3_exec(db, query, callback, 0, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
	}


	// close db
	sqlite3_close(db);


}
