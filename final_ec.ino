// Load Wi-Fi library
#include <WiFi.h>
#include <ESP32Servo.h> // load esp32 servo libarary

//Servo Config
Servo doorServo;

//define all pins for water pump
#define TRIG_PIN 23 // ESP32 in GPIO23 connected to Ultrasonic Sensor's TRIG pin
#define ECHO_PIN 22 // ESP32 pin GPIO22 connected to Ultrasonic Sensor's ECHO pin
float duration,distance_cm;
#define MOTOR_PIN 18
String motorState = "off";

// Replace with your network credentials
const char* ssid     = "SmartHome";
const char* password = "12345678";

// Set web server port number to 80
WiFiServer server(80);

String header;
const int output26 = 26;  //Servo Motor   

const int output27 = 25; // LEd

String output26State = "off";
String output27State = "off";

void setup() {
  Serial.begin(115200);

  //for Servo slide door
  doorServo.attach(output26);

  // Initialize the output variables as outputs
  // pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // Set outputs to LOW
  // digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
  doorServo.write(45);

  //ultrasonic sensor
  pinMode(TRIG_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);
  //MOTOR 
  pinMode(MOTOR_PIN,OUTPUT);



}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              doorServo.write(135);
            
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              doorServo.write(30);
              Serial.println("Servo -> 0");
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
            }else if (header.indexOf("GET /25/on") >= 0){
              Serial.println("Motor activated");
              motorState = "on";
              digitalWrite(MOTOR_PIN,HIGH);

            }else if (header.indexOf("GET /25/off") >=0 ){
              Serial.println("Motor Stop");
              motorState = "off";
              digitalWrite(MOTOR_PIN,LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><center><h1>SmartHome Control Pannel</h1></center><hr>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p>Slide Door " + output26State + "</p>");
            // If the output26State is off, it displays the ON button       
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>Electronic Bulb " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            
            client.println("<p>Water pump " + motorState + "</p>");
            // If the output27State is off, it displays the ON button       

            if (motorState=="off") {
              client.println("<p><a href=\"/25/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/25/off\"><button class=\"button button2\">OFF</button></a></p>");
            }



            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");


  }

  //water pump function 
  // generate 10-microsecond pulse to TRIG pin
  digitalWrite(TRIG_PIN,LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN,HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN,LOW);

  duration = pulseIn(ECHO_PIN,HIGH);
  distance_cm = duration / 58.2;

  Serial.print("distance  : ");
  Serial.println(distance_cm);
  delay(1000);



  if (distance_cm <= 6.0)
  {
    Serial.println("close");
    digitalWrite(MOTOR_PIN,LOW);
  }
  else{
    Serial.println("open");
  }
  delay(500);
}