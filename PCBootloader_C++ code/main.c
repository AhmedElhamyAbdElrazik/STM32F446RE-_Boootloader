#include <stdio.h>
#include <stdlib.h>
#include "OTA_PC_lib.h"
#include "rs232.h"




uint8_t APP_BIN[OTA_MAX_CODE_SIZE];

int main(int argc, char *argv[])
{
    int comport;
    int bdrate   = 9600;       /* 115200 baud */
    char mode[]= {'8','N','1',0}; /* *-bits, No parity, 1 stop bit */
    char bin_name[1024];
    FILE *Fptr = NULL;

    do
    {
        if( argc <= 2 )
        {
            printf("Please feed the COM PORT number and the Application Image....!!!\n");
            printf("Example: .\\etx_ota_app.exe 8 ..\\..\\Application\\Debug\\Blinky.bin");
            break;
        }

        //get the COM port Number
        comport = atoi(argv[1]) -1;
        strcpy(bin_name, argv[2]);

        printf("Opening COM%d...\n", comport+1 );

        if( RS232_OpenComport(comport, bdrate, mode, 0) )
        {
            printf("Can not open comport\n");
            break;
        }

        //send OTA Start command
        OTA_SendCMDPacket(comport,OTA_CMD_START);
        if(OTA_ReceiveResponsePacket(comport)!=OTA_ERROR_OK)
        {
            break;
        }

        printf("Opening Binary file : %s\n", bin_name);
        Fptr = fopen(bin_name,"rb");
        if( Fptr == NULL )
        {
            printf("Can not open %s\n", bin_name);
            break;
        }

        fseek(Fptr, 0L, SEEK_END);
        uint32_t app_size = ftell(Fptr);
        fseek(Fptr, 0L, SEEK_SET);

        printf("File size = %d\n", app_size);

        //Send OTA Header
        //TODO: Add CRC
        OTA_SendHeaderPacket(comport,app_size,0);
        if(OTA_ReceiveResponsePacket(comport)!=OTA_ERROR_OK)
        {
            break;
        }
        //read the full image
        if( fread( APP_BIN, 1, app_size, Fptr ) != app_size )
        {
            printf("App/FW read Error\n");
            break;
        }

        uint16_t size = 0;

        for( uint32_t i = 0; i < app_size; )
        {
            if( ( app_size - i ) >= OTA_MAX_DATA )
            {
                size = OTA_MAX_DATA;
            }
            else
            {
                size = app_size - i;
            }


            printf("[%d/%d]\r\n", (i+size)/OTA_MAX_DATA, app_size/OTA_MAX_DATA);
            OTA_SendDataPacket(comport,&APP_BIN[i],size);
            if(OTA_ReceiveResponsePacket(comport)!=OTA_ERROR_OK)
            {

            }

           i += size;
        }

        OTA_SendCMDPacket(comport,OTA_CMD_END);
        if(OTA_ReceiveResponsePacket(comport)!=OTA_ERROR_OK)
        {
            break;
        }
    }while (0);

    if(Fptr)
    {
        fclose(Fptr);
    }

    return(0);
}
