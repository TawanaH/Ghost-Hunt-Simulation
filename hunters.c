#include "defs.h"

//Function declarations
void moveHunter(Hunter* hunter);
int removeEvidence(Hunter* hunter);
void removeHunter(Hunter* hunter);
void addHunter(Hunter* hunter, Room* entering);
int isGhostInRoom(Room* curRoom);
int isHunterleaving(Hunter* curHunter);
void collectEvidence(Hunter* hunter);
int reviewEvidence(Hunter* hunter);

/* 
    Function: initHunter(char* name, Room* startingRoom, EvidenceType equipment, EvidenceList* sharedEvidence)
    Purpose:  Initializes a Hunter struct.
    Params:   
        Input: char* name - stores the name of the hunter being initialized.
        Input: Room* startingRoom - points to the room the hunter will start in (Van).
        Input: EvidenceType equipment - stores the type of evidence the hunter will be able to read.
        Input: EvidenceList* sharedEvidence - points the EvidenceList shared by all hunters.
    Return: Hunter - returns a fully initialized hunter.
*/
Hunter initHunter(char* name, Room* startingRoom, EvidenceType equipment, EvidenceList* sharedEvidence){
    Hunter new;
    new.curRoom = startingRoom;
    new.reader = equipment;
    strcpy(new.hunterName, name);
    new.sharedEvidencePointer = sharedEvidence;
    new.fear = 0;
    new.boredom = 0;
    l_hunterInit(name, equipment);
    return new;
}

/* 
    Function: runHunter(void* voidHunter)
    Purpose:  Runs the hunter thread. 
    Params:   
        Input/Output: void* voidHunter - points to the Hunter that is going to run in the thread.
    Return: void*
*/
void* runHunter(void* voidHunter){
    Hunter* curHunter = (Hunter*) voidHunter;
    //Loops the hunter so it keeps taking actions
    while(C_TRUE){
        usleep(HUNTER_WAIT);
        //Checks if the hunter is leaving due to either fear or boredom, and ends the loop if so
        if(isHunterleaving(curHunter) == C_TRUE){
            break;
        }
        //randomly chooses an action and then performs it
        int hunterChoice = randInt(0, 3);
        if(hunterChoice == 0){
            collectEvidence(curHunter);
        }
        else if(hunterChoice == 1){
            moveHunter(curHunter);
        }
        else{
            if(reviewEvidence(curHunter) == C_TRUE){
                l_hunterExit(curHunter->hunterName, LOG_EVIDENCE);
                break;
            }
        }
    }
    //removes the hunter from whatever room it's in when the hunter leaves
    removeHunter(curHunter);
    return NULL;
}

/* 
    Function: moveHunter(Hunter* hunter)
    Purpose:  Moves a hunter from one room to another randomly selected room.
    Params:   
        Input/Output: Hunter* hunter - points to the Hunter moving room.
    Return: void
*/
void moveHunter(Hunter* hunter){
    //Selects the room to move to
    Room* entering = selectConnectedRoom(hunter->curRoom, &(hunter->curRoom->roomHunterMutex));
    removeHunter(hunter);
    addHunter(hunter, entering);
}

/* 
    Function: removeHunter(Hunter* hunter)
    Purpose:  Removes a hunter from its current room.
    Params:   
        Input/Output: Hunter* hunter - points to the Hunter being removed from it's current room.
    Return: void
*/
void removeHunter(Hunter* hunter){
    if(sem_wait(&(hunter->curRoom->roomHunterMutex)) == 0){
        //Remove hunter from leaving room
        for(int i = 0; i < NUM_HUNTERS; i++){
            if(hunter->curRoom->curHunters[i] == hunter){
                hunter->curRoom->curHunters[i] = NULL;
                break;
            }
        }
        sem_post(&(hunter->curRoom->roomHunterMutex));
    }
}

/* 
    Function: addHunter(Hunter* hunter, Room* entering)
    Purpose:  Adds a given hunter to the specified room.
    Params:   
        Input/Output: Hunter* hunter - points to the Hunter being added to the specified room.
        Input/Output: Room* entering - points to the room the hunter is entering.
    Return: void
*/
void addHunter(Hunter* hunter, Room* entering){
    if(sem_wait(&(entering->roomHunterMutex)) == 0){
        //Add hunter to entering room
        for(int i = 0; i < NUM_HUNTERS; i++){
            if(entering->curHunters[i] == NULL){
                entering->curHunters[i] = hunter;
                l_hunterMove(hunter->hunterName, entering->roomName);
                break;
            }
        }
        hunter->curRoom = entering;
        sem_post(&(entering->roomHunterMutex));
    }    
}

