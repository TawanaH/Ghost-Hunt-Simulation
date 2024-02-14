#include "defs.h"

int main()
{   
    HouseType house;
    Ghost ghost;
    //Initialize all elements of the program
    initProgram(&(house), &(ghost));
    //Threading
    runThreads(&house, &ghost);
    //Print ending
    printEnd(house, ghost.ghostType);
    //Free dynamic memory
    freeProgram(house.rooms.head, house.sharedEvidence);
    return 0;
}