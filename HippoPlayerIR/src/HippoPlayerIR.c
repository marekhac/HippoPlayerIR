//
//  HippoPlayerIR
//
//  Created by Marek Hac on 23/08/2021.
//  Copyright © 2021 MARXSOFT Marek Hac. All rights reserved.
//  https://github.com/marekhac
//
//  Compile info:
//  gcc HippoPlayerIR.c

#include <stdio.h>
#include <string.h>
#include <exec/libraries.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <devices/serial.h>
#include <intuition/intuition.h>

struct IOExtSer *SerialIO;   /* pointer to I/O request */
struct MsgPort *SerialMP;   /* pointer to Message Port*/

#define BUFFER_SIZE 32
#define MAX_COMMAND_LENGTH 32
#define NUM_OF_ACTIONS 16
#define FAIL_TO_LOAD_CONFIG 1

// serial port params

#define INPUT_BUFFER_SIZE 512
#define READ_LENGTH 8
#define WRITE_LENGTH 8
#define STOP_BITS 1
#define BAUD_RATE 9600 //  number of bits transferred per second

// custom printfs

#define DEBUG_PRINT(...) if (debugMode) { printf(__VA_ARGS__); }

// print every action in the same line

#define ACTION_MSG(msg) printf("\r\33[2KACTION: %s",msg); fflush(stdout);

// version

static UBYTE *version = "$VER: HippoPlayerIR 1.4";

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
    COPY_TO_LIKEDMODS,
    QUIT
};

struct Action {
    enum ActionType type;
    char irCode[BUFFER_SIZE];
};

// methods declaration

void processCommandLineArgs(UWORD, char**);
void displayHeaderInfo();
void printQuitMessage();
void executeCommand(UBYTE *scriptFileName, UBYTE *message);
void runScriptForType(enum ActionType);
void setupCustomSerialParams();
void setupReadCommand();
ULONG loadConfiguration(struct Action*);
LONG doCommand(UBYTE *command, BPTR other);
UBYTE* concat(const UBYTE *s1, const UBYTE *s2);

// global variables

BOOL debugMode = FALSE;
BOOL irCodesMonitor = FALSE;
BYTE serialReadBuffer[BUFFER_SIZE]; // reserve 32 bytes storage
UBYTE *commandPrefix = "rx arexx/";

int main(UWORD argc, char* argv[])
{
    ULONG i = 0;
    ULONG signal;

    struct Action actions[NUM_OF_ACTIONS];

    displayHeaderInfo();

    processCommandLineArgs(argc, argv);

    if (loadConfiguration(actions) == FAIL_TO_LOAD_CONFIG)
    {
        return 0;
    }

    /* create message port for I/O request */

    if (SerialMP = (struct MsgPort *)CreatePort(0,0))
    {
        /* create I/O request */

        if (SerialIO = (struct IOExtSer *)CreateExtIO(SerialMP,sizeof(struct IOExtSer)))
        {
            if (OpenDevice(SERIALNAME,0L,SerialIO,0) )
            {
                printf("%s did not open\n",SERIALNAME);
            }
            else
            {
                setupCustomSerialParams();
                setupReadCommand();

                SendIO(SerialIO);

                do
                {
                    signal = Wait(1L << SerialMP -> mp_SigBit | SIGBREAKF_CTRL_D);

                    if (CheckIO(SerialIO) ) /* If request is complete... */
                    {
                        /* wait for specific I/O request */

                        WaitIO(SerialIO);

                        /* handle received ir code */

                        for (i = 0; i < NUM_OF_ACTIONS; i++)
                        {
                            if (strcmp(actions[i].irCode, serialReadBuffer) == 0)
                            {
                                runScriptForType(actions[i].type);
                                break;
                            }
                        }

                        if (actions[i].type == QUIT)
                        {
                            break;
                        }

                        if (irCodesMonitor)
                        {
                            printf(" - Button IR Code: %s\n", serialReadBuffer);
                        }

                        SendIO(SerialIO); /* restart I/O request */
                    }
                }
                while (!(signal & SIGBREAKF_CTRL_D));

                AbortIO(SerialIO);  /* ask device to abort request, if pending */
                WaitIO(SerialIO);   /* wait for abort, then clean up */
                CloseDevice(SerialIO); /* close serial device */

                printQuitMessage();
            }

            DeleteExtIO(SerialIO); /* delete I/O request */
        }
        else
        {
            printf("Unable to create IORequest\n");
            DeletePort(SerialMP); /* delete message port */
        }
    }
    else
    {
        printf("Unable to create message port\n");
    }

    return 0;
}


void displayHeaderInfo()
{
    printf("\nHippoPlayerIR Version 1.4\n");
    printf("Copyright © 2021 by MARXSOFT Marek Hac\n");
    printf("Press CTRL-D to exit\n\n");
}

