#include "OTA_lib.h"
#include <stdio.h>
#include "main.h"

extern  UART_HandleTypeDef      huart1;
#define BLUETOOTH_UART          huart1






/*
can't be modified except in OTA_STATE_RESPONSE since 
a response packet must be send firstly before end of receiving
*/
static uint8_t         ReceivingFlag   =1;

static uint32_t        TotalCodeSize   =0; //Info taken from Header Packet 
static uint32_t        ReseivedCodeSize=0; //What is the size received until now
static uint32_t        TotalCodeCRC    =0; //Info taken from Header Packet 
//static uint32_t        ReseivedCodeCRC =0; //What is the CRC for the received until now

static uint8_t         LastPacketFlag  =0;  
static OTA_Response_t  Response        =OTA_RESPONSE_ACK;
static OTA_State_t     CurrentState    =OTA_STATE_IDLE;
static uint8_t         Packet[OTA_MAX_PACKET];


/*Static Functions prototype*/
static OTA_Error_t   OTA_Idle               (void);
static OTA_Error_t   OTA_Cmd                (void);
static OTA_Error_t   OTA_Response           (void);
static OTA_Error_t   OTA_Header             (void);
static OTA_Error_t   OTA_Data               (void);
static void          OTA_ReInitializePacket (void); 
static OTA_Error_t   OTA_EraseFlash         (void);     
static OTA_Error_t   OTA_FlashData          (uint32_t data_len);


OTA_Error_t OTA_ReceiveAndFlashTheCode      (void)
{
	OTA_Error_t error=OTA_ERROR_OK;
	
	while(ReceivingFlag)
	{
		switch(CurrentState)
		{   
		    case OTA_STATE_IDLE :
			{   
				OTA_ReInitializePacket();
                error=OTA_Idle();

                //if there is an error override the current state
                if(error!=OTA_ERROR_OK)
                {
                	Response=OTA_RESPONSE_NACK;
                	CurrentState=OTA_STATE_RESPONSE;
                	printf("Error in IDLE state\r\n");
                }
			}
			break;

			case OTA_STATE_CMD :
			{
            	error=OTA_Cmd();
            	Response=(OTA_ERROR_OK==error)?OTA_RESPONSE_ACK:OTA_RESPONSE_NACK;
            	CurrentState=OTA_STATE_RESPONSE;

			}
			break;
	        case OTA_STATE_HEADER :
	        {
            	error=OTA_Header();
            	Response=(OTA_ERROR_OK==error)?OTA_RESPONSE_ACK:OTA_RESPONSE_NACK;
	            CurrentState=OTA_STATE_RESPONSE;
	        } 
	        break;
	        case OTA_STATE_DATA :
	        {   
	        	error=OTA_Data();
	        	Response=(OTA_ERROR_OK==error)?OTA_RESPONSE_ACK:OTA_RESPONSE_NACK;
	        	CurrentState=OTA_STATE_RESPONSE;
	        }
	        break;
	        case OTA_STATE_RESPONSE  :
	        {   
	        	OTA_Error_t PrevError=error;
	        	OTA_ReInitializePacket();
	        	error=OTA_Response();
	        	
	        	if((error!=OTA_ERROR_OK)||(1==LastPacketFlag)||(OTA_RESPONSE_NACK==Response))
	        	{
	        		ReceivingFlag=0;
	        	}
	        	
	        	CurrentState=OTA_STATE_IDLE;
	        	error=PrevError;
	        }
	        break;
	        default:
	        break;
		}

	}

	return error;
}


static void   OTA_ReInitializePacket        (void)
{
	uint32_t iterator=0;

	for(iterator=0;iterator<OTA_MAX_PACKET;iterator++)
	 {
		Packet[iterator]=0;
	 }
}
/*
Receive the SOF,PacketType and PacketLenght
and determine what is our Current State  
*/
static OTA_Error_t   OTA_Idle               (void)
{   
	OTA_Error_t error=OTA_ERROR_OK;

    if (HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_SOF],OTA_SOF_SIZE, HAL_MAX_DELAY )!=HAL_OK)
     {
     	error=OTA_ERROR_IDLE;
     }

    if(OTA_ERROR_OK==error)
    {
    	if(Packet[OTA_INDEX_SOF]!=OTA_SOF)
     	{
     		error=OTA_ERROR_IDLE;
     	}
    }
    if(OTA_ERROR_OK==error)
    {    
	     //the received data is in little endian 
         if ((HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_TYPE],OTA_PACKET_TYPE_SIZE, HAL_MAX_DELAY )!=HAL_OK)||
		     (HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_LENGTH],OTA_LENGTH_SIZE, HAL_MAX_DELAY )   !=HAL_OK)   )
          {
     	  	error=OTA_ERROR_IDLE;
          }
          else
          {
          	switch(Packet[OTA_INDEX_TYPE])
          	{
          	 	case OTA_PACKET_TYPE_CMD:     CurrentState=OTA_STATE_CMD;
          	 		break;
          	 	case OTA_PACKET_TYPE_HEADER:  CurrentState=OTA_STATE_HEADER;
          	 		break;
          	 	case OTA_PACKET_TYPE_DATA:    CurrentState=OTA_STATE_DATA;
          	 		break;
          	 	case OTA_PACKET_TYPE_RESPONSE: CurrentState=OTA_STATE_DATA;  //can't happen
          	 		break;
          	 	default:error=OTA_ERROR_IDLE;
          	 	        printf("Not A Type\r\n");
          	 		break;
          	}
            
          }
    }

	return error;
}

