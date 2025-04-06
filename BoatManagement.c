#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME 128
#define SLIP_RATE 12.50
#define LAND_RATE 14.00
#define TRAILER_RATE 25.00
#define STORAGE_RATE 11.20

typedef enum {
    SLIP, LAND, TRAILER, STORAGE
} LocationType;

typedef union {
    int slipNum;          // 1-85
    char bayLetter;       // A-Z
    char trailerTag[7];   // Assuming 6 chars + null
    int storageNum;       // 1-50
} LocationData;

typedef struct {
    char name[MAX_NAME];
    int length;           // up to 100'
    LocationType type;
    LocationData data;
    double amountOwed;
} Boat;

Boat* boats[MAX_BOATS] = {NULL};
int numBoats = 0;

int compareBoats(const void* a, const void* b) {
    Boat* boatA = *(Boat**)a;
    Boat* boatB = *(Boat**)b;
    return strcasecmp(boatA->name, boatB->name);
}

void loadData(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) return;

    char line[256];
    while (fgets(line, sizeof(line), file) && numBoats < MAX_BOATS) {
        Boat* boat = malloc(sizeof(Boat));
        if (!boat) {
            fclose(file);
            exit(1);
        }

        char* token = strtok(line, ",");
        strncpy(boat->name, token, MAX_NAME - 1);
        boat->name[MAX_NAME - 1] = '\0';

        boat->length = atoi(strtok(NULL, ","));
        
        char* typeStr = strtok(NULL, ",");
        if (strcmp(typeStr, "slip") == 0) boat->type = SLIP;
        else if (strcmp(typeStr, "land") == 0) boat->type = LAND;
        else if (strcmp(typeStr, "trailor") == 0) boat->type = TRAILER;
        else boat->type = STORAGE;

        token = strtok(NULL, ",");
        switch (boat->type) {
            case SLIP: boat->data.slipNum = atoi(token); break;
            case LAND: boat->data.bayLetter = token[0]; break;
            case TRAILER: strncpy(boat->data.trailerTag, token, 6); break;
            case STORAGE: boat->data.storageNum = atoi(token); break;
        }

        boat->amountOwed = atof(strtok(NULL, ","));

        boats[numBoats++] = boat;
    }
    fclose(file);
    qsort(boats, numBoats, sizeof(Boat*), compareBoats);
}

void saveData(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    for (int i = 0; i < numBoats; i++) {
        Boat* b = boats[i];
        fprintf(file, "%s,%d,%s,", b->name, b->length,
                b->type == SLIP ? "slip" : 
                b->type == LAND ? "land" : 
                b->type == TRAILER ? "trailor" : "storage");
        
        switch (b->type) {
            case SLIP: fprintf(file, "%d", b->data.slipNum); break;
            case LAND: fprintf(file, "%c", b->data.bayLetter); break;
            case TRAILER: fprintf(file, "%s", b->data.trailerTag); break;
            case STORAGE: fprintf(file, "%d", b->data.storageNum); break;
        }
        fprintf(file, ",%.2f\n", b->amountOwed);
    }
    fclose(file);
}

void printInventory() {
    for (int i = 0; i < numBoats; i++) {
        Boat* b = boats[i];
        printf("%-20s %2d' %7s ", b->name, b->length,
               b->type == SLIP ? "slip" : 
               b->type == LAND ? "land" : 
               b->type == TRAILER ? "trailor" : "storage");
        
        switch (b->type) {
            case SLIP: printf("# %2d", b->data.slipNum); break;
            case LAND: printf("   %c", b->data.bayLetter); break;
            case TRAILER: printf("%6s", b->data.trailerTag); break;
            case STORAGE: printf("# %2d", b->data.storageNum); break;
        }
        printf("   Owes $%7.2f\n", b->amountOwed);
    }
}

int findBoat(const char* name) {
    for (int i = 0; i < numBoats; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) return i;
    }
    return -1;
}

void addBoat(const char* csvLine) {
    if (numBoats >= MAX_BOATS) return;

    Boat* boat = malloc(sizeof(Boat));
    if (!boat) return;

    char temp[256];
    strcpy(temp, csvLine);
    
    char* token = strtok(temp, ",");
    strncpy(boat->name, token, MAX_NAME - 1);
    boat->name[MAX_NAME - 1] = '\0';

    boat->length = atoi(strtok(NULL, ","));
    
    char* typeStr = strtok(NULL, ",");
    if (strcmp(typeStr, "slip") == 0) boat->type = SLIP;
    else if (strcmp(typeStr, "land") == 0) boat->type = LAND;
    else if (strcmp(typeStr, "trailor") == 0) boat->type = TRAILER;
    else boat->type = STORAGE;

    token = strtok(NULL, ",");
    switch (boat->type) {
        case SLIP: boat->data.slipNum = atoi(token); break;
        case LAND: boat->data.bayLetter = token[0]; break;
        case TRAILER: strncpy(boat->data.trailerTag, token, 6); break;
        case STORAGE: boat->data.storageNum = atoi(token); break;
    }

    boat->amountOwed = atof(strtok(NULL, ","));

    boats[numBoats++] = boat;
    qsort(boats, numBoats, sizeof(Boat*), compareBoats);
}

void removeBoat(const char* name) {
    int index = findBoat(name);
    if (index == -1) {
        printf("No boat with that name\n");
        return;
    }
    
    free(boats[index]);
    for (int i = index; i < numBoats - 1; i++) {
        boats[i] = boats[i + 1];
    }
    boats[--numBoats] = NULL;
}

void makePayment(const char* name, double amount) {
    int index = findBoat(name);
    if (index == -1) {
        printf("No boat with that name\n");
        return;
    }
    
    if (amount > boats[index]->amountOwed) {
        printf("That is more than the amount owed, $%.2f\n", boats[index]->amountOwed);
        return;
    }
    
    boats[index]->amountOwed -= amount;
}

void newMonth() {
    for (int i = 0; i < numBoats; i++) {
        double rate = boats[i]->type == SLIP ? SLIP_RATE :
                     boats[i]->type == LAND ? LAND_RATE :
                     boats[i]->type == TRAILER ? TRAILER_RATE : STORAGE_RATE;
        boats[i]->amountOwed += boats[i]->length * rate;
    }
}

void cleanup() {
    for (int i = 0; i < numBoats; i++) {
        free(boats[i]);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    loadData(argv[1]);
    
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");

    char option[2], buffer[256];
    do {
        printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        fgets(option, 2, stdin);
        getchar(); // consume newline
        
        switch (tolower(option[0])) {
            case 'i':
                printInventory();
                break;
            case 'a':
                printf("Please enter the boat data in CSV format                 : ");
                fgets(buffer, 256, stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                addBoat(buffer);
                break;
            case 'r':
                printf("Please enter the boat name                               : ");
                fgets(buffer, 256, stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                removeBoat(buffer);
                break;
            case 'p':
                printf("Please enter the boat name                               : ");
                fgets(buffer, 256, stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                int index = findBoat(buffer);
                if (index != -1) {
                    printf("Please enter the amount to be paid                       : ");
                    double amount;
                    scanf("%lf", &amount);
                    getchar(); // consume newline
                    makePayment(buffer, amount);
                }
                break;
            case 'm':
                newMonth();
                break;
            case 'x':
                break;
            default:
                printf("Invalid option %c\n", option[0]);
        }
    } while (tolower(option[0]) != 'x');

    saveData(argv[1]);
    printf("\nExiting the Boat Management System\n");
    cleanup();
    return 0;
}
