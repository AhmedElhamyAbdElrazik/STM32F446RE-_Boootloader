/*======================================================================
Author : Ahmed ELhamy
version: 1.0
Date   : 2/1/2022

OTA :means ON The Air which describe how our code helps us to upload any code 
     using wireless communication (Bluetooth in our case)
========================================================================= */

#ifndef OTA_LIB_H_
#define OTA_LIB_H_


/*
 * OTA Packet format
 * Each Packet contains 9 bytes(constant number) + variable no of bytes determined according to its type
 *                              
 *                                  one of these
 *                                    is send  
 * ======================================================================
 * |         | Packet    |         |                |          |        |
 * | SOF(1B) | Type(1B)  | Len(2B) | CMD     (1B)   | CRC (4B) | EOF(1B)| 
 * |         |           |         | HEADER  (8B)   |          |        |
 * | (0xAA)  |           |         | DATA    (nB)   |          | (0xEE) |
 * |         |           |         | response(1B)   |          |        |
 * ======================================================================
 *  
 */

//change it with your application address
#define OTA_APP_ADDRESS         0x08020000 




#define OTA_SOF                 0xAA
#define OTA_EOF                 0xEE
//max data that can be recived in one packet excluding other info in the packet
#define OTA_MAX_DATA            1024 
#define OTA_PACKET_INFO_BYTES   9  
#define OTA_MAX_PACKET          (OTA_MAX_DATA+OTA_PACKET_INFO_BYTES)   
#define OTA_MAX_CODE_SIZE       OTA_MAX_DATA*384    //you used only 3 sectors 5,6,7
//Size of all packet parts in byte 
#define OTA_SOF_SIZE            1U
#define OTA_PACKET_TYPE_SIZE    1U
#define OTA_LENGTH_SIZE         2U
#define OTA_CMD_SIZE            1U
#define OTA_HEADER_SIZE         8U
#define OTA_RESPONSE_SIZE       1U
#define OTA_CRC_SIZE            4U
#define OTA_EOF_SIZE            1U

//used as packet array index
typedef enum {
	 OTA_INDEX_SOF=0,    //1byte
	 OTA_INDEX_TYPE=1,   //1byte
	 OTA_INDEX_LENGTH=2, //2bytes  2,3
	 OTA_INDEX_CMD=4,OTA_INDEX_HEADER=4,OTA_INDEX_RESPONSE=4,OTA_INDEX_DATA=4,
	 OTA_INDEX_CRC=5,   //4bytes 5,6,7,8
	 OTA_INDEX_EOF=9
}OTA_PacketIndex_t;

typedef enum{
	 OTA_STATE_IDLE   ,
	 OTA_STATE_CMD    ,
	 OTA_STATE_HEADER ,
	 OTA_STATE_DATA   ,
	 OTA_STATE_RESPONSE
}OTA_State_t;

typedef enum {
	 OTA_CMD_START,
	 OTA_CMD_END  ,
	 OTA_CMD_ABORT 
}OTA_Command_t;

typedef enum {
	 OTA_RESPONSE_NACK,
	 OTA_RESPONSE_ACK 
}OTA_Response_t;

typedef enum{
	 OTA_HEADER_CODE_SIZE,  //starts from the first  4 bytes in the header
     OTA_HEADER_CODE_CRC=4  //starts from the second 4 bytes in the header
}OTA_Header_t;

typedef enum{
     OTA_ERROR_OK      ,    //     Success
     OTA_ERROR_FLASH   ,    // not Success while flashing or erasing
     OTA_ERROR_CMD     ,    // not Success in CMD      packet
     OTA_ERROR_HEADER  ,    // not Success in header   packet
     OTA_ERROR_DATA    ,    // not Success in data     packet
	 OTA_ERROR_RESPONSE,   // not Success in response packet
     OTA_ERROR_IDLE  
}OTA_Error_t;

typedef enum{
	 OTA_PACKET_TYPE_CMD    ,
	 OTA_PACKET_TYPE_HEADER ,
	 OTA_PACKET_TYPE_DATA   ,
	 OTA_PACKET_TYPE_RESPONSE
}OTA_Packet_t;



OTA_Error_t   OTA_ReceiveAndFlashTheCode ();








#endif /* INC_ETX_OTA_UPDATE_H_ */
