/*
    Volume Manager
    hiniwalay ko lang
*/

#include <stdio.h>
#include <string.h>
#include "global_defs.h"
#include "volman.h"

// estimate number of initial disks
#define INIT_DISKS_SIZE 50

// not sure with limit
void getInitDisks(string arr[]) {
    FILE *fp = fopen(AV_DISKS, "r");
    string line;
    int i = 0;
    // printf("getting init disks\n");
    while(fgets(line, sizeof(line), fp) != NULL) {
        line[strlen(line) - 1] = '\0';
        strcpy(arr[i++], line);
    }
    strcpy(arr[i], "");
    fclose(fp);
}

int containsDisk(string *disks, string checkDisk) {
    int i = 0;
    // printf("containsDisk\n");
    for(i = 0; strcmp(disks[i], "") != 0; i++) {
        if (strcmp(disks[i], checkDisk) == 0)
            return 1;
    }
    return 0;
}

void makeVolume(int update) {
    char buff[512];
    FILE *partitions = popen("cat /proc/partitions | awk '{print $4}' | grep 'sd[b-z]'", "r");

    char pvs[255];
    char vgs[255];
    char lvs[255];
    char mkfs[255];
    char mount[255];
    char mkdir[255];
    char chmod[255];
    string initialDisks[INIT_DISKS_SIZE];

    int i;
    if(update) {
        getInitDisks(initialDisks);
    }

    while(fgets(buff, sizeof(buff), partitions) != NULL){
       buff[strlen(buff)-1] = '\0';

       // do not create volume again if already existing; alternative: pvdisplay
       if (update) {
           if (containsDisk(initialDisks, buff)) {
               continue;
           }
       }

       strcpy(pvs,"pvcreate /dev/");
       strcat(pvs, buff); //pvcreate /dev/sdb
       strcpy(vgs,"vgcreate vg");
       strcat(vgs, buff);
       strcat(vgs," /dev/");
       strcat(vgs, buff); //vgcreate vgsdb /dev/sdb
       strcpy(lvs,"lvcreate -l 100%FREE -n lv");
       strcat(lvs, buff);
       strcat(lvs," vg");
       strcat(lvs, buff); //lvcreate -l 100%FREE -n lvsdb vgsdb
       // make file system
       strcpy(mkfs, "mkfs -t ext4 /dev/vg");
       strcat(mkfs, buff);
       strcat(mkfs, "/lv");
       strcat(mkfs, buff);
       // mounting
       strcpy(mkdir, "mkdir /mnt/lv");
       strcat(mkdir, buff);
       strcpy(mount, "mount /dev/vg");
       strcat(mount, buff);
       strcat(mount, "/lv");
       strcat(mount, buff);
       strcat(mount, " /mnt/lv");
       strcat(mount, buff);
       strcpy(chmod, "chmod 777 -R /mnt/lv");
       strcat(chmod, buff);

       // execute commands
       system(pvs);
       system(vgs);
       system(lvs);
       system(mkfs);
       system(mkdir);
       system(mount);
       system(chmod);
       // debug
    //    printf("comm=%s\n", pvs);
    //    printf("comm=%s\n", vgs);
    //    printf("comm=%s\n", lvs);
    //    printf("comm=%s\n", mkfs);
    //    printf("comm=%s\n", mkdir);
    //    printf("comm=%s\n", mount);
    //    printf("comm=%s\n", chmod);
    //    printf("\n");
    }
    pclose(partitions);
}


// debug
// int main() {
//
//     // makeVolume(0);
//     // makeVolume(1);
//
//
//     return 0;
// }
