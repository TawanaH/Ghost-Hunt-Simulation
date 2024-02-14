#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MAX_STR                64
#define MAX_RUNS               50
#define BOREDOM_MAX            100
#define C_TRUE                 1
#define C_FALSE                0
#define HUNTER_WAIT            5000
#define GHOST_WAIT             600
#define NUM_HUNTERS            4
#define FEAR_MAX               10
#define LOGGING                C_TRUE
#define FEAR_INCREMENT         1
#define DESIRED_EVIDENCE_COUNT 3

typedef enum EvidenceType EvidenceType;
typedef enum GhostClass GhostClass;

enum EvidenceType { EMF, TEMPERATURE, FINGERPRINTS, SOUND, EV_COUNT, EV_UNKNOWN };
enum GhostClass { POLTERGEIST, BANSHEE, BULLIES, PHANTOM, GHOST_COUNT, GH_UNKNOWN };
enum LoggerDetails { LOG_FEAR, LOG_BORED, LOG_EVIDENCE, LOG_SUFFICIENT, LOG_INSUFFICIENT, LOG_UNKNOWN };

//Define stuctures
//Ghost struct
typedef struct Ghost{
  GhostClass ghostType;
  struct Room* curRoom;
  int boredomTimer; 
} Ghost;

//Room linked list
typedef struct RoomList {
    struct RoomNode* head;
    struct RoomNode* tail;
    int size;
} RoomList;

//Room linked list node
typedef struct RoomNode {
  struct Room* data;
  struct RoomNode* next;
} RoomNode;

//Evidence linked list
typedef struct EvidenceList {
    struct EvidenceNode* head;
    struct EvidenceNode* tail;
    sem_t evidenceMutex;
    int size;
} EvidenceList;

//Evidence linked list node
typedef struct EvidenceNode {
    EvidenceType data;
    struct EvidenceNode* next;
} EvidenceNode;

//Hunter struct
typedef struct Hunter{
    struct Room *curRoom;
    EvidenceType reader; //The type of evidence the hunter collects
    char hunterName[MAX_STR];
    struct EvidenceList* sharedEvidencePointer;
    int fear;
    int boredom;
} Hunter;

//Room struct
typedef struct Room{
    char roomName[MAX_STR];
    struct RoomList connectedRooms;
    struct EvidenceList evidenceList;
    struct Hunter* curHunters[NUM_HUNTERS];
    Ghost* ghost;
    sem_t roomHunterMutex;
    sem_t roomGhostMutex;
} Room;

//House struct
typedef struct{
    struct Hunter curHunters[NUM_HUNTERS];
    struct RoomList rooms;
    struct EvidenceList sharedEvidence;
} HouseType;


// Helper Utilies
int randInt(int,int);        // Pseudo-random number generator function
float randFloat(float, float);  // Pseudo-random float generator function
enum GhostClass randomGhost();  // Return a randomly selected a ghost type
void ghostToString(enum GhostClass, char*); // Convert a ghost type to a string, stored in output paremeter
void evidenceToString(enum EvidenceType, char*); // Convert an evidence type to a string, stored in output parameter

// Logging Utilities
void l_hunterInit(char* name, enum EvidenceType equipment);
void l_hunterMove(char* name, char* room);
void l_hunterReview(char* name, enum LoggerDetails reviewResult);
void l_hunterCollect(char* name, enum EvidenceType evidence, char* room);
void l_hunterExit(char* name, enum LoggerDetails reason);
void l_ghostInit(enum GhostClass type, char* room);
void l_ghostMove(char* room);
void l_ghostEvidence(enum EvidenceType evidence, char* room);
void l_ghostExit(enum LoggerDetails reason);

//Forward declarations needed across functions
void populateRooms(HouseType* house);
void initHouse(HouseType* house);
Room* createRoom(char* roomName);
void connectRooms(Room* room1, Room* room2);
void addRoom(RoomList* roomList, Room* room);
Hunter initHunter(char* name, Room* startingRoom, EvidenceType equipment, EvidenceList* sharedEvidence);
void initGhost(RoomList* rooms, Ghost* curGhost);
void* runHunter(void* voidHunter);
void* runGhost(void* curGhost);
void deallocateRoom(RoomNode* room);
void deallocateSharedEvidenceList(EvidenceList sharedEvidence);
GhostClass ghostGuess(int sharedEvidenceSize, EvidenceType* found);
void addEvidence(EvidenceList* evList, EvidenceType evType);
Room* selectConnectedRoom(Room* curRoom, sem_t* mutex);
void getNames(char hunterNames[][MAX_STR]);
void printEndIntro(Hunter curHunters[NUM_HUNTERS]);
void printEvidence(EvidenceList sharedEvidence, EvidenceType found[3]);
void printEndRemainder(GhostClass ghostDetermined, GhostClass actualType);
void printEnd(HouseType house, GhostClass actualType);
void freeProgram(RoomNode* head, EvidenceList sharedEvidence);
void initProgram(HouseType* house, Ghost* ghost);
void runThreads(HouseType* house, Ghost* ghost);