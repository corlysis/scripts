/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface. The device's I2C address is either 0x76 or 0x77.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

/*
 * Sensor connection:
 * BME280 GND --> NodeMCU GND
 * BME280 3.3V --> NodeMCU 3V3
 * BME280 SDA --> NodeMCU D2
 * BME280 SCL --> NodeMCU D1
 */


#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
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
const unsigned long delayTimeMs = 10000;


Adafruit_BME280 bme;
HTTPClient http;


void setup() {
    Serial.begin(9600);
    Serial.println("NodeMCU + BME280 + Corlysis");

    bool status;
    
    // you can also pass in a Wire library object like &Wire2
    status = bme.begin(sensorAddress);  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    
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
    float temperature = bme.readTemperature();
    //temperature = 9.0/5.0 * temperature + 32.0;
    float pressure = bme.readPressure();
    float humidity = bme.readHumidity();

    sendDataToCorlysis(temperature, pressure, humidity);
    
    delay(delayTimeMs);
}


void sendDataToCorlysis(float temperature, float pressure, float humidity) {
    static long counter = 0;
    
    char payloadStr[150];
    sprintf(payloadStr, "bme280_data temperature=%d.%02d,pressure=%d.%02d,humidity=%d.%02d", (int)temperature, (int)abs(temperature*100)%100, 
    (int)pressure, (int)abs(pressure*100)%100, (int)humidity, (int)abs(humidity*100)%100);
    Serial.println(payloadStr);
    
    char corlysisUrl[200];
    sprintf(corlysisUrl, "http://corlysis.com:8087/write?db=%s&u=token&p=%s", dbName, dbPassword);
    http.begin(corlysisUrl);
    //HTTPS variant - check ssh public key fingerprint
    //sprintf(corlysisUrl, "https://corlysis.com:8086/write?db=%s&u=token&p=%s", dbName, dbPassword);
    //http.begin(corlysisUrl, "FF:2D:E9:25:75:39:D1:A0:5C:99:02:34:EF:81:73:0F:3F:3E:2D:0D");

    
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
            Serial.println("WiFi: still not connected -> reboot.");
            WiFi.forceSleepBegin(); wdt_reset(); ESP.restart(); while(1)wdt_reset();
        }
        counter++;
        Serial.println("Data were not sent. Check network connection.");
    }
    Serial.println("");  
}
