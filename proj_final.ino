// final project
// Student Name: Ronghuan You (your1)

// Air quality three levels:
// healthy level:     0 < ppm_value < 150
// moderate level: 150 <= ppm_value <= 700
// warning level:   700 < ppm_value 

#include "WiFiEsp.h"
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(10, 11);  // RX, TX
#endif

char ssid[] = "Algorithm2.4ghz"; // your network SSID (name)
char pass[] = "Fibonacci0.0";    // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
char server[] = "3.15.197.76";

// variables
char v1[] = "AirQualityValue:";
char ppm_val[4];                // ppm_val_3dig
char ppm_val_2dig[3];
char ppm_val_4dig[5];

char v2[] = "PredicationAirQuality:";
char predi_value[4];    
char predi_2dig[3];     
char predi_4dig[5];     

char message1 = "AirQualityValueOver9999!";
char unit1[] = "PPM";
unsigned long pre_time = 0;

#define RED_PIN 2
#define YELLOW_PIN 3
#define GREEN_PIN 4
#define diff 55 

boolean green = false;
boolean yellow = false;
boolean red = false;

char get_request[200];
// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  pinMode(A0, INPUT);                           //MQ135 intput
  pinMode(RED_PIN,OUTPUT);
  pinMode(YELLOW_PIN,OUTPUT);
  pinMode(GREEN_PIN,OUTPUT);

  Serial.begin(115200);                         // initialize serial for debugging
  Serial1.begin(115200);                        // initialize serial for ESP module
  WiFi.init(&Serial1);                          // initialize ESP module
  
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
  //  Serial.println("WiFi shield not present");
  //  don't continue
  while (true);
}

  // attempt to connect to WiFi network
while ( status != WL_CONNECTED) {
  //  Serial.print("Attempting to connect to WPA SSID: ");
  //  Serial.println(ssid);
  // Connect to WPA/WPA2 network
  status = WiFi.begin(ssid, pass);
}

//  Serial.println("You're connected to the network");
  printWifiStatus();
}