void printQuitMessage()
{
	 ACTION_MSG("Quit Program\nThanks for using HippoPlayerIR :)\n\n");
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

/* --- Serial device configuration --- */

void setupReadCommand()
{
    SerialIO->IOSer.io_Command = CMD_READ;
    SerialIO->IOSer.io_Length = -1;
    SerialIO->IOSer.io_Data = &serialReadBuffer;
}

void setupCustomSerialParams()
{
    // update I/O request

    SerialIO->io_RBufLen = INPUT_BUFFER_SIZE;
    SerialIO->io_Baud = BAUD_RATE;
    SerialIO->io_ReadLen = READ_LENGTH;
    SerialIO->io_WriteLen = WRITE_LENGTH;
    SerialIO->io_StopBits = STOP_BITS;
    SerialIO->io_SerFlags &= ~SERF_PARTY_ON; // set parity off
    SerialIO->io_SerFlags |= SERF_XDISABLED; // set xON/xOFF disabled

    // update serial parameters using SDCMD_SETPARAMS command

    SerialIO->IOSer.io_Command = SDCMD_SETPARAMS;

    if (DoIO(SerialIO))
    {
        printf("Error setting serial parameters!\n");
    }
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
    actionNames[COPY_TO_LIKEDMODS] = "copyToLikedMods";
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
    UBYTE string[BUFFER_SIZE];
    UBYTE *filename = "PROGDIR:HippoPlayerIR.config";
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
    DEBUG_PRINT("  Actions configuration:\n");
    DEBUG_PRINT("-------------------------\n");

    while (fgets(string, BUFFER_SIZE, fp) != NULL)
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

void executeArexxScript(UBYTE *scriptFileName, UBYTE *message)
{
    UBYTE *command;

    ACTION_MSG(message);
    command = concat(commandPrefix, scriptFileName);
    doCommand(command,NULL);
}

void runScriptForType(enum ActionType type)
{
    UBYTE *command;
    UBYTE *playNextScript = "PlayNext.Hip";
    UBYTE *playPrevScript = "PlayPrev.hip";
    UBYTE *showSamplesScript = "showsamples.hip";
    UBYTE *playSelectedScript = "play2.hip";
    UBYTE *rewPatternScript = "rew.hip";
    UBYTE *ffwdPatternScript = "ffwd.hip";
    UBYTE *copyToLikedModsScript = "copyfile.hip";
    UBYTE *chooseNextScript = "chooseNext.hip";
    UBYTE *choosePrevScript = "choosePrev.hip";
    UBYTE *jump10ModsBackwardsScript = "jump10ModsBackwards.HiP";
    UBYTE *jump10ModsForwardScript = "jump10ModsForward.HiP";
    UBYTE *pauseScript = "pause.HiP";
    UBYTE *volumeUpScript = "volumeUp.HiP";
    UBYTE *volumeDownScript = "volumeDown.HiP";
    ULONG result;
    
    switch(type)
    {
        case VOL_DOWN_ACTION:
        {
            executeArexxScript(volumeDownScript, "Volume down");
            break;
        }
        case VOL_UP_ACTION:
        {
            executeArexxScript(volumeUpScript, "Volume up");
            break;
        }
        case CHOOSE_PREV:
        {
            executeArexxScript(choosePrevScript, "Choose previous module");
            break;
        }
        case CHOOSE_NEXT:
        {
            executeArexxScript(chooseNextScript, "Choose next module");
            break;
        }
        case PLAY_NEXT_ACTION:
        {
            executeArexxScript(playNextScript, "Play next module");
            break;
        }
        case PLAY_PREV:
        {
            executeArexxScript(playPrevScript, "Play previous module");
            break;
        }
        case SHOW_SAMPLES:
        {
            executeArexxScript(showSamplesScript, "Show/hide samples window");
            break;
        }
        case PLAY_SELECTED:
        {
            executeArexxScript(playSelectedScript, "Play selected module");
            break;
        }
        case STOP_CONTINUE:
        {
            executeArexxScript(pauseScript, "Stop/continue");
            break;
        }
        case REW_PATTERN:
        {
            executeArexxScript(rewPatternScript, "REW pattern");
            break;
        }
        case FFWD_PATTERN:
        {
            executeArexxScript(ffwdPatternScript, "FFWD pattern");
            break;
        }
        case JUMP_10_MODS_FORWARD:
        {
            executeArexxScript(jump10ModsForwardScript, "Jump 10 modules forward");
            break;
        }
        case JUMP_10_MODS_BACKWARDS:
        {
            executeArexxScript(jump10ModsBackwardsScript, "Jump 10 modules backwards");
            break;
        }
        case COPY_TO_LIKEDMODS:
        {
            executeArexxScript(copyToLikedModsScript, "Copy module to LikedMods: volume");
            break;
        }
        case QUIT:
        {
            break;
        }
    }
}

UBYTE* concat(const UBYTE *string1, const UBYTE *string2)
{
    UBYTE *result = (UBYTE *) malloc(strlen(string1) + strlen(string2) + 1);
    
    strcpy(result, string1);
    strcat(result, string2);

    return result;
}
