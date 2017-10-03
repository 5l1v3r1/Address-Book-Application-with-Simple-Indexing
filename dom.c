#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

//*********** VARIABLES ****************************
const char * DATA_FILENAME = "datafile.dat";
const char * INDEX_FILENAME = "indexfile.dat";
typedef struct _Address {
    int id;
    char name[20];
    char surname[20];
    char address[40];
    char notes[30];
} Contact; // structure of data file!
typedef struct _Node {
    char name[20];
    char surname[20];
    int id;
    int maxId; // it indicates number of records to have added in our linked list!
    int deleted; // it indicates is deleted node or not!
    int dirtyFlag;
    struct _Node * next;
} Node; // structure of index file!
Node * root; // root of index file!

static int numOfRecord = 1;

//*********** FUNCTIONS ****************************
void menu();
void printList(); // print our linked list in the ram!
void sortLinkedList(Node * newData); // sort our linked list!
void printDatFile(FILE * file); // this function is not used anymore, it is used only my tests. (print context within index file!)
char* concat(char * s1, char * s2); // concat (name + surname) function for strcmp!
void strlower(char * arr); // in order to take lower of the string!
void writeDataIndexFile(); // in order to store the linked list in our ram in the index file.
int loadIndexFile2Ram(); // retrieve our linked list from harddisk!
void setDirtyFlagToHeader(); // write dirty flag in the index file!
void createNewIndexFile(); // in order to create index and data file when application faces dirty flag set!

void addRecord();
void updateRecord();
void deleteRecord();
void findRecord();
void listRecords();

//**************************************

int main(){

    if (loadIndexFile2Ram()) {
        printf("Index file is creating again! \n");
        createNewIndexFile();
    }
    else
        printList();

    while (1) {
        menu();
        char choice;
        scanf(" %c", &choice);

        switch (choice) {
            case '1':
                addRecord();
                printList();
                break;
            case '2':
                updateRecord();
                printList();
                break;
            case '3':
                deleteRecord();
                printList();
                break;
            case '4':
                findRecord();
                break;
            case '5':
                listRecords();
                break;
            case '6':
                if (root != NULL) // if user enters 6 directly.
                    root->dirtyFlag = 0; // successful flushing so dirty flag must be re-set!
                writeDataIndexFile(); // in order to store the linked list in our ram in the index file.
                exit(0);
            default:
                printf("Wrong character! \n");
                break;
        }

    }

}

//**************************************

int loadIndexFile2Ram(){

    FILE * ptIndexFile = fopen(INDEX_FILENAME, "ab+");
    if (ptIndexFile) {

        Node * temp = (Node * ) malloc(sizeof(Node));

        int onlyOne = 1;
        int maxId;
        while (fread(temp, sizeof(Node), 1, ptIndexFile) != 0){

            if (onlyOne) {
                if (temp->dirtyFlag == 1){
                    //printf("!!! The file is not closed properly so it requires reorganization. \n");
                    return 1;
                }

                root = temp;
                maxId = temp->maxId;
                numOfRecord = maxId;
                onlyOne = 0;
            }

            if (maxId != 1) {
                maxId--;
                temp->next = (Node *) malloc(sizeof(Node));
                //printf("%s %s %d %d için geldi , nexti null değil! \n", temp->name, temp->surname, temp->id, temp->maxId);
                temp = temp->next;
            }

            temp->next = NULL;

            //printf("%s %s %d %d \n", temp.name, temp.surname, temp.id, temp.maxId);
        }
        fclose(ptIndexFile);

    }
    printf("\n\nLoaded index file from the harddisk : \n \n");
    return 0;
}

void menu(){
    printf("\nMy address book application \n\n");
    printf("Main Menu \n");
    printf("===================== \n");
    printf("  1. Add New Record \n");
    printf("  2. Update A Record \n");
    printf("  3. Delete A record \n");
    printf("  4. Find A Record \n");
    printf("  5. List Records \n");
    printf("  6. Exit \n\n");
    printf("Enter your choice ==> _ \n");
}

