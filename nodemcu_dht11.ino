#include <Wire.h>

#include "DHTesp.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Wi-Fi settings
const char* ssid = "YOUR-SSID";
const char* password = "YOUR-PASSWORD";

// Sensor settings
const uint8_t sensor_address = 0x76;

// Corlysis Setting - click to the database to get those info
const char* db_name = "YOUR-DB_NAME";
const char* db_password = "YOUR-DB-PASSWORD";
const unsigned long delayTimeMs = 20000;


DHTesp dht;
HTTPClient http;


void setup() {
    Serial.begin(9600);
    Serial.println("NodeMCU + DHT11 + Corlysis");

    bool status;
    
    dht.setup(2); // data pin 2 - D4
    
    delay(100); // let sensor boot up

    //Wi-Fi connection
    Serial.print("Connecting to the: ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected.");
    Serial.print("My IP address: ");
    Serial.println(WiFi.localIP());
    
}


void loop() {
    // read data from sensor
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();
    
    sendDataToCorlysis(temperature, humidity);
    
    delay(delayTimeMs);
}


void sendDataToCorlysis(float temperature, float humidity) {
    static long counter = 0;
    
    char payloadStr[150];
    sprintf(payloadStr, "dht11_data temperature=%d.%02d,humidity=%d.%02d", (int)temperature, (int)abs(temperature*100)%100, 
    (int)humidity, (int)abs(humidity*100)%100);
    Serial.println(payloadStr);
    
    char corlysisUrl[200];
    sprintf(corlysisUrl, "http://corlysis.com:8087/write?db=%s&u=token&p=%s", dbName, dbPassword);
    http.begin(corlysisUrl);
    //HTTPS variant - check ssh public key fingerprint
    //sprintf(corlysisUrl, "https://corlysis.com:8086/write?db=%s&u=token&p=%s", dbName, dbPassword);
    //http.begin(corlysisUrl, "92:23:13:0D:59:68:58:83:E6:82:98:EB:18:D7:68:B5:C8:90:0D:03");
    
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(payloadStr);
    Serial.print("http result:");
    Serial.println(httpCode);
    http.writeToStream(&Serial);
    http.end();

    if(httpCode == 204) {
        counter = 0;
        Serial.println("Data successfully sent.");
    }else{
        if(counter > 10 && httpCode == -1) {
            Serial.println("WiFi: still not connected -> going into deep sleep for 10 seconds.");
            ESP.deepSleep(10e6);
         }
        counter++;
        Serial.println("Data were not sent. Check network connection.");
    }
    Serial.println("");  
}
