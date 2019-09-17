/*****************************************/
/* Device-Support Functions              */
/* (c) Bruno Jennrich, June 8 1988       */
/* -----------------------------------   */
/* Compile info: cc Devs_Support         */
/*****************************************/

#include "exec/types.h"
#include "exec/io.h"
#include "exec/devices.h"

VOID CloseIt();
VOID *CreatePort();
VOID *CreateExtIO();
VOID DeletePort();
VOID DeleteExtIO();

/**************************************************/
/* GetDeviceBlock ()                              */
/* Function: Device-Block open and initialization */
/* ---------------------------------------------- */
/* Input: Size of device-blocks in bytes          */
/* ---------------------------------------------- */
/* Return: Initialize device-block                */
/**************************************************/

APTR GetDeviceBlock (Size)
ULONG Size;
{
	struct MsgPort *Device_Port;
  APTR Device_Request; /* no IORequest - APTR is universal */
	
	/* try to allocate Device-Port if this is not possible, leave program */
	/* Device_Port is a IOReplyPort (a message port) for message reports etc. */
  /* it holds reports to the system */
  
  /* CreatePort (Name, Prority) is exec support funciton */
  /* using devices msg ports have no names */
  
	Device_Port = (struct MsgPort *) CreatePort (0,0);
  
  /* CreatePort returns an "anchor" which we use in CreateExtIO() and OpenDevice() */
  
  if(Device_Port == 0)
  {
    CloseIt ("Couldnt get DEVICE-PORT!");
  }
  
  /* tries to allocate Device-Block. If that is not possible */
  /* give Device-Port back and leave program */
  
  /* CreateExtIO is exec support funciton to initialize */
  /* device structures. It makes an IORequest, */
  /* allocates memory for a device request structure */
  /* and set Device_Port as mn_ReplyPort etc. */
   
  Device_Request = (APTR) CreateExtIO (Device_Port, Size);
      
  if(Device_Request == 0)
  {
    DeletePort (Device_Port);
    CloseIt ("Couldnt get DEVICE-BLOCK!");
  }
  
  /* Give back previously installed Device-Block */
  
  return (Device_Request); /* return a POINTER! */
}

/**************************************************/
/* FreeDeviceBlock ()                             */
/* Function: Release Device-Block                 */
/* ---------------------------------------------- */
/* Input: IORequest                               */
/* IORequest: Release Device-Block                */
/**************************************************/

VOID FreeDeviceBlock (IORequest)
struct IORequest *IORequest;
{
 /* if IORequest can be opened, free up */
 /* Device-Port. The free up IORequest  */

 /* device can be used one at a time, so if we don't need it */
 /* we delete Device port and device block */
  
  if (IORequest != 0)
  {
    if (IORequest->io_Message.mn_ReplyPort != 0)
    {
      /* delete port (Device_Port set as mn_ReplyPort previously) */
      
      DeletePort (IORequest->io_Message.mn_ReplyPort);
    }
    DeleteExtIO(IORequest);
  }
}

/**************************************************/
/* Open_A_Device ()                               */
/* Function: Open any Device                      */
/* ---------------------------------------------- */
/* Input: - Parameter                             */
/* Name: Name the Device (i.e. "serial.device     */
/* Unit: Device-Unit                              */
/* Device_Request: Pointer to block to be         */
/*                 initialized (Device-Block)     */
/* Flags: Device-Flags                            */
/* Size: Size of the Device-Blocks                */
/**************************************************/

VOID Open_A_Device (Name, Unit, Device_Request, Flags, Size)
char *Name;
ULONG Unit;
APTR *Device_Request;
ULONG Flags;
ULONG Size;
{
  UWORD Error;  /* Error from OpenDevice() */
  
  /* if Size > 0, allocate Device-Block */
  /* if Size == 0, use initialized device block from user */
  
  if (Size != 0)
  {
    *Device_Request = GetDeviceBlock(Size);
  }
  
  /* Open Device */
  /* NOTE !!! "Device_Request" is a pointer to pointer! (**DevReq) */
  
  Error = OpenDevice (Name, Unit, *Device_Request, Flags);
  
  if(Error != 0)
  {
    printf("Open-Device Error#%41x\n",Error);
    CloseIt ("Couldnt get DEVICE!");
  }

}

/**************************************************/
/* Close_A_Device  ()                             */
/* Function: Device-Block free and close Device   */
/* ---------------------------------------------- */
/* Input: IORequest                               */
/* IORequest: Release Device-Block                */
/**************************************************/

VOID Close_A_Device (IORequest)
struct IORequest *IORequest;
{
  /* if IORequest can be opened, release Device-Port. Close Device */
  /* the release IORequest  */
  
  if (IORequest != 0)
  {
    if (IORequest->io_Message.mn_ReplyPort != 0)
      DeletePort (IORequest->io_Message.mn_ReplyPort);
    if (IORequest->io_Device != 0)
    {
      /* CloseDevice ensures that the device can be used by other programs */
      /* IORequest->io_Device is a pointer to opened device, CloseDevice() */
      /* extracts this pointer, and using it to close the device */
      
      CloseDevice (IORequest); /* send "close" command to io_Device */
    }

    DeleteExtIO (IORequest); /* release IORequest i.e. IOExtSer */
  }
}

/**************************************************/
/* Do_Command ()                                  */
/* Function: Execute Command                      */
/* ---------------------------------------------- */
/* Input - Parameter:                             */
/* DeviceBlock: Device-Block                      */
/* Command: Command                               */
/**************************************************/

VOID Do_Command (DeviceBlock, Command)
struct IORequest *DeviceBlock;
UWORD Command;
{
  DeviceBlock->io_Command = Command; /* device command is stored here */
  DoIO(DeviceBlock);
}

VOID Do_Abort (DeviceBlock, Command)
struct IORequest *DeviceBlock;
UWORD Command;
{
  DeviceBlock->io_Command = Command; /* device command is stored here */
  AbortIO(DeviceBlock);
}