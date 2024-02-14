#include "defs.h"

/* 
    Function: initProgram(HouseType* house, Ghost* ghost)
    Purpose:  Initialize the program so the game can run.
    Params:   
        Input/Output: HouseType* house - points to the house beinging initialized.
        Input/Output: Ghost* ghost - points to the ghost being initialized.
    Return: void
*/
void initProgram(HouseType* house, Ghost* ghost){
    char hunterNames[NUM_HUNTERS][MAX_STR];
    //Ask the user to input 4 hunter names
    getNames(hunterNames);
    // Initialize the random number generator
    srand(time(NULL));
    initHouse(house);
    populateRooms(house);
    //Initialize hunters & Place hunters in head of our room list
    for(int i = 0; i < NUM_HUNTERS; i++){
        house->curHunters[i] = initHunter(hunterNames[i], house->rooms.head->data, i, &(house->sharedEvidence));
    }
    //Adds the hunter to the van's list of current hunters when it is initialized
    for(int i = 0; i < NUM_HUNTERS; i++) {
        if(house->rooms.head->data->curHunters[i] == NULL) {
            house->rooms.head->data->curHunters[i] = &(house->curHunters[i]);
        }
    }
    initGhost(&(house->rooms), ghost);
}

/* 
    Function: getNames(char hunterNames[][MAX_STR])
    Purpose:  Get the names of the hunters from the user.
    Params:   
        Output: char hunterNames[][MAX_STR] - stores all of the hunter's names entered by the user.
    Return: void
*/
void getNames(char hunterNames[][MAX_STR]){
    char hunterNumber[NUM_HUNTERS][MAX_STR] = {"first", "second", "third", "fourth"};
    for(int i = 0; i < NUM_HUNTERS; i++){
        printf("Enter the name of the %s hunter: ", hunterNumber[i]);
        fgets(hunterNames[i], MAX_STR, stdin);
        hunterNames[i][strlen(hunterNames[i])-1] = 0;    
    }
}

/* 
    Function: runThreads(HouseType* house, Ghost* ghost)
    Purpose:  Runs all of the threads for the program (ghost and hunter's threads).
    Params:   
        Input: HouseType* house - points to the house storing all of the hunters.
        Input: Ghost* ghost - points to the ghost.
    Return: void
*/
void runThreads(HouseType* house, Ghost* ghost){
    pthread_t ghostThread;
    pthread_t hunterIDs[NUM_HUNTERS];
    //Creating ghost thread
    pthread_create(&ghostThread, NULL, runGhost, (void*) ghost);
    //Creating hunter threads
    for(int i = 0; i < NUM_HUNTERS; i++){
        pthread_create(&hunterIDs[i], NULL, runHunter, (void*) &(house->curHunters[i]));
    }
    //Waiting for all threads to complete
    pthread_join(ghostThread, NULL);
    for(int i = 0; i < NUM_HUNTERS; i++){
        pthread_join(hunterIDs[i], NULL);
    }
}

/* 
    Function: printEnd(HouseType house, GhostClass actualType)
    Purpose:  Prints the ending sequence/results after the program.
    Params:   
        Input: HouseType* house - points to the house storing all of the hunters.
        Input: Ghost* ghost - points to the ghost.
    Return: void
*/
void printEnd(HouseType house, GhostClass actualType){
    printEndIntro(house.curHunters);
    
    EvidenceType found[DESIRED_EVIDENCE_COUNT];
    printEvidence(house.sharedEvidence, found);
    GhostClass ghostDetermined = ghostGuess(house.sharedEvidence.size, found);

    printEndRemainder(ghostDetermined, actualType);
}

