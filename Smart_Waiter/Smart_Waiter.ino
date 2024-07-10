#include <Robojax_L298N_DC_motor.h>
#include <WiFi.h>
#include <HTTPClient.h>


//-----------------------------------MOTORS SETUP------------------------------------
// motor 1 settings
#define CHA 1
#define ENA 4 // this pin must be PWM enabled pin if Arduino board is used
#define IN1 16
#define IN2 17

// motor 2 settings
#define IN3 5
#define IN4 18
#define ENB 19// this pin must be PWM enabled pin if Arduino board is used
#define CHB 0

#define motorright 1 // do not change
#define motorleft 2 // do not change
#define speed 70

#define button 36

const int CCW = 1; // do not change
const int CW  = 2; // do not change

Robojax_L298N_DC_motor robot(IN1, IN2, ENA, CHA,  IN3, IN4, ENB, CHB);// initialise motors

//--------------------------------------MOTORS END--------------------------------------


//-----------------------------------IR SENSORS SETUP-----------------------------------
#define irSensorPin1  14
#define irSensorPin2  27  // Change this to the pin you've connected the sensor to
int sensorRight;
int sensorLeft;

//----------------------------------------IR END----------------------------------------

//------------------------------------ULTRASONIC SETUP----------------------------------
#define trigPin  15
#define echoPin  2
//-------------------------------------ULTRASONIC END-----------------------------------


//-----------------------------------COL0UR SENSOR SETUP--------------------------------
// Define color sensor pins
#define S0 23
#define S1 22
#define S2 32
#define S3 33
#define sensorOut 34

// Calibration Values
// Get these from Calibration Sketch

int redMin; // Red minimum value
int redMax; // Red maximum value
int greenMin; // Green minimum value
int greenMax; // Green maximum value
int blueMin; // Blue minimum value
int blueMax; // Blue maximum value

// Variables for Color Pulse Width Measurements

int redPW = 0;
int greenPW = 0;
int bluePW = 0;

// Variables for final Color values

int redValue;
int greenValue;
int blueValue;

//-----------------------------------COLOUR END---------------------------------------------

//-----------------------------------WIFI SETUP---------------------------------------------

const char* ssid = "realme 9 Pro+";
const char* password = "12345678";
const char* serverAddress = "192.168.70.35"; // Replace with your Express.js server's IP address
const int serverPort = 4000; // Replace with the port your Express.js server is listening on
const String url = "/"; // Replace with the specific URL path on your server

//-----------------------------------WIFI END-----------------------------------------------

//----------------------------------HOTEL SETUP---------------------------------------------

const int maxtable = 5;
String mode = "forward";
String junction;
String prev_junction;
char dat='s';

int orr=195,org=135,orb=80;
int br=5,bg=57,bb=122;
int rr=168,rg=55,rb=65;
int gr=64,gg=155,gb=115;
int wr=245,wg=245,wb=245;

const char path[maxtable][3] = 
{
  {'l','l','0'},
  {'l','r','0'},
  {'l','f','l'},
  {'l','f','r'},
  {'l','f','f'},
};
//----------------------------------HOTEL END-----------------------------------------------



void setup() 
{
  // Initialize serial communication
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");


  // Set the IR sensor pin as input
  pinMode(irSensorPin1, INPUT);
  pinMode(irSensorPin2, INPUT);
  pinMode(button,INPUT);

  // pin for front ultrasonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

    // Set S0 - S3 as outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  //
  // Set Sensor output as input
  pinMode(sensorOut, INPUT);
  
  // Set Frequency scaling to 20%
  digitalWrite(S0,HIGH);
  digitalWrite(S1,LOW);

  calibrate_white();
  calibrate_black();

  robot.begin();
//
}