void addRecord(){

    Contact contact;

    // scanf takes as the first parameter special regular expression
    // so scanf keeps taking in values until it encounters a '\n'-- newline, so spaces get saved as well

    printf("Enter a name : ");
    scanf(" %[^\n]s",contact.name);
    printf("Enter a surname : ");
    scanf(" %[^\n]s", contact.surname);
    printf("Enter a address : ");
    scanf(" %[^\n]s", contact.address);
    printf("Enter a note : ");
    scanf(" %[^\n]s", contact.notes);

    if (contact.name[0] != '\0' &&
        contact.surname[0] != '\0' &&
        contact.address[0] != '\0'){

        FILE * ptDataFile = fopen(DATA_FILENAME,"ab+");
        // open append & binary mode because we have to provide entry sequenced order.!

        if (ptDataFile) {

            // We assume all record have to be lower case in the background.
            // Thanks to this, we dont face any problem in strcmp function.
            strlower(contact.name);
            strlower(contact.surname);

            if (root == NULL){  // index file has no data!
                root = (Node *) malloc(sizeof(Node));
                root->dirtyFlag = 1;
                contact.id = numOfRecord; // the first item id must be start value of 1.
                // we already create numOfRecord as global and start value as 1.
                strcpy(root->name , contact.name);
                strcpy(root->surname , contact.surname);
                root->id = contact.id;
                root->next = NULL;
            }
            else {
                Node * iter = root;
                while (iter){// searching for duplicate node?
                    if (strcmp(iter->name,contact.name) == 0 && strcmp(iter->surname, contact.surname) == 0) {
                        printf("DUPLICATE KEY FOUNDED! \n");
                        return;
                    }
                    iter = iter->next;
                }
                contact.id = ++numOfRecord;

                Node * newData = (Node *) malloc(sizeof(Node));
                strcpy(newData->name , contact.name);
                strcpy(newData->surname , contact.surname);
                newData->id = contact.id;

                sortLinkedList(newData); // we have to arrange our linked list so i passed newdata my sort function
            }

            root->maxId = contact.id; // maxId indicates number of record which user add so we have to store this value.
            // in index file root the first line so it necessary to store this value in root node!

            printf("\nRecord has been added succesfully! \n");
            fwrite(&contact, sizeof(contact),1,ptDataFile);
            fclose(ptDataFile);

            // dirty flag header must be set in the index file header! so
            setDirtyFlagToHeader();

        }
        else
            fprintf(stderr,"Data file hasn't been created. \n");
    }
    else
        fprintf(stderr,"This record is invalid! Try again. \n");

}
void updateRecord(){

    listRecords();

    Contact old; Contact new;

    printf("Enter a old name which you want to update : ");
    scanf(" %[^\n]s",old.name);
    printf("Enter a old surname which you want to update : ");
    scanf(" %[^\n]s", old.surname);

    printf("Enter a new name : ");
    scanf(" %[^\n]s", new.name);
    printf("Enter a new surname : ");
    scanf(" %[^\n]s", new.surname);
    printf("Enter a new address : ");
    scanf(" %[^\n]s", new.address);
    printf("Enter a new note : ");
    scanf(" %[^\n]s", new.notes);

    //is there any duplicate name and surname correspond to new values given from user?
    Node * temp = root;
    while(temp){
        if (strcmp(temp->name, new.name) == 0 && strcmp(temp->surname, new.surname) == 0){
            fprintf(stderr,"New name and surname is not allowed. We already have this name and surname. \n");
            return;
        }
        temp = temp->next;
    }


    temp = root;
    while (temp){

        if (strcmp(temp->name, old.name) == 0 && strcmp(temp->surname, old.surname) == 0){// primary key kontrols!

            FILE * ptDataFile = fopen(DATA_FILENAME, "rb+");

            if (ptDataFile){
                fseek(ptDataFile, (temp->id-1)*sizeof(Contact), SEEK_SET); // position beginning of the specific id!

                //We update the new data for data file!
                Contact contact;
                fread(&contact, sizeof(Contact), 1, ptDataFile);
                strcpy(contact.name, new.name);
                strcpy(contact.surname, new.surname);
                strcpy(contact.address, new.address);
                strcpy(contact.notes, new.notes);
                fflush(ptDataFile);
                fseek(ptDataFile, -sizeof(Contact), SEEK_CUR);
                fwrite(&contact, sizeof(Contact),1,ptDataFile);
                fclose(ptDataFile);
                //

                // we create new node for the new data! and we will add this data to our linked list!
                Node * updatedRecord = (Node *) malloc(sizeof(Node));
                strcpy(updatedRecord->name, contact.name);
                strcpy(updatedRecord->surname, contact.surname);
                updatedRecord->id = temp->id; // id doesnt change so we can assign directly!
                updatedRecord->next = NULL;

                Node * travelList = root;

                if (root->id == updatedRecord->id){ // if old node is root node, we have to change root!

                    char * updateValues = concat(updatedRecord->name, updatedRecord->surname);
                    char * rootValues = concat(root->name, root->surname);

                    char * rootNextValues;
                    if (root->next != NULL)
                        rootNextValues = concat(root->next->name, root->next->surname);

                    if (strcmp(updateValues, rootValues) < 0 || (root->next != NULL && strcmp(updateValues, rootNextValues)) < 0){
                        // maybe root is directly new data!
                        updatedRecord->maxId = root->maxId;
                        updatedRecord->next = root->next;
                        root = updatedRecord;
                        updatedRecord->dirtyFlag = 1;
                    }
                    else {
                        // maybe root is the root->next;
                        if (root->next != NULL) {
                            root->next->maxId = root->maxId;
                            root->next->dirtyFlag = 1;
                            root = root->next;
                            sortLinkedList(updatedRecord);
                        }
                        else { // suppose that we just have a one node (a) we want to update a to b! so this condition provides this action!
                            updatedRecord->maxId = root->maxId;
                            updatedRecord->next = root->next;
                            updatedRecord->dirtyFlag = 1;
                            root = updatedRecord;
                        }
                    }
                }
                else {
                    // if root is the same
                    Node *travelAgain = root; // in order to delete old index node.

                    while (travelAgain) {
                        if (travelAgain->next != NULL && travelAgain->next->id == updatedRecord->id) {
                            travelAgain->next = temp->next;
                            break;
                        }
                        travelAgain = travelAgain->next;
                    }
                    // delete and sort!
                    sortLinkedList(updatedRecord);
                }
                // linked list is changed so we must set dirty flag!
                setDirtyFlagToHeader();
            }

        }
        temp = temp->next;
    }
    printList();

}
void deleteRecord(){

    Node delete;

    printf("Enter a name which you want to delete: ");
    scanf(" %[^\n]s",delete.name);
    printf("Enter a surname which you want to delete : ");
    scanf(" %[^\n]s", delete.surname);

    Node * temp = root;
    int lineNumber = 0; // which node in our linked list to be deleted?
    while (temp){

        if (strcmp(temp->name, delete.name) == 0 && strcmp(temp->surname, delete.surname) == 0) {

            FILE * ptIndexFile = fopen(INDEX_FILENAME, "rb+");

            if (ptIndexFile){

                root->maxId--; // remember maxId is like numOfRecord, so we have to decrease this value because
                // we deleted a record!
                temp->deleted = 1; // mark as deleted.

                Node * travel = root; // in order to delete this node in linked list

                if (travel->deleted == 1){ // if this node is root , we have to change root!
                    if (root->next != NULL) {
                        root->next->maxId = root->maxId; // we have to save this value for loadIndexFile2Ram
                        root->next->dirtyFlag = 1;
                        root = root->next;
                    }
                    else {
                        root = NULL;
                    }
                }
                else{
                    while (travel){
                        if (travel->next != NULL && travel->next->deleted == 1){
                            travel->next = temp->next;
                        }
                        travel = travel->next;
                    }
                }

                setDirtyFlagToHeader();

                fclose(ptIndexFile);
            }
        }
        lineNumber++;
        temp = temp->next;
    }


}
void findRecord(){

    listRecords();

    Node find;

    printf("Enter a name which you want to find: ");
    scanf(" %[^\n]s",find.name);
    printf("Enter a surname which you want to find : ");
    scanf(" %[^\n]s", find.surname);

    Node * temp = root;
    while (temp) {

        if (strcmp(temp->name, find.name) == 0 && strcmp(temp->surname, find.surname) == 0) {

            FILE * ptDataFile = fopen(DATA_FILENAME, "rb+");

            if (ptDataFile) {
                fseek(ptDataFile, (temp->id-1) * sizeof(Contact), SEEK_SET); // position beginning of the specific id!
                Contact contact;
                fread(&contact, sizeof(Contact), 1, ptDataFile);

                printf("Founded record: Name: %s, Surname: %s, Adddress: %s, Notes: %s \n",contact.name, contact.surname,contact.address,contact.notes);
                return;
            }

        }
        temp = temp->next;
    }
    printf("NOT FOUND!");

}
void listRecords(){

    char c;
    printf("Enter a one-character-long input to list records: ");
    scanf(" %c", &c);

    Node * temp = root;
    printf("Founded records: \n");
    while (temp){

        if (temp->name[0] == c){

            printf("Record NAME: %s and SURNAME: %s \n",temp->name, temp->surname);
            /*
            FILE * ptDataFile = fopen(DATA_FILENAME, "rb+");
            if (ptDataFile) {
                fseek(ptDataFile, (temp->id-1) * sizeof(Contact), SEEK_SET); // position beginning of the specific id!
                Contact contact;
                fread(&contact, sizeof(Contact), 1, ptDataFile);
                printf("Name: %s, Surname: %s, Adddress: %s, Notes: %s \n",contact.name, contact.surname,contact.address,contact.notes);
            }*/

        }
        temp = temp->next;
    }


}

