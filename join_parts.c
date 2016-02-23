/*
    Striped file assembly
    @author: Jeff
*/

/*
	NOT WORKING!!! PART1 IS NOT JOINED..?
	aidz bakit?????
	di ko talaga malaman saan error!
*/

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

#include "global_defs.h"

#define ASSEMBLYLOC	"/etc/somedir/"	// put some directory name where assembled file will appear

/*
	uses cat command...
*/
static int writePart(void *nref, int argc, char **argv, char **colname) {
	int i;
	string comm = "";
	char *files = (char *)nref;


	if(argc != 2) {
		fprintf(stderr, "Error in database table: missing columns\n");
		exit(1);
	} else {
		string name = "";
		sprintf(name, "\"%s/%s\" ", argv[1], argv[0]);
        strcat(files, name);
	}

	return 0;
}


/*
    filename: base filename
    ASSUMES COLUMN PARTNO ADDED
    ASSUMES DB IS ALREADY OPEN
*/
void assemble(string filename) {
    char *errmsg = 0;
	string files, comm = "", query = "";
	int rc;

    sprintf(query, "SELECT filename, location FROM VolContent WHERE filename LIKE '%s%%' ORDER BY partno;", filename);

	rc = sqlite3_exec(db, query, writePart, files, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
	}

	// combine file parts
	sprintf(comm, "cat %s> %s%s", files, ASSEMBLYLOC, filename);
	printf("comm = %s\n", comm);
	system(comm);

	// close db
	sqlite3_close(db);
}

int main(int argc, char **argv) {
	int rc;
	rc = sqlite3_open(DBNAME, &db);
	if (rc < 0) {
		printf("ERROR OPEN DB\n");
	}
    assemble("abc.txt");
}