void loop() 
{
  junction = "start";
  prev_junction = "start";
  int table_no=Get_tableNumber_from_server();
  Serial.println("from loop : ");
  Serial.println(table_no);
  dat='g';
  delay(2000);
  /*/
  int table_no=1;
  while(1)
  {
    if (Serial.available())
    {
      int temp =Serial.parseInt(); 
      if (temp>0 && temp<=4)
      {
        table_no=temp;
        Serial.print("table no : ");
        Serial.print(table_no);
        break;
      }     
    }
  }
/*/



  while(1)
  {

    Serial.println("inside running loop");
    junction = detect_junction(prev_junction);
    move();
    Serial.println("junction is  : ");
    Serial.print(junction);
//
    if(junction == "kitchen" && prev_junction != junction)
    {
      Serial.println("ENTRED KITCHEN");
      prev_junction = junction;
      stop();
//
      if (mode == "forward")
      {
        while(digitalRead(button) == 0)
        {
          Serial.println("waiting for food to be placed");
        }
        delay(2000);
      }  
// 
     

      decide_turn(path[table_no-1][0],mode);
    }

    else if(junction == "J1" && prev_junction != junction)
    {
      Serial.println("ENTRED J1");      
      prev_junction = junction;
      stop();
      decide_turn(path[table_no-1][1],mode);
    }

    else if(junction == "J2" && prev_junction != junction)
    {
      Serial.println("ENTRED J2");
      prev_junction = junction;
      stop();
      decide_turn(path[table_no-1][2],mode);
    }

    else if(junction == "table" && prev_junction != junction)
    {
      Serial.println("ENTRED TABLE");
      prev_junction = junction;
      stop();
//
      while(digitalRead(button) == 1)
      {
        Serial.println("waiting for food to be taken");
      }
//     
      delay(2000);
      decide_turn('r',mode);
      mode = "reverse";
    }

    else if(junction == "start" && prev_junction != junction)
    {
      prev_junction = junction;
      Serial.println("ENTRED START");
      stop();
      decide_turn('r',mode);
      dat='s';
      mode = "forward";
      break;
    }
    else
      continue;
//        
  }

}


//*************************************** FUNCTIONS ***********************************

void stop()
{
  robot.brake(motorright);
  robot.brake(motorleft);
  delay(500);
}


void move()
{
  //
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the echo pulse
  long duration = pulseIn(echoPin, HIGH);

  // Convert the duration into distance (in centimeters)
  float distance_cm = duration * 0.034 / 2;

  // 
  sensorRight = digitalRead(irSensorPin1);
  sensorLeft = digitalRead(irSensorPin2);
  
//
 if (Serial.available()){
     dat=Serial.read();
     Serial.println(dat);
  }
//
  while(distance_cm <=30)
  {
    robot.brake(motorright);
    robot.brake(motorleft);
    Serial.println(distance_cm);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send a 10 microsecond pulse to trigger the sensor
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the echo pulse
  duration = pulseIn(echoPin, HIGH);

  // Convert the duration into distance (in centimeters)
  distance_cm = duration * 0.034 / 2;
  }
//
  while ((sensorRight == HIGH && sensorLeft== HIGH) || dat == 's')
  {
    if (Serial.available()){
    dat=Serial.read();
    Serial.println(dat);
    break;

    }
    //Serial.println("motors off: lines detected or dat = s");
    robot.brake(motorright);
    robot.brake(motorleft);

  }

  if (sensorRight == LOW && sensorLeft== LOW )
  {
    Serial.println("motors on : no line detected");
    robot.rotate(motorright,speed,CW); 
    robot.rotate(motorleft,speed,CW); 
  }
//
//  
  if (sensorRight == HIGH) 
  {
      Serial.println("sensor Right :  Line detected");
      robot.rotate(motorright,speed,CCW);// turn motor1 with i% speed in CW direction (whatever is i)
      robot.rotate(motorleft,speed,CW); 
  }
  else
   { //Serial.println("sensor Right: no line detected");
   }
//

  if (sensorLeft == HIGH) 
  {
      Serial.println("sensor Left :  Line detected");
      robot.rotate(motorleft,speed,CCW);// turn motor1 with i% speed in CW direction (whatever is i)
      robot.rotate(motorright,speed,CW); 
  }
    else
   { //Serial.println("sensor Left: no line detected");
   }
}


