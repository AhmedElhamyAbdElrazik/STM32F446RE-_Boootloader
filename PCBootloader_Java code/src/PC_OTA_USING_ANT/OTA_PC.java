/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/Classes/Class.java to edit this template
 */
package PC_OTA_USING_ANT;


import java.util.Arrays;
import java.util.concurrent.TimeUnit;
import java.io.IOException;
import static PC_OTA_USING_ANT.BootloaderGUI.MyInputStream;
import static PC_OTA_USING_ANT.BootloaderGUI.MyOutputStream;

public class OTA_PC {
   
 
    
   final static byte OTA_INDEX_SOF           =0;    //1byte
   final static byte OTA_INDEX_TYPE          =1;  //1byte
   final static byte OTA_INDEX_LENGTH        =2; //2bytes  2,3
   final static byte OTA_INDEX_CMD           =4;
   final static byte OTA_INDEX_HEADER        =4;
   final static byte OTA_INDEX_RESPONSE      =4;
   final static byte OTA_INDEX_DATA          =4;
   final static byte OTA_INDEX_CRC           =5;//4bytes 5,6,7,8
   final static byte OTA_INDEX_EOF           =9;

   final static byte OTA_STATE_IDLE          =0;     
   final static byte OTA_STATE_CMD           =1;
   final static byte OTA_STATE_HEADER        =2;
   final static byte OTA_STATE_DATA          =3;
   final static byte OTA_STATE_RESPONSE      =4;

   final static byte OTA_CMD_START           =0;
   final static byte OTA_CMD_END             =1;
   final static byte OTA_CMD_ABORT           =2;

   final static byte OTA_RESPONSE_NACK       =0;
   final static byte OTA_RESPONSE_ACK        =1; 
       
   final static byte OTA_HEADER_CODE_SIZE    =0;     //starts from the first  4 bytes in the header
   final static byte OTA_HEADER_CODE_CRC     =4;

   final static byte OTA_ERROR_OK            =0;      //     Success
   final static byte OTA_ERROR_FLASH         =1;      // not Success while flashing or erasing
   final static byte OTA_ERROR_CMD           =2;      // not Success in CMD      packet
   final static byte OTA_ERROR_HEADER        =3;      // not Success in header   packet
   final static byte OTA_ERROR_DATA          =4;      // not Success in data     packet
   final static byte OTA_ERROR_RESPONSE      =5;      // not Success in response packet
   final static byte OTA_ERROR_IDLE          =6;

   final static byte OTA_PACKET_TYPE_CMD     =0; 
   final static byte OTA_PACKET_TYPE_HEADER  =1;
   final static byte OTA_PACKET_TYPE_DATA    =2;  
   final static byte OTA_PACKET_TYPE_RESPONSE=3;
   
    
   private final static int   OTA_APP_ADDRESS       = 0x08020000;   //
   private final static byte  OTA_SOF               = (byte)0xAA;
   private final static byte  OTA_EOF               = (byte)0xEE;
   //max data thastatic t ca n be recived in one packet excluding other info in the packet
           final static char  OTA_MAX_DATA         =  1024;
   private final static char  OTA_PACKET_INFO_BYTES=  9;
   private final static char  OTA_MAX_PACKET       =  (OTA_MAX_DATA+OTA_PACKET_INFO_BYTES);
           final static int   OTA_MAX_CODE_SIZE    =  OTA_MAX_DATA*384;
   private final static byte  OTA_SOF_SIZE         =  1;
   private final static byte  OTA_PACKET_TYPE_SIZE =  1;
   private final static byte  OTA_LENGTH_SIZE      =  2;  
   private final static byte  OTA_CMD_SIZE         =  1;
   private final static byte  OTA_HEADER_SIZE      =  8;
   private final static byte  OTA_RESPONSE_SIZE    =  1;
   private final static byte  OTA_CRC_SIZE         =  4;
   private final static byte  OTA_EOF_SIZE         =  1;
   private static final byte Packet[]    =new byte[OTA_MAX_PACKET];
   
