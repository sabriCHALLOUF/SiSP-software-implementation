#include <Arduino_FreeRTOS.h>

  /**
   * @version     : 1.0
   * @Author      : Sabri Challouf
   * @description : This file implements the sisp protocol on Arduino platform.
   * The SiSP is described in the next pseudo-algorithm :
   * 
   *    lclk++;
   *    sclk++;
   *    if( lclk % 100 ) != 0
   *    then   
   *          if msg_received(rclk) then 
   *                                sclk = ( sclk + rclk ) / 2
   *          endif 
   *    else broadcast(sclk) 
   *    endif
   * 
   */

/************************************************************************************************************************************
 *                                              T Y P E S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
 typedef uint32_t clok_val_type;


/************************************************************************************************************************************
 *                                           C O N S T A N T E S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
#define   DEBUG_ON
#define   DEFAULT_INT_VAL   0 
#define   OK                0 
#define   CLK_BYTE_LEN      sizeof(clok_val_type)  


/************************************************************************************************************************************
 *                                   G L O B A L E      V A R I A B L E S     D E F I N I T I O N S 
 ************************************************************************************************************************************/
clok_val_type   lclk = DEFAULT_INT_VAL ;
clok_val_type   sclk = DEFAULT_INT_VAL ;


/************************************************************************************************************************************
 *                                             S U B F U N C T I O N S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
/**
 * @function  : broadcast_Sclk
 * @param     : no params
 * @return    : return OK (zero) if the function if everything is well.
 */
int broadcast_Sclk()
{
  byte b[4];
  IntegerToBytes(sclk, b);
  for (int i=0; i<4; ++i)
  {
    Serial1.write(b[i]);
    #ifdef DEBUG_
      Serial.print(i);
      Serial.print(" : ");
      Serial.println(b[i]);
    #endif
  }
  
  //Serial1.write(sclk);
  #ifdef DEBUG_
    Serial.println();
    Serial.print(" CURRENT SENT SCLK  : ");
    Serial.println(sclk);
    delay(1000);
  #endif
  
  return OK;  
}



/**
 * @function  : listen_Recieved_RCLK
 * @param     : nothing
 * @return    : return OK (zero) if the function if everything is well.
 */

  int listen_Recieved_RCLK()
  {
     if (Serial1.available()) 
      {
        clok_val_type tmpRclk=DEFAULT_INT_VAL;
        byte byte_buffer=DEFAULT_INT_VAL;   // buffer to read the recieved byte
        for(int i=0;i<CLK_BYTE_LEN;i++)
        {
          byte_buffer=Serial1.read();
          tmpRclk += (long) byte_buffer << (8*i);  // convert the 4 bytes to a long number
          #ifdef DEBUG_
            Serial.print(" recieved_byte  : ");
            Serial.println(byte_buffer);
          #endif
          
        }
        
        #ifdef DEBUG_
          Serial.print(" tmpRclk  : ");
          Serial.println(tmpRclk);
          Serial.println();
        #endif
        
        
        
        if(tmpRclk)
        {

            #ifdef DEBUG_
              Serial.print(" OLD SCLK  : ");
              Serial.print(sclk);
            #endif
            sclk = ((sclk + tmpRclk) / 2) ;
            #ifdef DEBUG_
              Serial.print("\t\t\tRCLK  : ");
              Serial.print(tmpRclk);
              Serial.print("\t\tNEW SCLK  : ");
              Serial.println(sclk);
            #endif
            #ifdef DEBUG_ON
              if(tmpRclk==sclk)
                digitalWrite(LED_BUILTIN,HIGH);
              else
                digitalWrite(LED_BUILTIN,LOW);
            #endif
        }
      }
      
      return OK;  
  }


/**
 * @function  : IntegerToBytes
 * @param     : the long value to convert into bytes
 * @param     : the table of bytes that will contain the bytes of the long value
 * @return    : nothing
 */
void IntegerToBytes(long val, byte b[CLK_BYTE_LEN])
{
  for(int i=0; i<CLK_BYTE_LEN;i++)
  {
      b[i] = (byte )((val >> (8*i)) & 0xff);
  }

  /* same as :
      b[3] = (byte )((val >> 24) & 0xff);
      b[2] = (byte )((val >> 16) & 0xff);
      b[1] = (byte )((val >> 8) & 0xff);
      b[0] = (byte )((val) & 0xff);
  */
}


/************************************************************************************************************************************
 *                                       B A S I C        F U N C T I O N S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
void setup() 
{
  // USB Serial setup
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("============== COM 555 =============");

  // Serial 1 setup
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,LOW);
}


void loop() 
{
  
  lclk++;
  sclk++;

  if(lclk % 100)
  {      
      listen_Recieved_RCLK();  
  }
  else
  {
      broadcast_Sclk();
  }

  #ifdef DEBUG_ON
    //Serial.print(" CURRENT SENT SCLK  : ");
    Serial.println(sclk);
    delay(100);
  #endif
  
}