/* 
    Function: printEndIntro(Hunter curHunters[NUM_HUNTERS])
    Purpose:  Prints the intro of the ending sequence/results after the program.
    Params:   
        Input: Hunter curHunters[NUM_HUNTERS] - stores all of the hunters in the program.
    Return: void
*/
void printEndIntro(Hunter curHunters[NUM_HUNTERS]){
    printf("=======================================\n");
    printf("All done! Let's tally the results...\n");
    printf("=======================================\n");
    int numOfRunawaysDueToFear = 0;
    int numOfRunawaysDueToBoredom = 0;
    //Prints all hunters that have fear >= FEAR_MAX
    for(int i = 0; i < NUM_HUNTERS; i++){
        if(curHunters[i].fear >= FEAR_MAX){
            printf("    * %s has run way in fear!\n", curHunters[i].hunterName);
            numOfRunawaysDueToFear++;
        }
    }
    //Prints all hunters that have boredom >= BOREDOM_MAX
    printf("=======================================\n");
    for(int i = 0; i < NUM_HUNTERS; i++){
        if(curHunters[i].boredom >= BOREDOM_MAX){
            printf("    * %s has left due to boredom!\n", curHunters[i].hunterName);
            numOfRunawaysDueToBoredom++;
        }
    }
    
    if(numOfRunawaysDueToFear >= NUM_HUNTERS){
        printf("All the hunters have run away in fear!\n");
    }
    else if(numOfRunawaysDueToBoredom >= NUM_HUNTERS){
        printf("All the hunters have left due to boredom!\n");
    }
    
    if((numOfRunawaysDueToFear+numOfRunawaysDueToBoredom) >= NUM_HUNTERS){
        printf("The ghost has won!\nThe hunters failed!\n");
    } 
    else {
        printf("It seems the ghost has been discovered!\nThe hunters have won the game!\n");
    }
}

/* 
    Function: printEvidence(EvidenceList sharedEvidence, EvidenceType found[DESIRED_EVIDENCE_COUNT])
    Purpose:  Prints the evidence of the ending sequence/results after the program.
    Params:   
        Input: EvidenceList sharedEvidence - stores all of the evidence collected by the hunters.
        output: EvidenceType found[DESIRED_EVIDENCE_COUNT] -stores all of the evidence found by the hunters. 
    Return: void
*/
void printEvidence(EvidenceList sharedEvidence, EvidenceType found[DESIRED_EVIDENCE_COUNT]){
    printf("The hunters collected the following evidence: \n");
    EvidenceNode* curNode = sharedEvidence.head;
    for(int i = 0; i < sharedEvidence.size; i++) {
        char str[MAX_STR];
        evidenceToString(curNode->data, str);
        printf("    * %s\n", str);
        found[i] = curNode->data;
        curNode = curNode->next;
    }
}

/* 
    Function: ghostGuess(int sharedEvidenceSize, EvidenceType* found)
    Purpose:  Computes the ghost identified based of the hunter's shared evidence collection.
    Params:   
        Input: int sharedEvidenceSize - stores the number of pieces of evidence collected by the hunters.
        Input: EvidenceType* found -stores all of the evidence found by the hunters.
    Return: Ghostclass - returns the supposed ghost, based of the hunter's shared evidence collection.
*/
GhostClass ghostGuess(int sharedEvidenceSize, EvidenceType* found){
    GhostClass temp = GH_UNKNOWN;
    if(sharedEvidenceSize == DESIRED_EVIDENCE_COUNT){
        if((found[0] != SOUND) && (found[1] != SOUND) && (found[2] != SOUND)){
            temp = POLTERGEIST;
        }
        else if((found[0] != EMF) && (found[1] != EMF) && (found[2] != EMF)){
            temp = PHANTOM;
        }
        if((found[0] != TEMPERATURE) && (found[1] != TEMPERATURE) && (found[2] != TEMPERATURE)){
            temp = BULLIES;
        }
        if((found[0] != FINGERPRINTS) && (found[1] != FINGERPRINTS) && (found[2] != FINGERPRINTS)){
            temp = BANSHEE;
        }
    }
    return temp;
}

/* 
    Function: printEndRemainder(GhostClass ghostDetermined, GhostClass actualType)
    Purpose:  Prints the remainder of the ending sequence/results after the program.
    Params:   
        Input: GhostClass ghostDetermined - stores the ghost determined based of the hunter's shared evidence collection.
        Input: GhostClass actualType - stores the actual type of the ghost.
    Return: void
*/
void printEndRemainder(GhostClass ghostDetermined, GhostClass actualType){
    char ghostTypeGuess[MAX_STR];
    ghostToString(ghostDetermined, ghostTypeGuess);
    char ghostType[MAX_STR];
    ghostToString(actualType, ghostType);
    if(ghostDetermined == actualType){
        printf("Using the evidence they found, they correctly determined that the ghost is a %s\n", ghostTypeGuess);
    }
    else{
        printf("Using the evidence they found, they incorrectly determined that the ghost is a %s\nThe ghost is actually %s\n", ghostTypeGuess, ghostType);
    }
}

