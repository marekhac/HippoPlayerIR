/****************************
*   Ser_Support.c           *
*    August 1988            *
*  (c) Bruno Jennrich       *
* cc Ser_Support.c          *
*****************************/
#include "exec/types.h"
#include "exec/io.h"
#include "devices/serial.h"

/**************************************************/
/* Serial_Read()     (Ser_Support)                */
/* Function: Read data                            */
/* ---------------------------------------------- */
/* Input - Parameter:                             */
/* SerReq: Device-Block                           */
/* Data: Data buffer                              */
/* Len: Amount of data to be read                 */
/**************************************************/

VOID Serial_Read (SerReq, Data, Len)
struct IOExtSer *SerReq;  /* device block to use serial port */
APTR Data;
ULONG Len;
{
  /* transfer of data is impossible with only IORequest   */
  /* because there is no data and length fields.          */
  /* so we must use IOExtSer, a device specific structure */
  
  SerReq->IOSer.io_Data = Data;  /* pointer to data to read */
  SerReq->IOSer.io_Length = Len; /* bytes to read */
  
  /* if Len is -1, the serial device will read until it encouters */
  /* a charackter from the TermArray or null byte */
  
  Do_Command (SerReq, (UWORD)CMD_READ);
}

/**************************************************/
/* Serial_Write()     (Ser_Support)               */
/* Function: Read data                            */
/* ---------------------------------------------- */
/* Input - Parameter:                             */
/* SerReq: Device-Block                           */
/* Data: Data to be written                       */
/* Len: Amount of data to be written              */
/**************************************************/

VOID Serial_Write (SerReq, Data, Len)
struct IOExtSer *SerReq;
APTR Data;
ULONG Len;
{
  
  /* io_Data - address to buffer to which data should be written */
  
  SerReq->IOSer.io_Data = Data;
  SerReq->IOSer.io_Length = Len; /* bytes to transfer */
  
  /* if Len is -1, the serial device will transfer data */
  /* until it encounters a null byte */
  
  Do_Command (SerReq, (UWORD)CMD_WRITE);
}

