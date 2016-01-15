/*
    command executor for cvfs
    @author: Jeff
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global_defs.h"
#include "cmd_exec.h"

/*
    runs a command, writes command output to out
    comm: command to run
    out: output string
*/
void runCommand(string comm, string out) {
    FILE *fp;

    fp = popen(comm, "r");
    if(fp == NULL) {
        printf("runCommand: error fp is NULL\n");
        exit(1);    // exits program on error
    }


    string line;
    /* read the output */
    while(fgets(line, sizeof(line) - 1, fp) != NULL) {
        strcat(out, line);
    }
    out[strlen(out) - 1] = '\0';
    pclose(fp);
    // return 0;
}

/*
    runs a command, writes command output to file
    comm: command to run
    filePath: filename command output is written to
*/
void writeFromCommand(string comm, string filePath) {
    string cmdOut = "";
    FILE *fp;

    runCommand(comm, cmdOut);

    fp = fopen(filePath, "w");
    fprintf(fp, "%s", cmdOut);

    fclose(fp);
    // return 0;
}
