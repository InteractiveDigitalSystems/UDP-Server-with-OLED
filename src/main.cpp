#include <Arduino.h>
#include "WiFi.h"
#include "AsyncUDP.h"

#include <U8g2lib.h>
#include <Wire.h>

const char* ssid = "lego-robot";
const char* pass = "lego-2016";
AsyncUDP udp;
const int rele = 23;

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

uint8_t outputByteArray[128];
String outputString;


String converter(uint8_t* input)
{
  return String((char* )input);
}


void sendMessage(String msg){
  udp.writeTo((const uint8_t*) msg.c_str(), msg.length(),
              IPAddress(192, 168, 1, 3), 4001);
}



void setup() {

  //init OLED
  u8g2.begin();    //init
  u8g2.enableUTF8Print();        // Enable UTF8 support for Arduino print（）function.


  //init Wifi
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed");
    while (1) {
      delay(1000);
    }
  }
  if (udp.listen(4000)) {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    udp.onPacket([](AsyncUDPPacket packet) {
      Serial.print("UDP Packet Type: ");
      Serial.print(packet.isBroadcast()
                       ? "Broadcast"
                       : packet.isMulticast() ? "Multicast" : "Unicast");
      Serial.print(", From: ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.print(packet.remotePort());
      Serial.print(", To: ");
      Serial.print(packet.localIP());
      Serial.print(":");
      Serial.print(packet.localPort());
      Serial.print(", Length: ");
      Serial.print(packet.length());
      Serial.print(", Data: ");
      Serial.write(packet.data(), packet.length());
      Serial.println();

      
      outputString = String((char* )packet.data()); // direct convertion
      //outputString = converter(packet.data());   // using the "converter" function
      

      //in depth way below
      memset(outputByteArray, 0x00, sizeof(outputByteArray)); // clearing the memory
      memcpy(outputByteArray, packet.data() , packet.length()); //copying across memory
      outputByteArray[packet.length()] = 0x00; //terminating memory string, extra safe
      
      // reply to the client/sender
      packet.printf("Got %u bytes of data", packet.length());
    });
  }
  // Send unicast
  // udp.print("Hello Server!");
  // udp.
}


void loop() {
  //delay(5000);

  // Send broadcast on port 4000
  //udp.broadcastTo("Anyone here?", 4001);

  // Serial.println("waiting for udp message...");
  int x = 100;
  int y = 100;
  sendMessage("init " + String(x) + " " + String(y));

  //OLED
  u8g2.setFont(u8g2_font_maniac_tf); 
  u8g2.setFontDirection(0);  
  u8g2.firstPage();
  do {
    u8g2.setCursor(/* x=*/0, /* y=*/30);    //Define the cursor of print function, any output of the print function will start at this position.
    u8g2.print(outputString);
    //u8g2.write(outputByteArray, sizeof(outputByteArray));
    
  } while ( u8g2.nextPage() );
  delay(500);
}