void printList() {
    Node * ptr = root;
    printf(" [ \n");

//start from the beginning
    while(ptr != NULL) {
        printf("LinkedList in RAM: (NAME : %s, SURNAME: %s, ID: %d , %d) \n",ptr->name,ptr->surname,ptr->id, ptr->maxId);
        ptr = ptr->next;
    }
    printf(" ]\n");
}
void sortLinkedList(Node * newData){

    char * new = concat(newData->name, newData->surname);
    Node * temp = root;
    char * old = concat(temp->name, temp->surname);

    while (temp){

        if (strcmp(new,old) > 0){

            if (temp->next != NULL){
                old = concat(temp->next->name, temp->next->surname);
                if (strcmp(new,old) > 0){
                    temp = temp->next;
                }
                else{
                    newData->next = temp->next;
                    temp->next = newData;
                    break;
                }
            }
            else{
                temp->next = newData;
                newData->next = NULL;
                break;
            }
        }
        else {
            newData->next = root;
            newData->maxId = root->maxId;
            newData->dirtyFlag = 1;
            root = newData;
            break;
        }

    }

}
void printDatFile(FILE * ptFile){
    fseek(ptFile, 0, SEEK_SET);
    while (1){
        Node temp;
        int n = fread(&temp, sizeof(temp), 1, ptFile);
        if (n == 0)
            break;
        if (temp.deleted == 0)
            printf("printDATFILE : %s %s %d %d \n", temp.name, temp.surname, temp.id, temp.maxId);
    }
}