/* 
    Function: freeProgram(RoomNode* head, EvidenceList sharedEvidence)
    Purpose:  Frees all dynamically allocated memory associated with the program.
    Params:   
        Input/Output: RoomNode* head - points to the head of the house's room list.
        Input/Output: EvidenceList sharedEvidence - stores the hunter's shared evidence list.
    Return: void
*/
void freeProgram(RoomNode* head, EvidenceList sharedEvidence){
    RoomNode* curRoom = head;
    //Iterates through each room in the house
    while(curRoom != NULL){
        RoomNode* tempNext = curRoom->next;
        deallocateRoom(curRoom);
        curRoom = tempNext;
    }
    deallocateSharedEvidenceList(sharedEvidence);
}

/* 
    Function: deallocateRoom(RoomNode* roomNode)
    Purpose:  Deallocates a room.
    Params:   
        Input/Output: RoomNode* roomNode - points to the room being deallocated.
    Return: void
*/
void deallocateRoom(RoomNode* roomNode){
    EvidenceNode* curEvidenceNode = roomNode->data->evidenceList.head;
    //Iterates through all the evidence in each room
    while(curEvidenceNode != NULL){
        EvidenceNode* tempNext = curEvidenceNode->next;
        free(curEvidenceNode);
        curEvidenceNode = tempNext;
    }    
    RoomNode* curRoomNode = roomNode->data->connectedRooms.head;
    //Iterates though each room in the room's connected rooms
    while(curRoomNode != NULL){
        RoomNode* tempNext = curRoomNode->next;
        free(curRoomNode);
        curRoomNode = tempNext;
    }
    free(roomNode->data);
    free(roomNode);
}

/* 
    Function: deallocateSharedEvidenceList(EvidenceList sharedEvidence)
    Purpose:  Deallocates the hunter's shared evidence list.
    Params:   
        Input/Output: EvidenceList sharedEvidence - points to the shared evidence list being deallocated.
    Return: void
*/
void deallocateSharedEvidenceList(EvidenceList sharedEvidence){
    EvidenceNode* curEvidenceNode = sharedEvidence.head;
    while(curEvidenceNode != NULL){
        EvidenceNode* tempNext = curEvidenceNode->next;
        free(curEvidenceNode);
        curEvidenceNode = tempNext;
    }    
}

/* 
    Function: addEvidence(EvidenceList* evList, EvidenceType evType, sem_t* mutex)
    Purpose:  Adds evidence to a given evidence list.
    Params:   
        Input/Output: EvidenceList* evList - points to evidence list where evidence is being added.
        Input: EvidenceType evType - stores the evidence type being added to the evidence list.
    Return: void
*/
void addEvidence(EvidenceList* evList, EvidenceType evType){
    if(sem_wait(&(evList->evidenceMutex)) == 0){
        EvidenceNode* new = (EvidenceNode*) malloc(sizeof(EvidenceNode));
        new->data = evType;
        new->next = NULL;
        //If list is empty
        if(evList->head == NULL){
            evList->head = new;
        }
        //Else adding to the back
        else{
            evList->tail->next = new;
        }
        evList->tail = new;
        evList->size += 1;
        sem_post(&(evList->evidenceMutex));
    }
}

/* 
    Function: selectConnectedRoom(Room* curRoom, sem_t* mutex)
    Purpose:  Selects a room for the given entity to move into next.
    Params:   
        Input: Room* curRoom - points to room where the entity currently is.
        Input/Output: sem_t* mutex - points to a mutex that locks the current room while the next room is selected.
    Return: Room* - returns a pointer to a randomly selected connected room.
*/
Room* selectConnectedRoom(Room* curRoom, sem_t* mutex){
    Room* entering = NULL;
    if(sem_wait(mutex) == 0){
        //Randomly selects room from connected rooms
        int n = randInt(0, (curRoom->connectedRooms.size));
        RoomNode* curNode;
        curNode = curRoom->connectedRooms.head;
        for(int i = 0; i < n; i++){
            curNode = curNode->next;
        }
        entering = curNode->data;   
        sem_post(mutex);
    }
    return entering;
}