
#include <ESP8266WiFi.h>
#include <espnow.h>

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

#define   INTER_SCLK_DELAY  2000
#define   N                 1

#ifndef LED_BUILTIN
#define LED_BUILTIN 13
#endif

/************************************************************************************************************************************
 *                                   G L O B A L E      V A R I A B L E S     D E F I N I T I O N S 
 ************************************************************************************************************************************/

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xA4, 0xCF, 0x12, 0x97, 0x04, 0x40};

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    clok_val_type   sclk = DEFAULT_INT_VAL ;
} struct_message;


// Create a struct_message to hold incoming sensor readings
struct_message myData;

/************************************************************************************************************************************
 *                                             S U B F U N C T I O N S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
/**
 * @function  : 
 * @param     : the number of SCLKs to send 
 * @return    : return nothing.
 */
void N_SCLK_broadcast(int NBR) 
{ 
    for(int i=1; i<=NBR; i++)
    {
        myData.sclk=random(60000);
        // Send message via ESP-NOW
        esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
        Serial.println("==> Data sent "+myData.sclk);
        delay(INTER_SCLK_DELAY); 
    }
} 
/************************************************************************************************************************************
 *                               C A L L B A C K         F U N C T I O N S         D E F I N I T I O N S 
 ************************************************************************************************************************************/
/**
 * @function  : Callback when data is sent
 * @param     : no params 
 * @return    : return nothing.
 */
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) 
{
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

/**
 * @function  : Callback when data is received
 * @param     : no params 
 * @return    : return nothing.
 */
void OnDataRecv(uint8_t * mac, uint8_t *inData, uint8_t len) 
{
   struct_message incomingData;
  
  memcpy(&incomingData, inData, sizeof(incomingData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("SCLK : ");
  Serial.println(incomingData.sclk);
}
 
void setup() 
{
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
}

void loop() {
  N_SCLK_broadcast(N);
}
