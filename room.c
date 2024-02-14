#include "defs.h"

/* 
    Function: createRoom(char* roomName)
    Purpose:  Dynamically allocates a Room and initializes all values to their default values.
    Params:   
        Input: char* roomName - stores the name of the room being created.
    Return: Room* - returns pointer to the room that was just created.
*/
Room* createRoom(char* roomName){
    Room* temp = malloc(sizeof(Room));
    strcpy(temp->roomName, roomName);
    temp->connectedRooms.head = NULL;
    temp->connectedRooms.tail = NULL;
    temp->connectedRooms.size = 0;
    temp->evidenceList.head = NULL;
    temp->evidenceList.tail = NULL;
    temp->evidenceList.size = 0;
    temp->ghost = NULL;
    sem_init(&(temp->roomHunterMutex), 0, 1);
    sem_init(&(temp->evidenceList.evidenceMutex), 0, 1);
    sem_init(&(temp->roomGhostMutex), 0, 1);
    for(int i = 0; i < NUM_HUNTERS; i++){
        temp->curHunters[i] = NULL;
    }
    return temp;
}

/* 
    Function: connectRooms(Room* room1, Room* room2)
    Purpose:  Connects the two given rooms.
    Params:   
        Input/Output: Room* room1 - points to the first of the two rooms being connected.
        Input/Output: Room* room2 - points to the second of the two rooms being connected.
    Return: void
*/
void connectRooms(Room* room1, Room* room2) {
    //Connects the second room to the first
    addRoom(&(room1->connectedRooms), room2);
    //Connects the first room to the second
    addRoom(&(room2->connectedRooms), room1);
}

/* 
    Function: addRoom(RoomList* roomList, Room* room)
    Purpose:  Adds a given room to a list of rooms.
    Params:   
        Input/Output: RoomList* roomList - points to the list where the room is being added.
        Input: Room* room1 - points to the room being added to the roomlist.
    Return: void
*/
void addRoom(RoomList* roomList, Room* room){
    //Initialize new room node
    RoomNode* new = (RoomNode*) malloc(sizeof(RoomNode));
    new->data = room;
    new->next = NULL;

    //If list is empty
    if(roomList->head == NULL){
        roomList->head = new;
        roomList->tail = new;
    }
    //Else adding to the back
    else{
        roomList->tail->next = new;
        roomList->tail = new;
    }
    roomList->size = (roomList->size) + 1;
}