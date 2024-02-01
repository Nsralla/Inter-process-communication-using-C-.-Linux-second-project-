#include "local.h"
int supermarket_config[CONFIG_SIZE]; 

int main(int argc, char *arg[]){
    int numOfConfig = read_supermarket_config(supermarket_config); 
    int MINIMUM_ARRIVAL_RATE = supermarket_config[6];
    int MAXIMUM_ARRIVAL_RATE = supermarket_config[7];

    int arrivalRate = randBetween(MINIMUM_ARRIVAL_RATE, MAXIMUM_ARRIVAL_RATE);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    char buffer[20];
    strcpy(buffer, arg[1]);
    while(1){  
        switch (fork()) {
            case -1:
            perror("Client: fork");
            return 2;

            case 0:
            execlp("./customer", "customer", buffer, "&", 0);
        
            perror("customer: exec");
            return 3;
        }
        sleep(arrivalRate);  
    }
}