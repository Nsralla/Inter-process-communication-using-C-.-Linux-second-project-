#include "local.h"

int mid;
sem_t *sem;
int supermarket_config[CONFIG_SIZE]; 
void initialize_storage(int [],int,int);
void initialize_shelves(int [], int , int);
int check_storage_file(int , int, int );
void signal_catcher(int i);

int main(int argc, char *arg[]){
    
    sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if(sem == SEM_FAILED){
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    key_t       key; 
    if ((key = ftok(".", SEED)) == -1) {    
        perror("CASHIER:  Client: key generation");
        return 1;
    }
    if ((mid = msgget(key, 0 )) == -1 ) {
        mid = msgget(key, IPC_CREAT | 0777);
    }

    int myPid = getpid();
    printf("\nSupermarket[%d]: SUCCESSFULY CREATED REsources. MQ_Id =  %d SM_ID = %d \n", myPid, mid, sem);

    if ( sigset(SIGUSR1, signal_catcher) == SIG_ERR ) { 
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }
    if ( sigset(SIGUSR2, signal_catcher) == SIG_ERR ) { 
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    }
        if ( sigset(SIGINT, signal_catcher) == SIG_ERR ) { 
        perror("Sigset can not set SIGINT");
        exit(SIGINT);
    } 
    int numOfConfig = read_supermarket_config(supermarket_config); 
    int NUM_OF_SHELVING_TEAMS = supermarket_config[2];
    int  NUMOFPRODUCTS = supermarket_config[0];
    int SHELF_AMOUNT_PER_PRODUCT = supermarket_config[1];
    int STORAGE_AMOUNT_PER_PRODUCT = supermarket_config[14];
    int MINIMUM_ARRIVAL_RATE = supermarket_config[6];
    int MAXIMUM_ARRIVAL_RATE = supermarket_config[7];
    int maxRunTime = supermarket_config[10];
    alarm(maxRunTime*60);
    int itemsOnShelf[NUMOFPRODUCTS];
    int storage[NUMOFPRODUCTS];
    initialize_storage(storage, NUMOFPRODUCTS, STORAGE_AMOUNT_PER_PRODUCT); 
    initialize_shelves(itemsOnShelf, NUMOFPRODUCTS, SHELF_AMOUNT_PER_PRODUCT);

    switch (fork()) {
        case -1:
        perror("GUI: fork");
        return 2;

        case 0:    
        execlp("./a.out", "a.out" ,"&", 0);
        perror("GUI: exec");
        return 3;
    }

    for (int i = 0 ; i < NUM_OF_SHELVING_TEAMS; i++){
        switch (fork()) {
            case -1:
            perror("Shelving_team: fork");
            return 2;

            case 0:
            char buffer[20];
            sprintf(buffer, "%d", i);          
            execlp("./team", "team", buffer, "&", 0);
            perror("shelving_team: exec");
            return 3;
        }
    }

    switch (fork()) {
        case -1:
        perror("Client: fork");
        return 2;

        case 0:
        char ppid_str[20];
        sprintf(ppid_str, "%d", myPid);        
        execlp("./forkcustomers", "forkcustomers", ppid_str,"&", 0);
        perror("customer: exec");
        return 3;
    }
    while(1){
        pause();
    }
}

void cleanUp() {
    printf("Supermaket: Cleaning up to exit now!\n");
    if (sem != NULL) {
        sem_close(sem);
        sem_unlink(SEM_NAME);
    }

    if (mid != -1) {
        msgctl(mid, IPC_RMID, (struct msgid_ds *) 0); 
    }
}

void signal_catcher(int i){
    if (i == SIGALRM){
        printf("Just got alarm! Running Time is over. will Exit. \n");
        cleanUp();
        exit(-1000);
    }
    if(i == SIGUSR2){
        cleanUp();
        exit(-500);
    }
    if (i == SIGUSR1){

        printf("Supermarket: Just Recieved Signal, WIll Check File! \n ");

        sem  = sem_open(SEM_NAME,0);
        if(sem == SEM_FAILED){
            perror("sem_open (super market file)");
            exit(EXIT_FAILURE);
        }
        sem_wait(sem);

        FILE *file = fopen(SHELF_FILE, "r+");
        if ( file == NULL){
            perror("fopen (shelves file)");
            exit(EXIT_FAILURE);
        }

        int NUMOFPRODUCTS = supermarket_config[0];
        int RESTOCK_AMOUNT = supermarket_config[5];
        int itemsOnShelf[NUMOFPRODUCTS];
        int locks[NUMOFPRODUCTS];

        
        for(int i =0 ;i<NUMOFPRODUCTS;i++){
            if(fscanf(file,"%d %d", &itemsOnShelf[i], &locks[i]) != 2){
                printf(" IN SUPERMARKET FILE, Failed to read item %d.\n",i);
            }
        }
        
        int RESTOCK_THRESHOLD = supermarket_config[4];
          for(int i =0 ;i<NUMOFPRODUCTS;i++){
            if(itemsOnShelf[i] < RESTOCK_THRESHOLD && (locks[i] == 0)){
                
                locks[i] = 1; 
                MESSAGE msg;
                msg.msg_type = TO_TEAM;
                msg.index = i;
                msg.count =  check_storage_file( NUMOFPRODUCTS, i, RESTOCK_AMOUNT);
                printf("Supermarket: Shelf [%d] has [%d] which is Below the Threshold! Sending Message to Queue to refill %d units \n",i+1, itemsOnShelf[i], msg.count);
                key_t       key; 
                if ((key = ftok(".", SEED)) == -1) {    
                    perror("CASHIER:  Client: key generation");
                    return 1;
                }
                int myMid = msgget(key, 0 );
                int err = msgsnd(myMid, &msg, sizeof(MESSAGE) - sizeof(long), 0);
                if(err == -1){
                    perror("SUPERMARKET: Error sending the message to the Team queue\n");
                    cleanUp();
                    exit(EXIT_FAILURE);
                }
            }
            else if(itemsOnShelf[i] < RESTOCK_THRESHOLD && (locks[i] == 1)){
                printf("Supermarket: Item {%d} Is Below the Threshold, but Satus is 1! \n",i);
            }
        }
        rewind(file);
        for(int i =0 ;i<NUMOFPRODUCTS;i++){
            if( i == NUMOFPRODUCTS-1){
                fprintf(file,"%d %d", itemsOnShelf[i], locks[i]);  
                break;
            }
            fprintf(file,"%d %d\n", itemsOnShelf[i], locks[i]);  
        }
        
        fclose(file);
        
        sem_post(sem);
        sem_close(sem);
        
    }else{
        cleanUp();
        exit(1);
    }
}

void initialize_storage(int storage[], int NUMOFPRODUCTS, int STORAGE_AMOUNT_PER_PRODUCT){
    for (int i =0; i < NUMOFPRODUCTS; i++ ){
        storage[i] = STORAGE_AMOUNT_PER_PRODUCT;
    }

    FILE *file = fopen(STORAGE_FILE, "w");
    if ( file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    for(int i =0;i< NUMOFPRODUCTS; i++){
        if( i == NUMOFPRODUCTS-1){
            fprintf(file,"%d",storage[i]);
            break;
        }
         fprintf(file,"%d\n",storage[i]);
    }

    fclose(file);
}

void initialize_shelves(int itemsOnShelf[], int NUMOFPRODUCTS, int SHELF_AMOUNT_PER_PRODUCT){
    for (int i =0; i< NUMOFPRODUCTS; i++){
        itemsOnShelf[i] = SHELF_AMOUNT_PER_PRODUCT;
    }

    FILE *file = fopen(SHELF_FILE, "w");
    if ( file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    for(int i =0;i< NUMOFPRODUCTS; i++){
        if( i == NUMOFPRODUCTS-1){
            fprintf(file,"%d 0",itemsOnShelf[i]);
            break;
        }
        fprintf(file,"%d 0\n",itemsOnShelf[i]);
    }

    fclose(file);
}

int check_storage_file(int NUMOFPRODUCTS, int index, int RESTOCK_AMOUNT){

        FILE *file = fopen(STORAGE_FILE, "r+");
        if ( file == NULL){
            perror("fopen (storage file)");
            exit(EXIT_FAILURE);
        }
     
        int itemsInStorage[NUMOFPRODUCTS];
        for(int i =0 ;i<NUMOFPRODUCTS;i++){
            if(fscanf(file,"%d", &itemsInStorage[i]) != 1){
                printf(" IN SUPERMARKET FILE, Failed to read storage items %d.\n",i);
            }
        }

        int refillAmount = RESTOCK_AMOUNT > itemsInStorage[index] ? itemsInStorage[index] : RESTOCK_AMOUNT;

        for(int i =0;i< NUMOFPRODUCTS; i++){
            if(i == index){
                itemsInStorage[i] -= refillAmount;
                sendToGUI(i, SENT_TO_MODIFY_FILES, MODIFY_STORAGE, -1*refillAmount);
            }
        }

        rewind(file);
        for(int i =0;i< NUMOFPRODUCTS; i++){
            if( i == NUMOFPRODUCTS-1){
                fprintf(file,"%d",itemsInStorage[i]);
                break;
            }
            fprintf(file,"%d\n",itemsInStorage[i]);
        }
        fclose(file);
        
        int all_empty = 1;
          for(int i =0 ;i<NUMOFPRODUCTS;i++){
            if(itemsInStorage[i] != 0){
                all_empty = 0;
            }
        }
        if (all_empty){
            cleanUp();
            exit(1); 
        }
        return refillAmount;
}
