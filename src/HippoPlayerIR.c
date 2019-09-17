//
//  HippoPlayerIR
//
//  Created by Marek Hac on 28/08/2019.
//  Copyright © 2019 MARXSOFT Marek Hac. All rights reserved.
//
//  Compile info:
//  gcc HippoPlayerIR.c

#include <stdio.h>
#include <string.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <intuition/intuition.h>
#include "exec/types.h"
#include "exec/io.h"
#include "devices/serial.h"
#include "includes/dev_support.c"
#include "includes/ser_support.c"

struct IOExtSer *SerReq;
#define SER_LEN (ULONG) sizeof (struct IOExtSer)

#define MAX_STRING 256
#define MAX_COMMAND_LENGTH 32
#define NUM_OF_ACTIONS 15
#define FAIL_TO_LOAD_CONFIG 1

#define VOLUME_STEP 4
#define VOLUME_MAX 64
#define VOLUME_MIN 0

// custom printfs

#define DEBUG_PRINT(...) if (debugMode) { printf(__VA_ARGS__); }

// print every action in the same line

#define ACTION_MSG(msg) printf("\r\33[2KACTION: %s",msg); fflush(stdout);

// enums

enum ActionType {
    VOL_DOWN_ACTION,
    VOL_UP_ACTION,
    PLAY_NEXT_ACTION,
    PLAY_PREV,
    CHOOSE_PREV,
    CHOOSE_NEXT,
    SHOW_SAMPLES,
    PLAY_SELECTED,
    STOP_CONTINUE,
    REW_PATTERN,
    FFWD_PATTERN,
    JUMP_10_MODS_FORWARD,
    JUMP_10_MODS_BACKWARDS,
    QUIT
};

enum JumpDirection {
	FORWARD,
	BACKWARDS
};

struct Action {
    enum ActionType type;
    char irCode[MAX_STRING];
};

// methods declaration

void processCommandLineArgs(UWORD, char**);
void displayHeaderInfo();
void runScriptForType(enum ActionType);
void actionVolumeChange(enum ActionType);
void actionStopContinue();
void actionJumpOnPlaylist(ULONG, enum JumpDirection);
ULONG loadConfiguration(struct Action*);
LONG doCommand(UBYTE *command, BPTR other);

// global variables

BOOL debugMode = FALSE;
BOOL irCodesMonitor = FALSE;
LONG choosedModuleNumber = 1;
LONG volume = 64;
BOOL isMuted = FALSE; // stop or continue

void CloseIt(char *String)
{
    UWORD Error = 0;
    UWORD i;
    UWORD *dff180 = (UWORD *) 0xdff180; /* background color registry */

    if (strlen(String) > 0)
    {
        for (i=0; i<0xffff; i++)
        {
            *dff180 = i; /* blink background */
        }

        puts(String);
        Error = 10;
    }

    if (SerReq != 0L)
    {
        /* when error occurs all opened devices will be released */

        Close_A_Device (SerReq);
    }

    exit(Error);
}

int main(UWORD argc, char* argv[])
{
    ULONG i = 0;
    ULONG argNumber = 0;
    ULONG result = 1;

    BYTE Buffer[MAX_STRING];
    struct Action actions[NUM_OF_ACTIONS];

    displayHeaderInfo();

    processCommandLineArgs(argc, argv);

    if (loadConfiguration(actions) == FAIL_TO_LOAD_CONFIG)
    {
        return 0;
    }

    Open_A_Device ("serial.device", 0L, &SerReq, 0L, SER_LEN);

    while (!CheckSignal(SIGBREAKF_CTRL_D))
    {
        Serial_Read (SerReq, Buffer, -1);

        for (i = 0; i < NUM_OF_ACTIONS; i++)
        {
            if (strcmp(actions[i].irCode, Buffer) == 0)
            {
                runScriptForType(actions[i].type);
                break;
            }
        }

        if (actions[i].type == QUIT)
        {
            break; // finish the while loop
        }

        if (irCodesMonitor)
        {
            printf(" - Button IR Code: %s\n", Buffer);
        }
    }

    Close_A_Device(SerReq);
    return 0;
}

