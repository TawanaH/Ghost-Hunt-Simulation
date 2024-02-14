#include "defs.h"

//Forward declarations
void leaveEvidence(Ghost* ghost);
void moveRoom(Ghost* curGhost);
void addGhost(Ghost* ghost, Room* entering);
void removeGhost(Ghost* ghost);
int isHunterInRoom(Room* curRoom);
int isGhostLeaving(Ghost* curGhost, int* ghostChoice);

/* 
    Function: initGhost(RoomList* rooms, Ghost* curGhost)
    Purpose:  Initializes a Ghost struct.
    Params:   
        Input: RoomList* rooms - stores all the rooms in the house so the ghost can randomly choose one to start in.
        Input/Output: Ghost* curGhost - points to the ghost being initialized.
    Return: void
*/
void initGhost(RoomList* rooms, Ghost* curGhost){
    curGhost->boredomTimer = 0;
    curGhost->ghostType = randomGhost();
    int n = randInt(0, (rooms->size)-1);
    RoomNode* curNode = rooms->head->next;
    for(int i = 0; i < n; i++){
        curNode = curNode->next;
    }
    curGhost->curRoom = curNode->data;
    curGhost->curRoom->ghost = curGhost;
    l_ghostInit(curGhost->ghostType, curGhost->curRoom->roomName);
}

/* 
    Function: runGhost(void* voidGhost)
    Purpose:  Runs the ghost thread. 
    Params:   
        Input/Output: void* voidGhost - points to the Ghost that is going to run in the thread.
    Return: void*
*/
void* runGhost(void* voidGhost){
    Ghost* curGhost = (Ghost*) voidGhost;
    //Loops the ghost so it keeps taking actions
    while(C_TRUE){
        usleep(GHOST_WAIT);
        int ghostChoice;
        //Checks if the ghost is leaving and returns if so, and performs choice generation
        if(isGhostLeaving(curGhost, &ghostChoice) == C_TRUE){
            break;
        }
        //Leave evidence
        if(ghostChoice == 0){
            leaveEvidence(curGhost);
        }
        //Do nothing
        else if(ghostChoice == 1){
        }
        //Move
        else{
            //Needs update
            moveRoom(curGhost);
        }
    }
    return NULL;
}

/* 
    Function: isGhostLeaving(Ghost* curGhost, int* ghostChoice)
    Purpose:  Determine if the ghost is leaving the house
    Params:   
        Input: Ghost* curGhost - points to the ghost we are checking if it is leaving the house.
        Output: int* ghostChoice - points to the integer which stores a random int which determines what the ghost's next move will be.
    Return: int - returns C_FALSE if the ghost is not leaving the house. Returns C_TRUE if the ghost is leaving the house.
*/
int isGhostLeaving(Ghost* curGhost, int* ghostChoice){
    //Checks if a hunter is in the room, and if so, resets the boredom timer and limits the choices so it won't leave the room
    if(isHunterInRoom(curGhost->curRoom) == C_TRUE){
        curGhost->boredomTimer = 0;
        *ghostChoice = randInt(0, 2);
    }
    //Increments the boredom timer and randomly chooses an action
    else{
        curGhost->boredomTimer++;
        *ghostChoice = randInt(0, 3);
        if(curGhost->boredomTimer == BOREDOM_MAX){
            l_ghostExit(LOG_BORED);
            return C_TRUE;
        }
    }
    return C_FALSE;
}

/* 
    Function: isHunterInRoom(Room* curRoom)
    Purpose:  Determine if there is a hunter inside the room with a ghost.
    Params:   
        Input: Room* curRoom - points to the room we are checking contains hunter(s).
    Return: int - returns C_FALSE if there is no hunter in the room or C_TRUE if there is a hunter in the room.
*/
int isHunterInRoom(Room* curRoom){
    int hunterInRoom = C_FALSE;
    //Iterate through list of hunters in room
    if(sem_wait(&(curRoom->roomHunterMutex)) == 0){
        for(int i = 0; i < NUM_HUNTERS; i++){
            if(curRoom->curHunters[i] != NULL){
                hunterInRoom = C_TRUE;
            }
        }
        sem_post(&(curRoom->roomHunterMutex));
    }
    return hunterInRoom;
}

/* 
    Function: moveRoom(Ghost* curGhost)
    Purpose:  Moves the ghost from one room to another randomly selected room.
    Params:   
        Input/Output: Ghost* curGhost - points to the Ghost moving room.
    Return: void
*/
void moveRoom(Ghost* curGhost){
    //Selects the room to move to
    Room* entering = selectConnectedRoom(curGhost->curRoom, &(curGhost->curRoom->roomGhostMutex));
    removeGhost(curGhost); 
    addGhost(curGhost, entering);
}

/* 
    Function: removeGhost(Ghost* ghost)
    Purpose:  Removes a ghost from its current room.
    Params:   
        Input/Output: Ghost* ghost - points to the Ghost being removed from it's current room.
    Return: void
*/
void removeGhost(Ghost* ghost){
    if(sem_wait(&(ghost->curRoom->roomGhostMutex)) == 0){
        ghost->curRoom->ghost = NULL;
        sem_post(&(ghost->curRoom->roomGhostMutex));
    }
}

/* 
    Function: addGhost(Ghost* ghost, Room* entering)
    Purpose:  Adds a given ghost to the specified room.
    Params:   
        Input/Output: Ghost* ghost - points to the Ghost being added to the specified room.
        Input/Output: Room* entering - points to the room the Ghost is entering.
    Return: void
*/
void addGhost(Ghost* ghost, Room* entering){
    if(sem_wait(&(entering->roomGhostMutex)) == 0){
        ghost->curRoom = entering;
        ghost->curRoom->ghost = ghost;
        l_ghostMove(ghost->curRoom->roomName);
        sem_post(&(entering->roomGhostMutex));
    }
}

/* 
    Function: leaveEvidence(Ghost* ghost)
    Purpose:  Allow the ghost to leave evidence inside a room.
    Params:   
        Input: Ghost* ghost - points to the Ghost leaving evidence.
    Return: void
*/
void leaveEvidence(Ghost* ghost){
    int n;
    while(C_TRUE){
        //Randomly selects a piece of evidence to leave, and ensures the ghost can leave that type of evidence. If it can't, tries again.
        n = randInt(0, EV_COUNT);
        if(ghost->ghostType == POLTERGEIST && n != SOUND){
            addEvidence(&(ghost->curRoom->evidenceList), n);
            break;
        }
        else if(ghost->ghostType == BANSHEE && n != FINGERPRINTS){
            addEvidence(&(ghost->curRoom->evidenceList), n);
            break;
        }
        else if(ghost->ghostType == BULLIES && n != TEMPERATURE){
            addEvidence(&(ghost->curRoom->evidenceList), n);
            break;
        }
        else if(ghost->ghostType == PHANTOM && n != EMF){
            addEvidence(&(ghost->curRoom->evidenceList), n);
            break;
        }
    }
    l_ghostEvidence(n, ghost->curRoom->roomName);
}
