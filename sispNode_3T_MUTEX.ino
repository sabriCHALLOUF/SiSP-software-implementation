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

#include <Arduino_FreeRTOS.h> 
#include <semphr.h> 



/************************************************************************************************************************************
 *                                              T Y P E S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
 typedef uint32_t clok_val_type;


/************************************************************************************************************************************
 *                                           C O N S T A N T E S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
//#define   DEBUG_ON
//#define   DEBUG_ON_LED
#define   DEBUG_PRT_CLK
#define   DEFAULT_INT_VAL   0 
#define   OK                0 
#define   CLK_BYTE_LEN      sizeof(clok_val_type)  


/************************************************************************************************************************************
 *                                   G L O B A L E      V A R I A B L E S     D E F I N I T I O N S 
 ************************************************************************************************************************************/
//clok_val_type   lclk = DEFAULT_INT_VAL ;
clok_val_type   sclk = DEFAULT_INT_VAL ;
SemaphoreHandle_t serial_mutex;


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
  }

  #ifdef DEBUG_PRT_CLK
    //Serial.println(" Packet sent ");
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
        }

        #ifdef DEBUG_PRT_CLK
          Serial.print(" RCLK  : ");
          Serial.print(tmpRclk);
          Serial.print("           SCLK  : ");
          Serial.println(sclk);
        #endif
     
        if(tmpRclk)
        {
            xSemaphoreTake(serial_mutex, portMAX_DELAY); 
            sclk = ((sclk + tmpRclk) / 2) ;
            xSemaphoreGive(serial_mutex); 

            #ifdef DEBUG_ON_LED
              if(tmpRclk==sclk)
                digitalWrite(LED_BUILTIN,HIGH);
                Serial.print(" Synchronisation DONE ");
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
 *                                       T A S K s        F U N C T I O N S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
void task_inc(void *pvParameters) { 
    while(1) {
        //Serial.println("SCLK Incrementation Task"); 
        
        xSemaphoreTake(serial_mutex, portMAX_DELAY); 
        sclk++; 
        xSemaphoreGive(serial_mutex);
               
        vTaskDelay(25 / portTICK_PERIOD_MS); 
    } 
} 

void task_broadcast(void *pvParameters) { 
    while(1) { 
        //Serial.println("BROADCASTING Task");  

        broadcast_Sclk();
        
        vTaskDelay(1000/portTICK_PERIOD_MS); 
    } 
} 


void task_reception(void *pvParameters) { 
    while(1) { 
        //Serial.println("Reception Task");
        listen_Recieved_RCLK(); 
        vTaskDelay(30/ portTICK_PERIOD_MS); 
    } 
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

  serial_mutex = xSemaphoreCreateMutex(); 
  if (serial_mutex == NULL) { 
      Serial.println("Mutex can not be created"); 
  } 
  xTaskCreate(task_inc, "Task INC", 200, NULL, 1, NULL); 
  xTaskCreate(task_broadcast, "Task BROAD", 200, NULL, 1, NULL);
  xTaskCreate(task_reception, "Task RECEPTION", 200, NULL, 1, NULL);
  vTaskStartScheduler(); 
}


void loop() {
  }
