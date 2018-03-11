//WiFi
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WiFiManager.h"
#include <ESP8266mDNS.h>

//MQTT
#include <PubSubClient.h>

//NTP
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 0, 60000);

//const char* mqtt_server = "192.168.43.110";
const char *mqtt_server = "m12.cloudmqtt.com";
const int mqtt_port = 10745;
const char *mqtt_user = "igutgmoo";
const char *mqtt_pass = "ZRSNzyn0bG6e";
const char *mqtt_client_name = "esp_d1_mini"; // Client connections cant have the same connection name


WiFiClient espClient;

PubSubClient mqttclient(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

/* Weather Logic */

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

//open weather map api key
String apiKey = "e107d5d4f634aff19140e50271c39380";

//the city you want the weather for
String location = "Panaji, IN";

char weather_server[] = "api.openweathermap.org";

/****************************** */


/**************************/

#include <Ticker.h>  //Ticker Library

#include "DHT.h"

#define DHTPIN D1     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

Ticker dht21reader;

/**************************/

ESP8266WebServer server ( 80 );

void handleRoot();
void handleNotFound();


void handleRoot() {
  char temp[500];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  char *html = R"##(<html>
  <head>
    <meta charset='utf-8'/>
    <meta http-equiv='refresh' content='5'/>
    <title>ESP8266 Demo</title>
    <style>
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }
    </style>
  </head>
  <body>
    <h1>Hello from ESP8266!</h1>
    <p><a href="/inline"> Inline </a></p>
    <p>Uptime: %02d:%02d:%02d</p>
    </body>
  </html>
  )##";

  snprintf ( temp, 500, html,
             hr, min % 60, sec % 60
           );
  server.send ( 200, "text/html", temp );
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

/**************************************************************************/

/************************* MQTT Setup *********************************/

void setup_wifi()
{

  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  wifiManager.setAPCallback(configModeCallback);

  if (!wifiManager.autoConnect("Esp8266APP")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected... :)");

  // Start the server
  server.on ( "/", handleRoot );
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );

  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println ( "HTTP server started" );

  // Print the IP address
  Serial.println(WiFi.localIP());

}

void callback(char* topic, byte* payload, unsigned int length) {

  char *city = NULL;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  city = (char*) malloc(length + 1);
  {
    int i;
    for (i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
      city[i] = (char)payload[i];
    }
    city[i] = '\0';
  }


  location = (char*) city;

  Serial.println(location);

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

  readdht21();
  getWeather();
  //delay(5000);


  Serial.print("Publish message: ");

  mqttclient.publish("outTopic", "Done");

}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    mqttclient.disconnect();
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttclient.connect(mqtt_client_name, mqtt_user, mqtt_pass)) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttclient.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttclient.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void printDiffString(String now, String later, String weatherType) {
  int indexNow = now.indexOf(weatherType);
  int indexLater = later.indexOf(weatherType);
  // for all types of weather except for clear skies, if the current weather does not contain the weather type and the later message does, send notification
  if (weatherType != "clear") {
    if (indexNow == -1 && indexLater != -1) {
      Serial.println("Oh no! It is going to " + weatherType + " later! Predicted " + later);
    }
  }
  // for clear skies, if the current weather does not contain the word clear and the later message does, send notification that it will be sunny later
  else {
    if (indexNow == -1 && indexLater != -1) {
      Serial.println("It is going to be sunny later! Predicted " + later);

    }
  }
}

void printDiffFloat(float now, float later, String parameter, String unit) {
  String change;
  if (now > later) {
    change = "drop from ";
  }
  else if (now < later) {
    change = "rise from ";
  }
  else {
    return;
  }
  Serial.print("UPDATE: The " + parameter + " will " + change);
  Serial.print(now);
  Serial.print(unit + " to ");
  Serial.print(later);
  Serial.println(unit + "!");
}


void getWeather() {

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  if (espClient.connect(weather_server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    espClient.print("GET /data/2.5/forecast?");
    espClient.print("q=" + location);
    espClient.print("&appid=" + apiKey);
    espClient.print("&cnt=3");
    espClient.println("&units=metric");
    espClient.println("Host: api.openweathermap.org");
    espClient.println("Connection: close");
    espClient.println();
  } else {
    Serial.println("unable to connect");
  }

  delay(1000);
  String line = "";

  while (espClient.connected()) {
    line = espClient.readStringUntil('\n');
    display_logic(line);
    //Serial.println(line);
/*    
    Serial.println("parsingValues");

    DynamicJsonBuffer  jsonBuffer(2000);

    JsonObject& root = jsonBuffer.parseObject(line);

    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
    }

    JsonArray& list = root["list"];
    JsonObject& now = list[0];
    JsonObject& later = list[1];

    String city = root["city"]["name"];
    float tempNow = now["main"]["temp"];
    float humidityNow = now["main"]["humidity"];
    String weatherNow = now["weather"][0]["description"];

    float tempLater = later["main"]["temp"];
    float humidityLater = later["main"]["humidity"];
    String weatherLater = later["weather"][0]["description"];

    Serial.println(city);
    Serial.println(tempNow);
    Serial.println(humidityNow);
    Serial.println(weatherNow);

    Serial.println(tempLater);
    Serial.println(humidityLater);
    Serial.println(weatherLater);

    printDiffFloat(tempNow, tempLater, "temperature", "*C");
    printDiffString(weatherNow, weatherLater, "rain");
    printDiffString(weatherNow, weatherLater, "snow");
    printDiffString(weatherNow, weatherLater, "hail");
    printDiffString(weatherNow, weatherLater, "clear");
    printDiffFloat(humidityNow, humidityLater, "humidity", "%");
    Serial.println();
    */
  }
}


void setup_mqttclient()
{
  mqttclient.setServer(mqtt_server, mqtt_port);
  mqttclient.setCallback(callback);
}


void readdht21() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
}


void setup() {

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(9600);
  setup_wifi();
  display_setup();
  Serial.println("Abhijeet is here");
  setup_mqttclient();
  timeClient.begin();
  //dht.begin();
  //dht21reader.attach(60*60, readdht21);
  //delay(2000);
  //display_logic();
  //delay(2000);
}

void loop() {
  server.handleClient();
  yield();

  if (!mqttclient.connected()) {
    reconnect();
  }
  mqttclient.loop();
  timeClient.update();
  if(timeClient.getSeconds() == 0) {
    Serial.print(timeClient.getHours());
    Serial.print(":");
    Serial.println(timeClient.getMinutes());
  }

  /*
    long now = millis();
    if (now - lastMsg > 2000) {
      lastMsg = now;
      ++value;
      snprintf (msg, 75, "hello world #%ld", value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("outTopic", msg);
    }
  */
}