void processCommandLineArgs(UWORD argc, char* argv[])
{
    ULONG argNumber = 0;

    for (argNumber = 0; argNumber < argc; argNumber++)
    {
        if (strcmp(argv[argNumber],"-debug") == 0)
        {
            debugMode = TRUE;
            printf("[ DEBUG MODE     : ON]\n");
        }

        if (strcmp(argv[argNumber],"-monitor") == 0)
        {
            irCodesMonitor = TRUE;
            printf("[ IR CODE MONITOR: ON]\n");
        }

        if (strcmp(argv[argNumber],"-help") == 0)
        {
            printf("\nUsage: HippoPlayerIR <options>\n\n");
            printf("Where <options> is one of:\n\n");
            printf("-debug   : Debug mode\n");
            printf("-monitor : Turn on IRCodes monitor\n");
            printf("-help    : Help & usage\n\n");
        }
    }
}

void displayHeaderInfo()
{
    printf("\nHippoPlayerIR Version 1.0\n");
    printf("Copyright © 2019 by MARXSOFT Marek Hac\n");
    printf("http://github.com/marekhac\n\n");
}

LONG doCommand(UBYTE *command, BPTR other)
{
    struct TagItem stags[4];

    stags[0].ti_Tag = SYS_Input;
    stags[0].ti_Data = NULL;
    stags[1].ti_Tag = SYS_Output;
    stags[1].ti_Data = NULL;
    stags[2].ti_Tag = SYS_Asynch;
    stags[2].ti_Data = TRUE;
    stags[3].ti_Tag = TAG_DONE;
    stags[3].ti_Data = 0;

    return(SystemTagList(command, stags));
}

void createActionNamesArray(UBYTE* actionNames[])
{
    actionNames[VOL_DOWN_ACTION] = "volumeDown";
    actionNames[VOL_UP_ACTION] = "volumeUp";
    actionNames[QUIT] = "quitProgram";
    actionNames[PLAY_NEXT_ACTION] = "playNext";
    actionNames[PLAY_PREV] = "playPrev";
    actionNames[CHOOSE_PREV] = "choosePrev";
    actionNames[CHOOSE_NEXT] = "chooseNext";
    actionNames[SHOW_SAMPLES] = "showSamples";
    actionNames[PLAY_SELECTED] = "playSelected";
    actionNames[STOP_CONTINUE] = "stopContinue";
    actionNames[REW_PATTERN] = "rewPattern";
    actionNames[FFWD_PATTERN] = "ffwdPattern";
    actionNames[JUMP_10_MODS_FORWARD] = "10ModsForward";
    actionNames[JUMP_10_MODS_BACKWARDS] = "10ModsBackwards";
}

enum ActionType getActionType(UBYTE *name, UBYTE* actionNames[])
{
    ULONG i;

    for (i = 0; i < NUM_OF_ACTIONS; i++)
    {
        if (strcmp(actionNames[i], name) == 0)
        {
            return i;
        }
    }

    return 0;
};

ULONG loadConfiguration(struct Action *actions)
{
    FILE *fp;
    UBYTE string[MAX_STRING];
    UBYTE *filename = "HippoPlayerIR.config";
    UBYTE *item;
    UBYTE *actionNames[NUM_OF_ACTIONS];
    ULONG i = 0;

    struct Action action;

    fp = fopen(filename, "r");
    if (fp == NULL){
        printf("Could not open file: %s\n",filename);
        return 1;
    }

    createActionNamesArray(actionNames);

    DEBUG_PRINT("-------------------------\n");
    DEBUG_PRINT("  Actons configuration:\n");
    DEBUG_PRINT("-------------------------\n");

    while (fgets(string, MAX_STRING, fp) != NULL)
    {
        // parse config file

        string[strcspn(string, "\r\n")] = 0; // remove useless chars
        item = strtok(string, ":"); // split line with delimiter ":"

        action.type = getActionType(item, actionNames);

        DEBUG_PRINT("%-3s\t", item); // action type name

        item = strtok(NULL, ": ");

        strcpy(action.irCode,item);

        DEBUG_PRINT("%s\n", item);

        actions[i] = action;

        i++;
    }

    DEBUG_PRINT("-------------------------\n");

    fclose(fp);
    return 0;
}

