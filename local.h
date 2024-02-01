#ifndef __LOCAL_H_
#define __LOCAL_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/prctl.h>
#include <time.h>
#include <pthread.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

#define MAX_ITEMS 10
#define CONFIG_SIZE 15
#define SHELF_FILE "shelf.txt"
#define SEM_NAME "fkifhd"
#define SHELF_FILE "shelf.txt"
#define STORAGE_FILE "storage.txt"
#define SEED 'v'
#define GUISEED 'a'
#define TO_TEAM 1L

typedef struct{
    long msg_type;
    int index;
    int count;
} MESSAGE;

#define SENT_BY_CUSTOMER 0
#define SENT_BY_TEAM 1
#define SENT_TO_MODIFY_FILES 2

#define ADD_FLAG 0
#define REMOVE_FLAG 1
#define MODIFY_FLAG 2

#define MODIFY_STORAGE 3
#define MODIFY_SHELVES 4

typedef struct {
    long msg_type;
    int pid;
    int sender_type;
    int flag;  
    int val;    
} MessageGUI;

int read_supermarket_config(int configValues[]);
int randBetween(int min, int max);
void sendToGUI( int pid ,int sender_type ,int flag, int val );

#endif
