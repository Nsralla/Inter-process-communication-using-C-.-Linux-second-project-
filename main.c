#include <raylib.h>
#include "local.h"

typedef struct{
  int id;
  int val;
} Entity;

Entity customers[100];
Entity teams[100];
int shelves[100];
int storage[100];
int customerCounter = 0, teamCounter = 0, shelvesCount = 0, storageCount = 0;
int read_supermarket_config(int configValues[]);
void* myThreadFunction(void* );

int main(void) {
    int supermarket_config[CONFIG_SIZE]; 
    int numOfConfig = read_supermarket_config(supermarket_config); 
    int  NUMOFPRODUCTS = supermarket_config[0];

  FILE *file = fopen(STORAGE_FILE, "r");
    if ( file == NULL){
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    for(int i =0 ;i<NUMOFPRODUCTS;i++){
        if(fscanf(file,"%d", &storage[i]) != 1){
            printf(" IN SUPERMARKET FILE, Failed to read storage items %d.\n",i);
        }
        storageCount++;
    }
    fclose(file);

    FILE *file2 = fopen(SHELF_FILE, "r+");
    if ( file2 == NULL){
        perror("fopen (shelves file)");
        exit(EXIT_FAILURE);
    }
    
    int locks[NUMOFPRODUCTS];
    for(int i =0 ;i<NUMOFPRODUCTS;i++){
        if(fscanf(file2,"%d %d", &shelves[i], &locks[i]) != 2){
            printf(" IN TEAM FILE, Failed to read item %d.\n",i);
        }
        shelvesCount++;
    }

    fclose(file2);

  int screenWidth = 1000;
  int screenHeight = 1000;

  InitWindow(screenWidth, screenHeight, "Supermarket");
  
  Color rectangleColor = SKYBLUE;
  Color textColor = BLACK;

  Font font = LoadFontEx("Roboto-Black.ttf", 20, 0, 0); 

  int width = 175;
  int height = 50;

  SetTargetFPS(60);
    pthread_t myThread;
    if (pthread_create(&myThread, NULL, myThreadFunction, NULL) != 0) {
        return 1;
    }

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (customerCounter > 0){
        char **customerStrings = (char **)malloc(customerCounter * sizeof(char *));
        int baseX = 100;
        int baseY = 10;
        int stepSizeCustomer = screenHeight / customerCounter;
        for (int i = 0 ; i < customerCounter ; i++){
            customerStrings[i] = (char *)malloc(100);
            snprintf(customerStrings[i], 100, "customer[%d]", customers[i].id);
            const char * text = customerStrings[i];
            DrawRectangle(baseX, ( baseY  + stepSizeCustomer*(i+1)) % screenHeight , width, height, rectangleColor);
            DrawTextEx(font, customerStrings[i], (Vector2){baseX , ( baseY  + stepSizeCustomer*(i+1)) % screenHeight}, font.baseSize, 0.0f, textColor);
        }
    }
    
    if(teamCounter > 0){
        char **cashierStrings = (char **)malloc(teamCounter * sizeof(char *));
        int baseCASHX = 300;
        int baseCASHY = 10;
        int stepSizeCashier = screenHeight / teamCounter;
        for (int i = 0 ; i < teamCounter ; i++){
            cashierStrings[i] = (char *)malloc(100);
            snprintf(cashierStrings[i], 100, "Team[%d] working \n\non item[%d]", teams[i].id, teams[i].val+1);
            const char * text = cashierStrings[i];
            DrawRectangle(baseCASHX, ( baseCASHY + stepSizeCashier*(i+1) ) % screenHeight , width, height, rectangleColor);
            DrawTextEx(font, text, (Vector2){baseCASHX , ( baseCASHY + stepSizeCashier*(i+1)  ) % screenHeight }, font.baseSize, 0.0f, textColor);
        }
    }

        if(shelves > 0){
        char **shelveStrings = (char **)malloc(shelvesCount * sizeof(char *));
        int baseCASHX = 600;
        int baseCASHY = 10;
        int stepSizeShelf = screenHeight / shelvesCount;
        for (int i = 0 ; i < shelvesCount ; i++){
            shelveStrings[i] = (char *)malloc(100);
            snprintf(shelveStrings[i], 100, "Shelf[%d] quantity[%d]", i+1, shelves[i]);
            const char * text = shelveStrings[i];
            DrawRectangle(baseCASHX, ( baseCASHY + stepSizeShelf*(i+1) ) % screenHeight , width, height, rectangleColor);
            DrawTextEx(font, text, (Vector2){baseCASHX , ( baseCASHY + stepSizeShelf*(i+1)  ) % screenHeight }, font.baseSize, 0.0f, textColor);
        }
    }

        if(storageCount > 0){
        char **storageStrings = (char **)malloc(storageCount * sizeof(char *));
        int baseCASHX = 800;
        int baseCASHY = 10;
        int stepSizeStorage = screenHeight / storageCount; 
        for (int i = 0 ; i < storageCount ; i++){
            storageStrings[i] = (char *)malloc(100);
            snprintf(storageStrings[i], 100, "Storage[%d] quantity[%d]", i+1, storage[i]);
            const char * text = storageStrings[i];
            DrawRectangle(baseCASHX, ( baseCASHY + stepSizeStorage*(i+1) ) % screenHeight , width, height, rectangleColor);
            DrawTextEx(font, text, (Vector2){baseCASHX , ( baseCASHY + stepSizeStorage*(i+1)  ) % screenHeight }, font.baseSize, 0.0f, textColor);
        }
    }


    EndDrawing();
  }

  UnloadFont(font);
  CloseWindow();
  return 0;
}

