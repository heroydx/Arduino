/*
 *  参考代码：https://gist.github.com/tablatronix/497f3b299e0f212fc171ac5662fa7b42
 *  This sketch demonstrates how to scan WiFi networks. 
 *  The API is almost the same as with the WiFi Shield library, 
 *  the most obvious difference being the different file you need to include:
 */
#include "ESP8266WiFi.h"

void setup() {
  Serial.begin(115200);
//   Serial.setDebugOutput(true);
  
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);

  Serial.println("");
  Serial.println("Setup done");
  Serial.println("");
  Serial.println("MAC: " + WiFi.macAddress());
}

String encryptionTypeStr(uint8_t authmode) {
    switch(authmode) {
        case ENC_TYPE_NONE:
            return "OPEN";
        case ENC_TYPE_WEP:
            return "WEP";
        case ENC_TYPE_TKIP:
            return "WPA_PSK";
        case ENC_TYPE_CCMP:
            return "WPA2_PSK";
        case ENC_TYPE_AUTO:
            return "WPA_WPA2_PSK";
        default:
            return "UNKOWN";
    }
}

void loop() {
  //  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks(false,false);
  //  Serial.println("scan done");
  int o = n;
  int loops = 0;
  
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    // sort by RSSI
    int indices[n];
    int skip[n];
    
    String ssid;
    
    for (int i = 0; i < n; i++) {
      indices[i] = i;
    }

    // CONFIG
    bool sortRSSI   = true; // sort aps by RSSI
    bool removeDups = false; // remove dup aps ( forces sort )
    bool printAPs   = true; // print found aps
    
    bool printAPFound = false; // do home ap check   
    //const char* homeAP = "MYAP"; // check for this ap on each scan
    const char* homeAP = "BUPT_J2_1776"; // check for this ap on each scan
    // --------

    bool homeAPFound   = false;
    
    if(removeDups || sortRSSI){
      for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            loops++;
            //int temp = indices[j];
            //indices[j] = indices[i];
            //indices[i] = temp;
            std::swap(indices[i], indices[j]);  
            std::swap(skip[i], skip[j]);  
          }
        }
      }
    }

    if(removeDups){
      for (int i = 0; i < n; i++) {
        if(indices[i] == -1){
          --o;
          continue;
        }
        ssid = WiFi.SSID(indices[i]);
        for (int j = i + 1; j < n; j++) {
          loops++;
          if (ssid == WiFi.SSID(indices[j])) {
            indices[j] = -1;
          }
        }
      }
    }

//    Serial.println((String)loops);
    Serial.print(o);
    Serial.println(" networks found of " + (String)n);

    Serial.println("00: (RSSI)[BSSID][hidden] SSID [channel] [encryption]");
    for (int i = 0; i < n; ++i)
    {
      if(printAPFound && (WiFi.SSID(indices[i]) == homeAP)) homeAPFound = true;
    
      if(printAPs && indices[i] != -1){
      // Print SSID and RSSI for each network found
      Serial.printf("%02d", i + 1);
      Serial.print(":");

      Serial.print(" (");
      Serial.print(WiFi.RSSI(indices[i]));
      Serial.print(")");
      
      Serial.print(" [");
      Serial.print(WiFi.BSSIDstr(indices[i]));
      Serial.print("]");
      
      Serial.print(" [");
      Serial.print((String) WiFi.isHidden(indices[i]));
      Serial.print("]");

      Serial.print(" " + WiFi.SSID(indices[i]));
      // Serial.print((WiFi.encryptionType(indices[i]) == ENC_TYPE_NONE)?" ":"*");
      
      Serial.print(" [");
      Serial.printf("%02d",(int)WiFi.channel(indices[i]));
      Serial.print("]");

      Serial.print(" [");
      Serial.print((String) encryptionTypeStr(WiFi.encryptionType(indices[i])));
      Serial.print("]");

//      Serial.print(" WiFi index: " + (String)indices[i]);

      Serial.println();
      }
      delay(10);
    }
     if(printAPFound && !homeAPFound) Serial.println("HOME AP NOT FOUND");
     Serial.println("");
  }

  // Wait a bit before scanning again
  delay(500);
}