   //Size of all packet parts in byte

   
   
private static void  OTA_ReInitializePacket ()
   {
      Arrays.fill(Packet, (byte)0);
   }
static byte          OTA_SendCMDPacket      (byte Cmd) throws InterruptedException, IOException
{
    OTA_ReInitializePacket();
    int TotalPacketLenght=OTA_PACKET_INFO_BYTES+OTA_CMD_SIZE;

    Packet[OTA_INDEX_SOF]     = OTA_SOF;
    Packet[OTA_INDEX_TYPE]    =OTA_PACKET_TYPE_CMD;
    
    Packet[OTA_INDEX_LENGTH]  =OTA_CMD_SIZE;
    Packet[OTA_INDEX_LENGTH+1]=(OTA_CMD_SIZE>>8);
    
    Packet[OTA_INDEX_CMD]     =Cmd;
    //To do :CRC
    //set packet CRC
    for(int i=0;i<4;i++)
    {
      //To do :Calculate CRC for packet instead of this 0 value
       Packet[OTA_INDEX_CRC+OTA_CMD_SIZE-1+i]=0;
    }

    Packet[OTA_INDEX_EOF+OTA_CMD_SIZE-1]=OTA_EOF;
    
    TimeUnit.MICROSECONDS.sleep(300);
    
    for(int i = 0; i < TotalPacketLenght; i++)
    {
        TimeUnit.MICROSECONDS.sleep(1);
        MyOutputStream.write(Packet[i]);

    }
  return OTA_ReceiveResponsePacket ();

}
   
static byte          OTA_SendHeaderPacket      (int CodeSize,int CodeCRC) throws InterruptedException, IOException
{
    OTA_ReInitializePacket();
    int TotalPacketLenght=OTA_PACKET_INFO_BYTES+OTA_HEADER_SIZE;

    Packet[OTA_INDEX_SOF] =OTA_SOF;
    Packet[OTA_INDEX_TYPE]=OTA_PACKET_TYPE_HEADER;

    Packet[OTA_INDEX_LENGTH]  =OTA_HEADER_SIZE;
    Packet[OTA_INDEX_LENGTH+1]=(OTA_HEADER_SIZE>>8);
    //set total code size and total code CRC (header packet content)&packet CRC
    for(int i=0;i<4;i++)
    {
       Packet[OTA_INDEX_HEADER+OTA_HEADER_CODE_SIZE+i]= (byte)(CodeSize>>(i*8));
       //To do :Calculate CRC for the whole CODE 
       Packet[OTA_INDEX_HEADER+OTA_HEADER_CODE_CRC+i]=  (byte) (CodeCRC>>(i*8));
       //To do :Calculate CRC for packet instead of this 0 value
       Packet[OTA_INDEX_CRC+OTA_HEADER_SIZE-1+i]=0;
    }


    Packet[OTA_INDEX_EOF+OTA_HEADER_SIZE-1]=OTA_EOF;

    TimeUnit.MICROSECONDS.sleep(300);
    for(int i = 0; i < TotalPacketLenght; i++)
    {
         TimeUnit.MICROSECONDS.sleep(1);
         MyOutputStream.write(Packet[i]);
    }
    
    return OTA_ReceiveResponsePacket ();
}

static byte          OTA_SendDataPacket(byte Data[],int StartingPosition,int DataLength) throws InterruptedException, IOException
{
    OTA_ReInitializePacket();
    int TotalPacketLenght=OTA_PACKET_INFO_BYTES+DataLength;

    Packet[OTA_INDEX_SOF] =OTA_SOF;
    Packet[OTA_INDEX_TYPE]=OTA_PACKET_TYPE_DATA;

    Packet[OTA_INDEX_LENGTH]  =(byte)DataLength;
    Packet[OTA_INDEX_LENGTH+1]=(byte)(DataLength>>8);
    
    System.arraycopy(Data,StartingPosition, Packet,OTA_INDEX_DATA,DataLength);

    //set packet CRC
    for(int i=0;i<4;i++)
    {
      //To do :Calculate CRC for packet instead of this 0 value
       Packet[OTA_INDEX_CRC+DataLength-1+i]=0;
    }

    Packet[OTA_INDEX_EOF+DataLength-1]=OTA_EOF;
    
    TimeUnit.MICROSECONDS.sleep(300);
    for(int i = 0; i < TotalPacketLenght; i++)
    {
        TimeUnit.MICROSECONDS.sleep(1);
        MyOutputStream.write(Packet[i]);

    }
    return OTA_ReceiveResponsePacket (); 
}

static byte          OTA_ReceiveResponsePacket () throws InterruptedException, IOException
{
    OTA_ReInitializePacket();
    int TotalPacketLenght=OTA_PACKET_INFO_BYTES+OTA_RESPONSE_SIZE;

    byte error =OTA_ERROR_OK;
    
    TimeUnit.MICROSECONDS.sleep(500);
    for(int i=0;i<TotalPacketLenght;i++)
    {
        TimeUnit.MICROSECONDS.sleep(1);
        Packet[i]=(byte)MyInputStream.read();
   
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
    if(Packet[OTA_INDEX_EOF+OTA_RESPONSE_SIZE-1]!=OTA_EOF)
    {
        error=OTA_ERROR_RESPONSE;
    }

    /*check crc here */

    if(error!=OTA_ERROR_OK)
    {   
        error=OTA_ERROR_RESPONSE;
    }

   return error;
}
}