static OTA_Error_t   OTA_Cmd                (void)
{
	OTA_Error_t error=OTA_ERROR_OK;
    uint16_t length=*((uint16_t *)&Packet[OTA_INDEX_LENGTH]);
    //not a CMD
    if(OTA_CMD_SIZE!=length)
    {
    	error=OTA_ERROR_CMD; 
    }
    if(OTA_ERROR_OK==error)
    {   
    	//if there is any error in receiving any of those the others will be short circuited
    	//else all of them will be received
    	if ((HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_CMD],OTA_CMD_SIZE, HAL_MAX_DELAY )!=HAL_OK)               ||
    	    (HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_CRC+OTA_CMD_SIZE-1],OTA_CRC_SIZE, HAL_MAX_DELAY )!=HAL_OK)||
            (HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_EOF+OTA_CMD_SIZE-1],OTA_EOF_SIZE, HAL_MAX_DELAY )!=HAL_OK)  )
    	{
    		error=OTA_ERROR_CMD;
    	}

    }
    if((OTA_ERROR_OK==error))
    {
		if(Packet[OTA_INDEX_EOF+OTA_CMD_SIZE-1]!=OTA_EOF)
    	{
			error=OTA_ERROR_CMD;
		}
    }
    

    /*handling CRC should be done here*/


    if(OTA_ERROR_OK==error)
    {
    	switch(Packet[OTA_INDEX_CMD])
    	{
    		case OTA_CMD_START:
    		{
    			printf("Start CMD\r\n");
    			error=OTA_EraseFlash();
    		}
    		break;
	        case OTA_CMD_END:
	        {
	        	printf("End CMD\r\n");
	        	LastPacketFlag=1;
	        	if(ReseivedCodeSize!=TotalCodeSize)
	        	{
	        		printf("Error,ReseivedCodeSize!=TotalCodeSize\r\n");
	        		error=OTA_ERROR_CMD;
	        	}
	        }
	        break;
	        case OTA_CMD_ABORT:
	        {
	        	printf("Abort CMD\r\n");
				LastPacketFlag=1;
				error=OTA_ERROR_CMD;
	        }
	        break;
	        default:
	        {
	        	printf("Wrong CMD\r\n");
	        	error=OTA_ERROR_CMD;
	        }
	        break;
    	}

    }
   
    
	return error;
}

static OTA_Error_t   OTA_EraseFlash         (void)
{
      OTA_Error_t error=OTA_ERROR_OK;
	  printf("Erasing the Flash memory...\r\n");
	  
	  if(HAL_FLASH_Unlock()!=HAL_OK)
	  {
      	 error=OTA_ERROR_FLASH;      
	  }
      //Erase the Flash
      FLASH_EraseInitTypeDef EraseInitStruct;
      uint32_t SectorError;

      EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
      EraseInitStruct.Sector        = FLASH_SECTOR_5;
      EraseInitStruct.NbSectors     = 3;                    //erase 3 sectors(5,6,7)
      EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
      if(OTA_ERROR_OK==error)
      {
      	if (HAL_FLASHEx_Erase( &EraseInitStruct, &SectorError )!= HAL_OK )
      	{
      		error=OTA_ERROR_FLASH;
      	}
      }
      if(HAL_FLASH_Lock()!=HAL_OK)
	  {
      	  error=OTA_ERROR_FLASH;      
	  }

      return error;
}