char* concat(char * s1, char * s2){
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}
void strlower(char * arr){
    while (*arr = tolower(*arr)) arr++;
}

void writeDataIndexFile(){

    FILE * ptIndexFile = fopen(INDEX_FILENAME, "wb");

    // write and binary. Truncate to zero length or create file for writing.
    if (ptIndexFile){
        Node * ptr = root;
        while(ptr != NULL) {
            Node node;
            node.id = ptr->id;
            strcpy(node.name,ptr->name);
            strcpy(node.surname,ptr->surname);
            node.maxId = root->maxId;
            node.deleted = 0;
            if (ptr->deleted == 0)
                fwrite(&node, sizeof(node),1,ptIndexFile);
            ptr = ptr->next;
        }
        fclose(ptIndexFile);
    }

}

void setDirtyFlagToHeader(){
    FILE * ptIndexFile = fopen(INDEX_FILENAME, "wb");
    if (root != NULL)
        fwrite(root, sizeof(Node),1,ptIndexFile);
    fclose(ptIndexFile);
}
void createNewIndexFile(){
    FILE * ptDataFile = fopen(DATA_FILENAME,"rb");
    if (ptDataFile){
        int maxId = 0;
        int onlyOne = 1;
        while (1){

            Contact contact;
            int n = fread(&contact, sizeof(Contact), 1, ptDataFile);
            if (n == 0)
                break;
            maxId++;
            if (onlyOne){
                root = (Node *) malloc(sizeof(Node));
                strcpy(root->name, contact.name);
                strcpy(root->surname, contact.surname);
                root->id = contact.id;
                root->next = NULL;
                onlyOne = 0;
            }else {
                Node * newData = (Node *) malloc(sizeof(Node));
                strcpy(newData->name , contact.name);
                strcpy(newData->surname , contact.surname);
                newData->id = contact.id;

                sortLinkedList(newData);
            }

        }
        fclose(ptDataFile);
        root->maxId = maxId;
        numOfRecord = maxId;
        printList();
    }
}