void runScriptForType(enum ActionType type)
{
    UBYTE *commandPlayNext = "rx arexx/PlayNext.hip";
    UBYTE *commandPlayPrev = "rx arexx/PlayPrev.hip";
    UBYTE *commandShowSamples = "rx arexx/showsamples.hip";
    UBYTE *commandPlaySelected = "rx arexx/play2.hip";
    UBYTE *commandRewPattern = "rx arexx/rew.hip";
    UBYTE *commandFfwdPattern = "rx arexx/ffwd.hip";
    ULONG result;

    switch(type)
    {
        case VOL_DOWN_ACTION:
        {
            ACTION_MSG("Volume down");
            actionVolumeChange(type);
            break;
        }
        case VOL_UP_ACTION:
        {
            ACTION_MSG("Volume up");
            actionVolumeChange(type);
            break;
        }
        case CHOOSE_PREV:
        {
            ACTION_MSG("Choose previous module");
            actionJumpOnPlaylist(1, BACKWARDS);
            break;
        }
        case CHOOSE_NEXT:
        {
            ACTION_MSG("Choose next module");
            actionJumpOnPlaylist(1, FORWARD); ;
            break;
        }
        case PLAY_NEXT_ACTION:
        {
            ACTION_MSG("Play next module");
            choosedModuleNumber += 1;
            result = doCommand(commandPlayNext,NULL);
            break;
        }
        case PLAY_PREV:
        {
            ACTION_MSG("Play previous module");
            choosedModuleNumber -= 1;
            result = doCommand(commandPlayPrev,NULL);
            break;
        }
        case SHOW_SAMPLES:
        {
            ACTION_MSG("Show/hide samples window");
            result = doCommand(commandShowSamples,NULL);
            break;
        }
        case PLAY_SELECTED:
        {
            ACTION_MSG("Play selected module");
            result = doCommand(commandPlaySelected,NULL);
            break;
        }
        case STOP_CONTINUE:
        {
            actionStopContinue();
            break;
        }
        case REW_PATTERN:
        {
            ACTION_MSG("REW pattern");
            result = doCommand(commandRewPattern,NULL);
            break;
        }
        case FFWD_PATTERN:
        {
            ACTION_MSG("FFWD pattern");
            result = doCommand(commandFfwdPattern,NULL);
            break;
        }
        case QUIT:
        {
            ACTION_MSG("Quit Program\nThanks for using HippoPlayerIR :)\n\n");
            break;
        }

        case JUMP_10_MODS_FORWARD:
        {
            ACTION_MSG("Jump 10 modules forward");
            actionJumpOnPlaylist(10, FORWARD);
            break;
        }
        case JUMP_10_MODS_BACKWARDS:
        {
            ACTION_MSG("Jump 10 modules backwards");
            actionJumpOnPlaylist(10, BACKWARDS);

            break;
        }
    }
}

void actionVolumeChange(enum ActionType type)
{
    UBYTE charVolume[MAX_COMMAND_LENGTH];
    LONG result;

    // volume down

    if (type == VOL_DOWN_ACTION)
    {
        volume -= VOLUME_STEP;

        if (volume < VOLUME_MIN)
        {
            volume = VOLUME_MIN;
        }
    }

    // volume up

    if (type == VOL_UP_ACTION)
    {
        volume += VOLUME_STEP;

        if (volume > VOLUME_MAX)
        {
            volume = VOLUME_MAX;
        }
    }

    sprintf(charVolume, "rx arexx/volume.hip %d", volume);

    result = doCommand(charVolume, NULL);
}

void actionStopContinue()
{
    UBYTE *commandStopPlaying = "rx arexx/stop.hip";
    UBYTE *commandContinuePlaying = "rx arexx/cont.hip";
    LONG result;

    if (isMuted == FALSE)
    {
        ACTION_MSG("Stop playing module");
        result = doCommand(commandStopPlaying,NULL);
        isMuted = TRUE;
    }
    else
    {
        ACTION_MSG("Continue playing module");
        result = doCommand(commandContinuePlaying,NULL);
        isMuted = FALSE;
    }
}

void actionJumpOnPlaylist(ULONG number, enum JumpDirection direction)
{
    UBYTE charChoosedModule[MAX_COMMAND_LENGTH];
    LONG result;

    // jump forward

    if (direction == FORWARD)
    {
        choosedModuleNumber += number;
    }

    // jump backwards

    if (direction == BACKWARDS)
    {
        choosedModuleNumber -= number;

        if (choosedModuleNumber < 0)
        {
            choosedModuleNumber = 1;
        }
    }

    sprintf(charChoosedModule, "rx arexx/choose.hip %d", choosedModuleNumber);

    result = doCommand(charChoosedModule, NULL);
}