/* 
    Function: collectEvidence(Hunter* hunter)
    Purpose:  Allows a hunter to attempt to collect evidence.
    Params:   
        Input/Output: Hunter* hunter - points to the Hunter attempting to collect evidence.
    Return: void
*/
void collectEvidence(Hunter* hunter){
    //Removes evidence correlated with the hunter's equipment from the room, and then returns if no evidence found
    if(removeEvidence(hunter) == C_FALSE){
        return;
    }
    l_hunterCollect(hunter->hunterName, hunter->reader, hunter->curRoom->roomName);
    if(sem_wait(&(hunter->sharedEvidencePointer->evidenceMutex)) == 0){
        //Checks if the evidence is already in the shared evidence list, and returns if so
        if(hunter->sharedEvidencePointer->head != NULL){
            EvidenceNode* curNode = hunter->sharedEvidencePointer->head;
            while(curNode != NULL){
                if(curNode->data == hunter->reader){
                    sem_post(&(hunter->sharedEvidencePointer->evidenceMutex));
                    return;
                }
                curNode = curNode->next;
            }
        }
        sem_post(&(hunter->sharedEvidencePointer->evidenceMutex)); 
    }
    //add hunter->reader to the tail of the shared evidence list
    addEvidence(hunter->sharedEvidencePointer, hunter->reader);
}

/* 
    Function: removeEvidence(Hunter* hunter, int* found)
    Purpose:  Removes a piece of evidence from a room if the hunter is able to read the evidence.
    Params:   
        Input/Output: Hunter* hunter - points to the Hunter removing evidence from a room.
    Return: int - returns a value of C_TRUE if a hunter was able to successfully collect evidence from the room, or C_FALSE otherwise
*/
int removeEvidence(Hunter* hunter){
    int found = C_FALSE;
    if(sem_wait(&(hunter->curRoom->evidenceList.evidenceMutex)) == 0){
        EvidenceNode* prev = NULL;
        EvidenceNode* curNode = hunter->curRoom->evidenceList.head;
        //Loops through the evidence list and removes all evidence matching the hunter's equipment    
        while(curNode != NULL){
            if(curNode->data == hunter->reader){
                //Removing from head
                if(curNode == hunter->curRoom->evidenceList.head){
                    prev = hunter->curRoom->evidenceList.head;
                    hunter->curRoom->evidenceList.head = prev->next;
                    free(prev);
                }
                //Removing from tail
                else if(curNode == hunter->curRoom->evidenceList.tail){
                    hunter->curRoom->evidenceList.tail = prev;
                    free(curNode);
                    prev->next = NULL;
                }
                //Removing from the middle
                else{
                    prev->next = curNode->next;
                    free(curNode);
                }
                found = C_TRUE;
                break;
            }
            prev = curNode;
            curNode = curNode->next;
        }
        sem_post(&(hunter->curRoom->evidenceList.evidenceMutex));
    }
    return found;
}

/* 
    Function: reviewEvidence(Hunter* hunter)
    Purpose:  Allows a hunter to review all of the evidence in the hunter's shared evidence list.
    Params:   
        Input: Hunter* hunter - points to the Hunter reviewing the sharedEvidenceList.
    Return: int - returns a value of C_TRUE if the hunter has enough evidence to know what ghost is present, or C_FALSE otherwise
*/
int reviewEvidence(Hunter* hunter){
    if(sem_wait(&(hunter->sharedEvidencePointer->evidenceMutex)) == 0){
        //Checks if the hunters have found the amount of evidence they need and returns C_TRUE or C_FALSE accordingly
        if(hunter->sharedEvidencePointer->size >= DESIRED_EVIDENCE_COUNT){
            sem_post(&(hunter->sharedEvidencePointer->evidenceMutex));
            l_hunterReview(hunter->hunterName, LOG_SUFFICIENT);
            return C_TRUE;
        }
        sem_post(&(hunter->sharedEvidencePointer->evidenceMutex));
        l_hunterReview(hunter->hunterName, LOG_INSUFFICIENT);
        return C_FALSE;
    }
}

/*
    Function: isHunterleaving(Hunter* curHunter)
    Purpose: checks if the hunter is leaving due to boredom or fear
    Params:
        Input/Output: Hunter* curHunter - points to the hunter being checked
    Return: int - returns C_TRUE if the hunter is leaving, or C_FALSE otherwise
*/
int isHunterleaving(Hunter* curHunter){
    if(isGhostInRoom(curHunter->curRoom) == C_TRUE){
        curHunter->fear += FEAR_INCREMENT;
        curHunter->boredom = 0;
        if(curHunter->fear == FEAR_MAX){
            l_hunterExit(curHunter->hunterName, LOG_FEAR);
            return C_TRUE;
        }
    } else {
        curHunter->boredom++;
        if(curHunter->boredom == BOREDOM_MAX){
            l_hunterExit(curHunter->hunterName, LOG_BORED);
            return C_TRUE;
        }
    }
    return C_FALSE;
}

/* 
    Function: isGhostInRoom(Room* curRoom)
    Purpose:  Determine if there is a ghost inside the room with the hunter.
    Params:   
        Input: Room* curRoom - points to the room we are checking contains the ghost.
    Return: int - returns C_FALSE if there is no ghost in the room or C_TRUE if there is a ghost in the room.
*/
int isGhostInRoom(Room* curRoom){
    int ghostInRoom = C_FALSE;
    if(sem_wait(&(curRoom->roomGhostMutex)) == 0){
        if(curRoom->ghost != NULL){
            ghostInRoom = C_TRUE;
        }
        sem_post(&(curRoom->roomGhostMutex));
    }
    return ghostInRoom;
}