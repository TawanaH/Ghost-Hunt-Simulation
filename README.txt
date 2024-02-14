Purpose: Creates and runs threads representing a ghost and 4 hunters, which then perform various actions until the hunters all flee, or they determine the ghost's type. The program then prints relevant text based on how the threads ran, and frees all dynamically allocated data.

List of files:
    main.c: Contains the code for the main control flow.
    helpers.c: Contains the code for initializing ghosts, ghost behaviour, and various other helper functions.
    house.c: Contains the code/calls for initializing the house, creating rooms, connecting two rooms together, and adding rooms to a house.
    room.c: Contains code/implementations for creating rooms, connecting two rooms together, and adding rooms to a house.
    hunters.c: Contains code for initializing hunters, hunter behaviour, and various other helper functions.
    ghost.c: Contains code for initializing ghosts, ghost behaviour, and various other helper functions.
    logger.c: Contains code for logging ghost and hunter behaviour/operations.
    utils.c: Contains code for generating random integers & floats, converting EvidenceType to string, and converting Ghostclass to string.
    defs.h: Contains forward declarations, struct definitions, and constant definitions.
    Makefile: Calling make will compile all files that have been changed and then link all object files into an executable file. Calling make clean will delete object files.
    README.txt: Contains all relevant information about the program.

Compliling and Executing:
1. Navigate to the folder containing the source code in a terminal.
2. Use the command 'make' to compile and link all necessary files.
3. Use the command './finalProject' while in the folder containing the executable to run the program.

Optional:
1. Navigate to the folder containing the source code in a terminal and use 'make clean' after running 'make all' to delete all the files created by running 'make all'.
Behaviour should be varied as is, but if you want to force specific outputs:
2. In defs.h, reduce BOREDOM_MAX to see the hunters and ghost exit due to boredom with increased probability.
3. In defs.h, increase FEAR_INCREMENT to see the hunters exit due to fear with increased probability.

Usage instructions:
    Follow the on-screen prompts once the program has started running by entering names for the specified hunters.

Generative AI: No AI used