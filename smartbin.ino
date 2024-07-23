#include <Ultrasonic.h>
#include <Servo.h>
#include <ESP8266WiFi.h>

#define TRIG_PIN_D7 D7
#define ECHO_PIN_D8 D8
#define TRIG_PIN_D3 D0  // Pin for the new sensor
#define ECHO_PIN_D4 D1  // Pin for the new sensor
#define SERVO_PIN D6

Ultrasonic ultrasonic1(TRIG_PIN_D7, ECHO_PIN_D8);  // Sensor for lid opening
Ultrasonic ultrasonic2(TRIG_PIN_D3, ECHO_PIN_D4);  // Sensor for bin level
Servo servo;

int lidOpenAngle = 180;
int lidClosedAngle = 0;
int maxDistance = 20;
int maxBinDistance = 5;  // Maximum distance for bin level monitoring

const char *ssid = "GITAM";
const char *password = "Gitam$$123"; 
const char *server = "api.thingspeak.com";
const String apiKey = "Enter ThingSpeak API Key";
const char *apiHost = "api.thingspeak.com";

void setup() {
  Serial.begin(115200);
  servo.attach(SERVO_PIN);
  servo.write(lidClosedAngle);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  int distanceLid = ultrasonic1.read();
  int distanceBin = ultrasonic2.read();

  if (distanceLid < maxDistance) {
    servo.write(lidOpenAngle);
    servo.write(lidOpenAngle);
    delay(500);
  } else {
    servo.write(lidClosedAngle);
  }
  
  Serial.print("Distance Lid: ");
  Serial.print(distanceLid);
  Serial.println(" cm");

  Serial.print("Distance Bin: ");
  Serial.print(distanceBin);
  Serial.println(" cm");

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    String url = "/update?api_key=" + apiKey +
               "&field1=" + String(distanceLid) +
               "&field2=" + String(distanceBin);

    if (client.connect(server, 80)) {
      client.println("GET " + url + " HTTP/1.1");
      client.println("Host: " + String(server));
      client.println("Connection: close");
      client.println();
      delay(10);
      client.stop();
      Serial.println("Data sent to ThingSpeak");
    }
  }

  if (distanceBin < maxBinDistance) {
    WiFiClient client;
    if (client.connect("maker.ifttt.com", 80)) {
      client.println("GET /trigger/bin_full/with/key/4v4_uYObr6iDkMkHm5MC- HTTP/1.1");
      client.println("Host: maker.ifttt.com");
      client.println("Connection: close");
      client.println();
      delay(10);
      client.stop();
      Serial.println("Bin Full Alert sent to IFTTT");
    }
  }

  delay(3000);
}