void loop()
{
  unsigned long timer = millis();  
  delay(1000);
  if (!client.connected()){
  //  Serial.println("Starting connection to server...");
  client.connect(server, 5000);
  }
  
  //  Serial.println("Connected to server");
  // ==========================================================================================
  // set up and calibration
  int current_ppm[10];                            // array to store ppm values with 10 size
  int original_ppm = 0;                         
  int calc_ppm = 0;                               //int for calculated ppm
  int sum_for_calib = 0;                          //int for averaging

  int arr_pre[100];
  int sum_pre = 0;
  
  for (int x = 0; x < 10; x++)                  
  {                   
    current_ppm[x] = analogRead(A0);
    delay(200);
  }

  for (int x = 0; x < 10; x++)                  //add samples together
  {                     
    sum_for_calib += current_ppm[x];  
  }
  
  original_ppm = sum_for_calib / 10;            //divide samples by 10
  calc_ppm = original_ppm - diff;               //get calculated ppm
  // ==========================================================================================
  // for prediction ppm_value
  for (int x = 0; x < 100; x++)                  //store calculated_value to the array
  {     
    arr_pre[x] = calc_ppm;          
  }

  for (int x = 0; x < 100; x++)                  // get sum for all samples
  {                     
    sum_pre += arr_pre[x];  
  }
  unsigned predi = sum_pre/100;

  // ==========================================================================================
  // part for LED lights
  if(calc_ppm <= 150)
  {
    digitalWrite(GREEN_PIN, HIGH);
    green = true;
    yellow = false;
    red = false;
  }
  
  if (green == true){
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(YELLOW_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
  }

  if( calc_ppm >= 150 and calc_ppm <= 700)
  {
    green = false;
    red = false;
    yellow = true;
  }
  
  if (yellow == true){
    digitalWrite(YELLOW_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
  }

  if( calc_ppm > 700)
  {
    red = true;
    green = false;
    yellow = false;
  }

  if (red == true){
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(YELLOW_PIN, LOW);
  }

  
  String ppm_value = String(calc_ppm);
  if (ppm_value.length() == 3){
    for (int i = 0; i < ppm_value.length(); i++)
    {
      ppm_val[i] = ppm_value.charAt(i);
    }
    ppm_val[4] = '\0';
  }
  else if (ppm_value.length() == 2){
    for (int i = 0; i < ppm_value.length(); i++)
    {
      ppm_val_2dig[i] = ppm_value.charAt(i);
    }
    ppm_val_2dig[3] = '\0';
  }
  else if (ppm_value.length() == 4){
    for (int i = 0; i < ppm_value.length(); i++)
    {
      ppm_val_4dig[i] = ppm_value.charAt(i);
    }
    ppm_val_4dig[5] = '\0';
  }
 
  // ==========================================================================================
  //  Make a HTTP request for prediction ppm value 
  // update prediction ppm value every hour
  if (timer - pre_time >= 3600000)
  {
    String str_predi = String(predi);
    if (str_predi.length() == 3){
      for (int i = 0; i < str_predi.length(); i++)
      {
        predi_value[i] = str_predi.charAt(i);
      }
      predi_value[4] = '\0';
    }
    else if(str_predi.length() == 2){
      for (int i = 0; i < str_predi.length(); i++)
      {
        predi_2dig[i] = str_predi.charAt(i);
      }
      predi_2dig[3] = '\0'; 
    }
    else if(str_predi.length() == 4){
      for (int i = 0; i < str_predi.length(); i++)
      {
        predi_4dig[i] = str_predi.charAt(i);
      }
      predi_4dig[5] = '\0';  
    }

    if (str_predi.length() == 2){
      sprintf(get_request,"GET /?var=%s%s%s HTTP/1.1\r\nHost: 0.0.0.0\r\nConnection: close\r\n\r\n", v2, predi_2dig, unit1);
    }
    else if (str_predi.length() == 3){
      sprintf(get_request,"GET /?var=%s%s%s HTTP/1.1\r\nHost: 0.0.0.0\r\nConnection: close\r\n\r\n", v2, predi_value, unit1);
    }
    else if (str_predi.length() == 4){
      sprintf(get_request,"GET /?var=%s%s%s HTTP/1.1\r\nHost: 0.0.0.0\r\nConnection: close\r\n\r\n", v2, predi_4dig, unit1);
    }else{
      sprintf(get_request,"GET /?var=%s HTTP/1.1\r\nHost: 0.0.0.0\r\nConnection: close\r\n\r\n", message1);
    }
    
    client.print(get_request);
    while (client.available())
    {
      char c = client.read();
      Serial.write(c);
    }
    pre_time = timer;
  }
  // ==========================================================================================
  //  Make a HTTP request
  if (ppm_value.length() == 2){
    sprintf(get_request,"GET /?var=%s%s%s HTTP/1.1\r\nHost: 0.0.0.0\r\nConnection: close\r\n\r\n", v1, ppm_val_2dig, unit1);
  }
  else if(ppm_value.length() == 3){
    sprintf(get_request,"GET /?var=%s%s%s HTTP/1.1\r\nHost: 0.0.0.0\r\nConnection: close\r\n\r\n", v1, ppm_val, unit1);
  }
  else if(ppm_value.length() == 4){
    sprintf(get_request,"GET /?var=%s%s%s HTTP/1.1\r\nHost: 0.0.0.0\r\nConnection: close\r\n\r\n", v1, ppm_val_4dig, unit1);
  }else{
      sprintf(get_request,"GET /?var=%s HTTP/1.1\r\nHost: 0.0.0.0\r\nConnection: close\r\n\r\n", message1);
  }
  client.print(get_request);
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
  // send data for graph
  Serial.println(ppm_value);
  //  delay(20000);
}
  // ==========================================================================================
void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // print the received signal strength
  long rssi = WiFi.RSSI();
  
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