void decide_turn(char data,String mode)
{
  if (mode == "reverse" )
  {
    if(data == 'l')
    {  data='r';}
    else if(data =='r')
    {  data='l';}
  }

  if (data == 'l')
  {
    sensorRight = digitalRead(irSensorPin1);
    robot.rotate(motorleft,speed+20,CCW);// turn motor1 with i% speed in CW direction (whatever is i)
    robot.rotate(motorright,speed+20,CW);

    while(sensorRight == LOW)
    {
      sensorRight = digitalRead(irSensorPin1);
      Serial.println("inside if NO line");
    }

    while(sensorRight == HIGH)
    {
      sensorRight = digitalRead(irSensorPin1);
      Serial.println("inside if  line");
    }

    while(sensorRight == LOW)
    {
      sensorRight = digitalRead(irSensorPin1);
      Serial.println("FINAL NO line");
    }
    
    robot.brake(motorright);
    robot.brake(motorleft);
    
  }
  else if (data == 'r')
  {
    sensorLeft = digitalRead(irSensorPin2);
    robot.rotate(motorleft,speed+20,CW);// turn motor1 with i% speed in CW direction (whatever is i)
    robot.rotate(motorright,speed+20,CCW);

    while(sensorLeft == LOW)
    {
      sensorLeft = digitalRead(irSensorPin2);
      Serial.println("inside if NO line");
    }

    while(sensorLeft == HIGH)
    {
      sensorLeft = digitalRead(irSensorPin2);
      Serial.println("inside if  line");
    }

    while(sensorLeft == LOW)
    {
      sensorLeft = digitalRead(irSensorPin2);
      Serial.println("FINAL NO line");
    }
    
    robot.brake(motorright);
    robot.brake(motorleft);

  }
  else if (data == 'f')
  {

  }
}

String detect_junction (String before_junction)
{
  // Read Red value
  redPW = getRedPW();
  redValue = map(redPW, redMin,redMax,255,0);

  // Read Green value
  greenPW = getGreenPW();
  greenValue = map(greenPW, greenMin,greenMax,255,0);

  // Read Blue value
  bluePW = getBluePW();
  blueValue = map(bluePW, blueMin,blueMax,255,0);
  
  /*/ Print output to Serial Monitor
  Serial.print("Red = ");
  Serial.print(redValue);
  Serial.print(" - Green = ");
  Serial.print(greenValue);
  Serial.print(" - Blue = ");
  Serial.println(blueValue);
/*/
  if (redValue == constrain(redValue,orr-20,orr+20) && greenValue == constrain(greenValue,org-20,org+20) && blueValue == constrain(blueValue,orb-20,orb+20) )
  {
    Serial.println("ORANGE COLOUR DETECTED");
    return "J2";
  }

  else if (redValue == constrain(redValue,br-20,br+20) && greenValue == constrain(greenValue,bg-20,bg+20) && blueValue == constrain(blueValue,bb-20,bb+20) )
  {
    Serial.println("BLUE COLOUR DETECTED");
    return "J1";
  }

  else if (redValue == constrain(redValue,rr-20,rr+20) && greenValue == constrain(greenValue,rg-20,rg+20) && blueValue == constrain(blueValue,rb-20,rb+20) )
  {
    Serial.println("RED COLOUR DETECTED");
    return "kitchen";
  }

  else if (redValue == constrain(redValue,wr-20,wr+20) && greenValue == constrain(greenValue,wg-20,wg+20) && blueValue == constrain(blueValue,wb-20,wb+20) )
  {
    Serial.println("WHITE COLOUR DETECTED");
    return "start";
  }

  else if (redValue == constrain(redValue,gr-20,gr+20) && greenValue == constrain(greenValue,gg-20,gg+20) && blueValue == constrain(blueValue,gb-20,gb+20) )
  {
    Serial.println("GREEN COLOUR DETECTED");
    return "table";
  }
  else
  {
    Serial.println(before_junction);
    return before_junction;
  }
}