void* myThreadFunction(void* args){
    key_t       key; 
    int mid;
    if ((key = ftok(".", GUISEED)) == -1) {    
      perror("GUI : key generation");
      return 1;
    }
    if ((mid = msgget(key, 0 )) == -1 ) {
      mid = msgget(key,IPC_CREAT | 0777);
    }
    printf("\nGUI: SUCCESSFULY CREATED Message Queue. Id =  %d \n", mid);

    while(1){
    MessageGUI msg; 
    int n = msgrcv(mid, &msg, sizeof(msg), TO_TEAM, 0);
    if (msg.sender_type == SENT_BY_CUSTOMER) {
        if (msg.flag == ADD_FLAG) {
        Entity ent;
        ent.id = msg.pid;
        ent.val = msg.val;
        customers[customerCounter] = ent;
        customerCounter++;
        } else if (msg.flag == REMOVE_FLAG) {
            removeEntry(customers, 100, msg.pid);
            customerCounter--;
        }
    } else if (msg.sender_type == SENT_BY_TEAM) {
        if (msg.flag == ADD_FLAG) {
        Entity ent;
        ent.id = msg.pid;
        ent.val = msg.val;
        teams[teamCounter] = ent;
        teamCounter++;
        } else if (msg.flag == REMOVE_FLAG) {
            removeEntry(teams, 100, msg.pid);
            teamCounter--;
        } else if (msg.flag == MODIFY_FLAG) {
            for (int i = 0 ; i < teamCounter; i++){
              if (teams[i].id == msg.pid){
                teams[i].val = msg.val;
              }
            }
        }
    }else if (msg.sender_type == SENT_TO_MODIFY_FILES) {
        if (msg.flag == MODIFY_SHELVES) {
            shelves[msg.pid] += msg.val;
        } else if (msg.flag == MODIFY_STORAGE) {
            storage[msg.pid] += msg.val;
        } 
    }
    }
}

void removeEntry(Entity arr[], int size, int entry) {
  int i;
  for (i = 0; i < size; ++i) {
    if (arr[i].id == entry) {
      break;
    }
  }

  if (i == size) {
    return;
  }

  for (int j = i + 1; j < size; ++j) {
    arr[j - 1] = arr[j];
  }
}

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
