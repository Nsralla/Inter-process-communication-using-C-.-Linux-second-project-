#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "local.h"

int read_supermarket_config(int configValues[]){
     FILE* file = fopen("supermarket_config.txt", "r"); 

    if (file == NULL) {
        printf("ERROR OPENING THE supermarket config file\n");
        return 0;
    }

    char thresholdName[60];
    int value;

    int i = 0;
    while (fscanf(file, "%s %d", thresholdName, &value) == 2) {
        configValues[i] = value;
        if (++i >= 15) {
            break;
        }
    }
    fclose(file);
    return i;
}

void sendToGUI(int pid ,int sender_type ,int flag, int val ) {
    key_t key;
    int mid;
    MessageGUI msg;

    if ((key = ftok(".", GUISEED )) == -1) {
        perror("Manager: key generation");
        return;
    }

    if ((mid = msgget(key, 0)) == -1) {
        perror("Manager: msgget");
        return;
    }

    msg.msg_type = TO_TEAM;
    msg.pid = pid;
    msg.sender_type = sender_type;
    msg.flag = flag;
    msg.val = val;

    if (msgsnd(mid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("Manager: msgsnd error");
    }
}

int randBetween(int min, int max){
    return rand() % (max - min + 1 ) + min;
}