// Function to read Red Pulse Widths
int getRedPW() {

  // Set sensor to read Red only
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  delay(10);
  // Define integer to represent Pulse Width
  int PW;
  // Read the output Pulse Width
  PW = pulseIn(sensorOut, LOW);
  // Return the value
  return PW;

}

// Function to read Green Pulse Widths
int getGreenPW() {

  // Set sensor to read Green only
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  delay(10);
  // Define integer to represent Pulse Width
  int PW;
  // Read the output Pulse Width
  PW = pulseIn(sensorOut, LOW);
  // Return the value
  return PW;

}

// Function to read Blue Pulse Widths
int getBluePW() {

  // Set sensor to read Blue only
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  delay(10);
  // Define integer to represent Pulse Width
  int PW;
  // Read the output Pulse Width
  PW = pulseIn(sensorOut, LOW);
  // Return the value
  return PW;

}

int Get_tableNumber_from_server() 
{
  while(1)
  {
    if (WiFi.status() == WL_CONNECTED) {
      // Create a WiFiClient object
      WiFiClient client;

      // Create a HTTPClient object
      HTTPClient http;

      // Construct the full URL
      String fullUrl = "http://" + String(serverAddress) + ":" + String(serverPort) + url;

      // Begin the HTTP request
      Serial.print("Sending GET request to: ");
      Serial.println(fullUrl);

      if (http.begin(client, fullUrl)) {
        // Send the GET request
        int httpResponseCode = http.GET();

        // Check if the request was successful
        if (httpResponseCode > 0) {
          // Get the response payload
          String response = http.getString();
         // Serial.println("DEBUGGING");
          Serial.println("Response:");
          Serial.println(response);
          int tablenumber = response.toInt();
         // Serial.println("after convering to int");
          Serial.println(tablenumber);
          if (tablenumber != 0 && tablenumber <= maxtable)
          {
           // Serial.println("inside returning if");
            return tablenumber;// return the table number and end the function
          }
        } 
        else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
        }

        // End the HTTP connection
        http.end();
      } 
      else {
        Serial.println("Unable to connect to server.");
      }
    } 
    else {
      Serial.println("WiFi not connected.");
    }

    // Wait before making the next request
    delay(3000); // 3 seconds
  }
}

void calibrate_white()
{
  Serial.println("PRESS G TO CALIBRATE WITH WHITE : ");
  while(1)
  {
    if (Serial.available())
    {
      char dat=Serial.read();
      if(dat=='g')
      {
        Serial.println("calibrating with white.....");

        redMin = getRedPW();
        greenMin = getGreenPW();
        blueMin = getBluePW();
        unsigned long startTime = millis();

        while (millis() - startTime < 7000) 
        {
          redPW = getRedPW();
          greenPW = getGreenPW();
          bluePW = getBluePW();
          redMin=min(redMin,redPW);
          greenMin=min(greenMin,greenPW);
          blueMin=min(blueMin,bluePW);
        }

        Serial.println("white calibrated successfully");
        break;
      }
    }
  }
}

void calibrate_black()
{
  Serial.println("PRESS G TO CALIBRATE WITH BLACK : ");
  while(1)
  {
    if (Serial.available())
    {
      char dat=Serial.read();
      if(dat=='g')
      {
        Serial.println("calibrating with black.....");

        redMax = getRedPW();
        greenMax = getGreenPW();
        blueMax = getBluePW();
        unsigned long startTime = millis();

        while (millis() - startTime < 7000) 
        {
          redPW = getRedPW();
          greenPW = getGreenPW();
          bluePW = getBluePW();
          redMax=max(redMax,redPW);
          greenMax=max(greenMax,greenPW);
          blueMax=max(blueMax,bluePW);
        }
        
        Serial.println("black calibrated successfully");
        break;
      }
    }
  }
}