static OTA_Error_t   OTA_Header             (void)
{
	OTA_Error_t error=OTA_ERROR_OK;
    uint16_t length=*((uint16_t *)&Packet[OTA_INDEX_LENGTH]);
    
    //not a Header
    if(OTA_HEADER_SIZE!=length)
    {
    	error=OTA_ERROR_HEADER; 
    }
    if(OTA_ERROR_OK==error)
    {

    	//if there is any error in receiving any of those the others will be short circuited
    	//else all of them will be received
    	if ((HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_HEADER],OTA_HEADER_SIZE, HAL_MAX_DELAY )!=HAL_OK)            ||
    	    (HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_CRC+OTA_HEADER_SIZE-1],OTA_CRC_SIZE, HAL_MAX_DELAY )!=HAL_OK)||
            (HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_EOF+OTA_HEADER_SIZE-1],OTA_EOF_SIZE, HAL_MAX_DELAY )!=HAL_OK)  )
    	{
    		error=OTA_ERROR_HEADER;
    	}

    }
    if(OTA_ERROR_OK==error)
    {   
		if(Packet[OTA_INDEX_EOF+OTA_HEADER_SIZE-1]!=OTA_EOF)
		{
			error=OTA_ERROR_HEADER;
		}
    }
    if(OTA_ERROR_OK==error)
    {
    //the first 4bytes contains the total code size
    TotalCodeSize=*((uint32_t*)&Packet[OTA_INDEX_HEADER+OTA_HEADER_CODE_SIZE]);
    //the second 4bytes contains the total code CRC
    TotalCodeCRC =*((uint32_t*)&Packet[OTA_INDEX_HEADER+OTA_HEADER_CODE_CRC ]);
    }

    printf("TotalCodeSize=%ld \r\n",TotalCodeSize);

    /*handling CRC should be done here*/

    return error;

}

static OTA_Error_t   OTA_Data               (void)
{
	OTA_Error_t error=OTA_ERROR_OK;
    uint16_t length=*((uint16_t *)&Packet[OTA_INDEX_LENGTH]);
    //can't receive data more than the max data
    if(length>OTA_MAX_DATA)
    {
    	error=OTA_ERROR_DATA; 
    }
    if(OTA_ERROR_OK==error)
    {
    	//if there is any error in receiving any of those the others will be short circuited
    	//else all of them will be received
    	if ((HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_DATA],length, HAL_MAX_DELAY )!=HAL_OK)              ||
    	    (HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_CRC+length-1],OTA_CRC_SIZE, HAL_MAX_DELAY )!=HAL_OK)||
    	    (HAL_UART_Receive( &BLUETOOTH_UART,& Packet[OTA_INDEX_EOF+length-1],OTA_EOF_SIZE, HAL_MAX_DELAY )!=HAL_OK)  )
    	{
    		error=OTA_ERROR_DATA;
    	}
    }
    if(OTA_ERROR_OK==error)
    {
    	if(Packet[OTA_INDEX_EOF+length-1]!=OTA_EOF)
		{
			error=OTA_ERROR_DATA;	
		}
    }
    if(OTA_ERROR_OK==error)
    {
    	printf("DataSize=%d\r\n",length);
    	error=OTA_FlashData(length);
    }



    return error;
}

static OTA_Error_t   OTA_FlashData          (uint32_t data_len)
{   
	uint32_t i=0;
	OTA_Error_t error=OTA_ERROR_OK;
	printf("Flashing %ld[Bytes]\r\n",data_len);

	if(HAL_FLASH_Unlock()!=HAL_OK)
	{    printf("Cant unlock flash");
   		 error=OTA_ERROR_FLASH;      
	}

    for(i = 0;i < data_len; i++ )
    {
      if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE,(OTA_APP_ADDRESS+ReseivedCodeSize),Packet[OTA_INDEX_DATA+i])==HAL_OK)
      {
        ReseivedCodeSize += 1;
      }
      else
      {
        error=OTA_ERROR_FLASH;
        break;
      }
    }
    if(HAL_FLASH_Lock()!=HAL_OK)
	{
    	printf("Cant lock flash");
   		 error=OTA_ERROR_FLASH;      
	}

    if(OTA_ERROR_OK==error)
    {
    	printf("[%ld/%ld]\r\n",ReseivedCodeSize/OTA_MAX_DATA,TotalCodeSize/OTA_MAX_DATA);
    }

	return error;
}

static OTA_Error_t   OTA_Response           (void)
{   
    OTA_Error_t error=OTA_ERROR_OK;
	Packet[OTA_INDEX_SOF] =OTA_SOF;
	Packet[OTA_INDEX_TYPE]=OTA_PACKET_TYPE_RESPONSE;
	
	uint16_t* LengthPtr=(uint16_t*)&Packet[OTA_INDEX_LENGTH];
	*LengthPtr =OTA_RESPONSE_SIZE;
	
	Packet[OTA_INDEX_RESPONSE]=Response;
	
	uint32_t* CRCPtr=(uint32_t*)&Packet[OTA_INDEX_CRC+OTA_RESPONSE_SIZE-1];
	*CRCPtr=0x00000000; //you should calculate the real CRC here
	
	Packet[OTA_INDEX_EOF+OTA_RESPONSE_SIZE-1]=OTA_EOF;
	HAL_Delay(3);
	if(HAL_UART_Transmit(&BLUETOOTH_UART,Packet,OTA_RESPONSE_SIZE+OTA_PACKET_INFO_BYTES, HAL_MAX_DELAY)!=HAL_OK)
	{
		error=OTA_ERROR_RESPONSE;
	}
    if(OTA_ERROR_OK==error)
	{
		Response?printf("Response ACK\r\n"):printf("Response NACK\r\n");
	}

	return error;
}
