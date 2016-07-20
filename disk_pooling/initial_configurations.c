/*
    THIS IS TEMP VERSION 2: diskname array solution
    CVFS Inital Configuration
    @author: Aidz and Jeff           :">
    requires:
        open-iscsi
        nmap
        sqlite3
        libsqlite3-dev
    uses:
        cmd_exec.c
        volman.c
*/

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <syslog.h>

#include "../Global/global_definitions.h"
#include "../Utilities/cmd_exec.h"
#include "../volume_management/make_volumes.h"
#include "initial_configurations.h"

#include <stdlib.h>

#define TARGET_FILE "target_list.txt"
#define GBSTR  "GiB"
#define MBSTR  "MB"    // assumption
#define TBSTR  "TB"

double toBytes(String sUnit) {
    char *ptr;

    ptr = strtok(sUnit, " ");
    double val = atof(ptr);
    ptr = strtok(NULL, " ");
    if (strcmp(ptr, GBSTR) == 0) {
        val *= 1000000000;
    } else if (strcmp(ptr, MBSTR) == 0) {
        val *= 1000000;
    } else if (strcmp(ptr, TBSTR) == 0) {
        val *= 1000000000000;
    }
    // syslog(LOG_INFO, "DiskPooling: val = %f\n", val);
    return val;
}

void initialize() {

    system("clear");


    String ip = "", netmask = "", command = "", alltargetsStr = "", currtarget = "", iqn = "", sendtargets = "", sql = "";
    String disklist = "", assocvol = "", mountpt = "", avspace = "", command1 = "", sql1 = "";


    int counter = 1;
    //sqlite3 information
    sqlite3 *db;
    int rc = 0;

    //open sqlite3 db
    rc = sqlite3_open(DBNAME,&db);
    if (rc){
       printf("\nCannot open database.\n");
       exit(0);
    }

    // get network information of initiator
    // what if interface is not eth0? what if eth1...?
    runCommand("ip addr show eth0 | grep \'inet \' | awk \'{print $2}\' | cut -f1 -d\'/\'", ip);
    runCommand("ip addr show eth0 | grep \'inet \' | awk \'{print $2}\' | cut -f2 -d\'/\'", netmask);
    // printf("*****  Network information  *****\n");
    ip[strlen(ip)] = '\0';
    netmask[strlen(netmask)] = '\0';
    syslog(LOG_INFO, "DiskPooling: Initiator IP address: %s/%s\n\n", ip, netmask);


    // do nmap for active hosts with port 3260 open
    // syslog(LOG_INFO, "DiskPooling: Scanning network...\n");
    sprintf(command, "nmap -v -n %s%s%s -p 3260 | grep open | awk '/Discovered/ {print $NF}'", ip, "/", netmask);
    runCommand(command, alltargetsStr);

    // discover iscsi targets on hosts scanned successfully
    char *ptr;
    ptr = strtok(alltargetsStr, "\n");

    while(ptr != NULL) {
    	printf("%s is a target.\n", ptr);
    	sprintf(sendtargets,"iscsiadm -m discovery -t sendtargets -p %s | awk '{print $2}'",ptr);
    	runCommand(sendtargets,iqn);
        printf("%s\n", iqn);
        String login = "";
        sprintf(login, "iscsiadm -m node -l -T %s -p %s:3260", iqn, ptr);
	printf("Login = %s\n", login);
        system(login);        
        sleep(3);
        sprintf(sql,"insert into Target(tid, ipadd,iqn) values (%d, '%s','%s');",counter, ptr,iqn);

        printf("** initconf, sql = %s\n", sql);

        rc = sqlite3_exec(db,sql,0,0,0);
        if (rc != SQLITE_OK){
            printf("\nDid not insert successfully!\n");
            exit(0);
        }
        strcpy(sendtargets,"");
        strcpy(sql,"");
        strcpy(iqn, "");
        ptr = strtok(NULL, "\n");
        counter++;
    }
    //sleep(5);
    //printf("\n\nLogging in to targets...");
    //system("iscsiadm -m node --login");
    printf("\n\nAvailable partitions written to file \"%s\"\n", AV_DISKS);
    sleep(5);
    sprintf(command, "cat /proc/partitions > '%s'", AV_DISKS);
    system(command);
    system("cat /proc/partitions");

    makeVolume(0);

    system("cat '../file_transaction/AvailableDisks.txt' | grep sd[b-z] | awk '{print $4}' > SDNAME.txt");

    FILE *fp;
    String l;

    fp = fopen("SDNAME.txt", "r");
    if (fp == NULL) {
        printf("file not found\n");
        exit(1);
    }

    //while(fscanf(fp, "%s", l) != EOF) {
     //   printf("read: %s\n", l);
    //}

    // syslog(LOG_INFO, "DiskPooling: DONE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    // syslog(LOG_INFO, "DiskPooling: Disklist before: %s\n\n", disklist);
    //strcat(disklist, "\n");
    // syslog(LOG_INFO, "DiskPooling: PTR Before: %s\n\n", ptr1);
    // syslog(LOG_INFO, "DiskPooling: DIskList after: %s\n\n", disklist);
    counter = 1;
    while(fscanf(fp, "%s", l) != EOF) {
    //    syslog(LOG_INFO, "DiskPooling: PTR: %s\n\n", ptr1);
    //    syslog(LOG_INFO, "DiskPooling: INSIDE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("this line is read: %s\n", l);
       strcat(assocvol,"/dev/vg");
       strcat(assocvol,l);
       strcat(assocvol,"/lv");
       strcat(assocvol,l);


       strcat(mountpt,"/mnt/lv");
       strcat(mountpt,l);

       sprintf(command1,"lvdisplay %s | grep 'LV Size' | awk '{print $3,$4}'",assocvol);

       runCommand(command1,avspace);

       // edit here not sure if working (assume: avspace = "12.3 GiB")
       double space_bytes = toBytes(avspace);

       sprintf(sql1,"update Target set assocvol = '%s', mountpt = '%s', avspace = %lf where tid = %d", assocvol, mountpt, space_bytes, counter);
       printf("just checking sql1 = %s\n", sql1);
    //    syslog(LOG_INFO, "DiskPooling: SQL1 = %s\n", sql1);
       rc = sqlite3_exec(db,sql1,0,0,0);

       if (rc != SQLITE_OK){
           syslog(LOG_INFO, "DiskPooling: Error: Target is not recorded");
           exit(0);
       }

       strcpy(assocvol,"");
       strcpy(mountpt,"");
       strcpy(avspace,"");
       counter++;
    //    pp = strtok(NULL,"\n");
    //    printf("***************\n\n\n%s\n\n\n", ptr1);
    }

    printf("closign file\n");
    fclose(fp);

    printf("\n\nInitialization finished\n");
}
