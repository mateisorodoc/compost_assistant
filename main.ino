/*

Authors:

Sorodoc Matei - Sensor readings and Wi-Fi connectivity, Visual Web Interface and Web Server Communication
Ivas Catalin - Methods for normalizing data, Visual Oled Display
Elisei Marian - Data processing
Lutsch Antonia - Sensor research
Szecsi Antonia - Sensor research

*/



// Load Wi-Fi library
#include <ESP8266WiFi.h>

//Screen config
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels   
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Sensor input
#define TEMP_INPUT 13
#define HUM_INPUT 15
#define WAT_INPUT 14

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//function prototyping, documnetation for each one is written at the bottom of the fuule
int getTemperature();
int getHumidity();
int isWater();
int getDecision(int, int, int);
void drawStats();
void drawAction();

// Replace with your network credentials
const char* ssid     = "test";
const char* password = "test1234";

const int water_treshold = 80;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
 
  // Initialize variables
  pinMode(A0, INPUT);
  pinMode(TEMP_INPUT,OUTPUT);
  pinMode(HUM_INPUT, OUTPUT);
  pinMode(WAT_INPUT, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(1000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("CHECKING THE WIFI NETWORK...");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin(); 
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  /////////////TEMpUERATURE AND SENSORS
  display.clearDisplay();
  int temp = getTemperature();
  int hum = getHumidityPercent();
  int water = isWater();
  
  int dec = getDecision(hum, temp, water);
  display.setCursor(0, 0);
  if(dec == 1)
  {
    display.println("OK");
  }
  else
  {
    display.println("WARNING");
  }
    
  
  display.setCursor(0, 20);
  display.print("Temperature: ");
  display.print(temp);
  display.println("C");
  
  display.print("Humidity: ");
  display.print(hum);
  display.println("%");
  
  display.print("Water level:");
  if(water > water_treshold)
    display.println("Drain"); 
  else
    display.println("OK");

  display.display();
  //////////////
  
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
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
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");

            client.println("<script type=\"text/javascript\">setInterval(\'window.location.reload()\', 2000);</script>");
            
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">   <link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.7.2/css/all.css\" integrity=\"sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr\" crossorigin=\"anonymous\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;} .data {font-size:3.5em;} .info {font-size: 2em;} .info1 {font-size: 2.5em;}</style></head>");
                        
            // Web Page Heading
            client.println("<body><h1 style=\"font-size: 4em; margin-bottom: 40px;\">Compost assistant</h1>");
            
            // Display temperature
            String tempera(temp); 
            String temperatureRow = "<p class=\"data\"> <i class=\"fas fa-thermometer-half\" style=\"color:#059e8a;\"></i> Temperature: " +  tempera + "</p>";
            
            client.println(temperatureRow);
            int perc = getHumidityPercent();
            //Display humidity
            String humiPerc(perc);
            String humidityRow = "<p class=\"data\"> <i class=\"fas fa-tint\" style=\"color:#00add6;\"> </i> Humidity: "  + humiPerc + "%</p>";

            client.println(humidityRow);

            //Display water level
            String aqua="OK";
            if(water > water_treshold)
              aqua = "Drain";
       
            String waterRow = "<p class=\"data\"> <i class=\"fas fa-water\" style=\"color:#00add6;\"></i> Water level: " + aqua + "</p>";
      
            client.println(waterRow);
            client.println("<p class=\"info1\">To do: </p>");
            if(water > water_treshold)  
              client.println("<p class=\"info\">Too much water in the container. Drain it.</p>");
            if(temp < 45)
              client.println("<p class=\"info\">The temperature is too low, add more vegetable/fruit/food scraps.</p>");
            else if(temp > 72)
              client.println("<p class=\"info\">The temperature it too high, add some coffe grounds, shredded newspapers or bits of cardboard.</p>");

            if(hum < 40)
              client.println("<p class=\"info\">The humidity too low, water the compost</p>");

            if(dec == 1)
              client.println("<p>Your compost is in ideal condition.</p>");  
            else 
              client.println("<p class=\"info\">Mix the compost for aeration and temperature homogenization.</p>");

            
            
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
}


/*
 * draw Stats is the main function of the program where display is constructed
 */

/**************** SENSORS ***********************/
int getTemperature()
{
  /*
  digitalWrite(TEMP_INPUT, HIGH);
  delay(100);
  int temp = analogRead(A0) / 10;
  digitalWrite(TEMP_INPUT, LOW);
  return temp;
  */
      
  int Vo;
  float R1 = 10000;
  float logR2, R2, T, Tc, Tf;
  float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
  digitalWrite(TEMP_INPUT, HIGH);
  delay(1000);
  Vo = analogRead(A0);
  digitalWrite(TEMP_INPUT, LOW);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2));
  Tc = T - 273.15; //temperatura
  return Tc;
}


int getHumidity()
{
  digitalWrite(HUM_INPUT, HIGH);
  delay(100);
  int hum = analogRead(A0);
  digitalWrite(HUM_INPUT, LOW);
  return hum;
}

int getHumidityPercent()
{
  digitalWrite(HUM_INPUT, HIGH);
  delay(100);
  int hum = analogRead(A0);
  digitalWrite(HUM_INPUT, LOW);
  hum = (780-hum)/5;
  return hum;
}

//temporarly set to 0 due to lack of button on NodeMCU
int isWater()
{
  digitalWrite(WAT_INPUT, HIGH);  // turn the sensor ON
  delay(100);       
  // wait 10 milliseconds
  int water_value = analogRead(A0);
  digitalWrite(WAT_INPUT, LOW);


  water_value = water_value * 100/600;
  return water_value;
}
/*********************************************************/
/*Returns action cases based*/
int getDecision(int hum, int temp, int water)
{

  if(water > water_treshold)
    return 0;
  if(hum > 35 && hum < 65)
    return 0;
  if(temp < 45)
    return 0;
  if(temp > 72)  
    return 0;
    
   return 1;
}
