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
const char* ssid = "";
const char* password = "";

// Sensor settings
const uint8_t sensor_address = 0x76;

// Corlysis Setting
const char* db_name = "";
const char* token_password = "";
const unsigned long delayTime = 5000;


Adafruit_BME280 bme;
HTTPClient http;


void setup() {
    Serial.begin(9600);
    Serial.println("NodeMCU + BME280 + Corlysis");

    bool status;
    
    // (you can also pass in a Wire library object like &Wire2)
    status = bme.begin(sensor_address);  
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    
    delay(100); // let sensor boot up

    //Wi-Fi connection
    Serial.print("Connecting to the: ");
    Serial.println(ssid);
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
    printValues(temperature, pressure, humidity);
    delay(delayTime);
}


void printValues(float temperature, float pressure, float humidity) {
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(pressure / 100.0F);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.println();
}

void sendDataToCorlysis(float temperature, float pressure, float humidity) {

    char payload_str[150];
    sprintf(payload_str, "bme280_data temperature=%d.%02d,pressure=%d.%02d,humidity=%d.%02d", (int)temperature, (int)(temperature*100)%100, 
    (int)pressure, (int)(pressure*100)%100, (int)humidity, (int)(humidity*100)%100);
    Serial.println(payload_str);
    
    char corlysis_url[200];
    sprintf(corlysis_url, "https://corlysis.com:8086/write?db=%s&u=token&p=%s", db_name, token_password);
    http.begin(corlysis_url, "C9:26:17:DA:C1:9E:BB:9F:3E:6E:56:89:E7:69:DA:9F:14:EA:54:DE");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(payload_str);
    Serial.print("http result:");
    Serial.println(httpCode);
    http.writeToStream(&Serial);
    http.end();

    if(httpCode == 204) {
        Serial.println("Data successfully sent.");      
    }else{
        Serial.println("Data were not sent. Check network connection.");  
    }
}

