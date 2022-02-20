#include "OTA_PC_lib.h"
#include "rs232.h"
#include <stdio.h>
#include <time.h>
#include<windows.h>
static uint8_t Packet[OTA_MAX_PACKET];



static void          OTA_ReInitializePacket (void);
void                 delay(uint32_t milliseconds);


void         OTA_SendCMDPacket         (uint32_t COMPort,OTA_Command_t Cmd)
{
    OTA_ReInitializePacket();
    uint32_t TotalPacketLenght=OTA_PACKET_INFO_BYTES+OTA_CMD_SIZE;

    Packet[OTA_INDEX_SOF]=OTA_SOF;
    Packet[OTA_INDEX_TYPE]=OTA_PACKET_TYPE_CMD;
    uint16_t* LengthPtr=(uint16_t*)&Packet[OTA_INDEX_LENGTH];
    *LengthPtr =OTA_CMD_SIZE;
    Packet[OTA_INDEX_CMD]=Cmd;
    uint32_t* CRCPtr=(uint32_t*)&Packet[OTA_INDEX_CRC+OTA_CMD_SIZE-1];
    *CRCPtr=0x00000000; //you should calculate the real CRC here
    Packet[OTA_INDEX_EOF+OTA_CMD_SIZE-1]=OTA_EOF;

    for(uint32_t i = 0; i < TotalPacketLenght; i++)
    {
        delay(1);
        if(RS232_SendByte(COMPort, Packet[i]))
        {
            printf("OTA CMD Err\n");
            break;
        }
    }

}

OTA_Error_t  OTA_ReceiveResponsePacket (uint32_t COMPort)
{
    OTA_ReInitializePacket();
    uint32_t TotalPacketLenght=OTA_PACKET_INFO_BYTES+OTA_RESPONSE_SIZE;

    OTA_Error_t error =OTA_ERROR_OK;
    if(RS232_PollComport( COMPort,Packet,TotalPacketLenght))
    {
         for(uint32_t i=0;i<TotalPacketLenght;i++)
         {
             printf("ox%x \n",Packet[i]);
         }

        if(Packet[OTA_INDEX_SOF]!=OTA_SOF)
        {
            error=OTA_ERROR_RESPONSE;
        }
        if(Packet[OTA_INDEX_TYPE]!=OTA_PACKET_TYPE_RESPONSE)
        {
            error=OTA_ERROR_RESPONSE;
        }
        if(Packet[OTA_INDEX_LENGTH]!=OTA_RESPONSE_SIZE)
        {
            error=OTA_ERROR_RESPONSE;
        }
        if(Packet[OTA_INDEX_RESPONSE]!=OTA_RESPONSE_ACK)
        {
            error=OTA_ERROR_RESPONSE;
        }

        /*check crc here */

        if(Packet[OTA_INDEX_EOF+OTA_RESPONSE_SIZE-1]!=OTA_EOF)
        {
            error=OTA_ERROR_RESPONSE;
        }

    }

    else
    {
        printf("Error in receiving Response Packet\n");
        error=OTA_ERROR_RESPONSE;
    }


 return error;
}


void         OTA_SendHeaderPacket      (uint32_t COMPort,uint32_t CodeSize,uint32_t CodeCRC)
{
    OTA_ReInitializePacket();
    uint32_t TotalPacketLenght=OTA_PACKET_INFO_BYTES+OTA_HEADER_SIZE;

    Packet[OTA_INDEX_SOF]=OTA_SOF;
    Packet[OTA_INDEX_TYPE]=OTA_PACKET_TYPE_HEADER;

    uint16_t* LengthPtr=(uint16_t*)&Packet[OTA_INDEX_LENGTH];
    *LengthPtr =OTA_HEADER_SIZE;

    uint32_t* CodeSizePtr=(uint32_t*)&Packet[OTA_INDEX_HEADER+OTA_HEADER_CODE_SIZE];
    *CodeSizePtr=CodeSize;

    uint32_t* CodeCRCPtr=(uint32_t*)&Packet[OTA_INDEX_HEADER+OTA_HEADER_CODE_CRC];
    *CodeCRCPtr=CodeCRC;

    uint32_t* CRCPtr=(uint32_t*)&Packet[OTA_INDEX_CRC+OTA_HEADER_SIZE-1];
    *CRCPtr=0x00000000; //you should calculate the real CRC here

    Packet[OTA_INDEX_EOF+OTA_HEADER_SIZE-1]=OTA_EOF;

    printf("In Header Packet size is %d \n",CodeSize);
    for(uint32_t i = 0; i < TotalPacketLenght; i++)
    {
        delay(1);
        if(RS232_SendByte(COMPort, Packet[i]))
        {
            printf("OTA Header Err\n");
            break;
        }
    }
}

void         OTA_SendDataPacket        (uint32_t COMPort,uint8_t* Data,uint32_t DataLenght)
{
    OTA_ReInitializePacket();
    uint32_t TotalPacketLenght=OTA_PACKET_INFO_BYTES+DataLenght;
    if(TotalPacketLenght>OTA_MAX_PACKET)
    {
        printf("the DataLength must be less than %d",OTA_MAX_DATA);
        return;
    }
    Packet[OTA_INDEX_SOF]=OTA_SOF;
    Packet[OTA_INDEX_TYPE]=OTA_PACKET_TYPE_DATA;

    uint16_t* LengthPtr=(uint16_t*)&Packet[OTA_INDEX_LENGTH];
    *LengthPtr =DataLenght;
    printf("In Data Packet size is %d\n",DataLenght);

    for(uint32_t i=0; i<DataLenght; i++)
    {
        Packet[OTA_INDEX_DATA+i]=Data[i];
    }

    uint32_t* CRCPtr=(uint32_t*)&Packet[OTA_INDEX_CRC+DataLenght-1];
    *CRCPtr=0x00000000; //you should calculate the real CRC here

    Packet[OTA_INDEX_EOF+DataLenght-1]=OTA_EOF;


    for(uint32_t i = 0; i < TotalPacketLenght; i++)
    {
        //delay here
        if(RS232_SendByte(COMPort, Packet[i]))
        {
            printf("OTA Data Err\n");
            break;
        }
        //printf("In Data Packet Byte no. %d is %x\n",i,Packet[i]);
    }
}

static void OTA_ReInitializePacket    (void)
{
    uint32_t iterator=0;

    for(iterator=0; iterator<OTA_MAX_PACKET; iterator++)
    {
        Packet[iterator]=0;
    }
}

void delay(uint32_t milliseconds)
{
    long pause;
    clock_t now,then;

    pause = milliseconds*(CLOCKS_PER_SEC/1000);
    now = then = clock();
    while( (now-then) < pause )
        now = clock();
}
