#include <SoftwareSerial.h>

 #include <Servo.h>  
#include <LiquidCrystal.h>

// Pin Definitions
const int rs = 13, en = 12, d4 = 11, d5 = 10, d6 = 9, d7 = 8;
const int m1 = 7, m2 = 6, m3 = 5, m4 = 4;   
const int echo = A4, trig = A5;
const int buzzer = A1;
const int servoPin = 3;

// Global Variables
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
Servo myservo;  
String data;
const float SAFE_DISTANCE = 20.0;  // Safe distance in cm
const float CRITICAL_DISTANCE = 10.0;  // Critical proximity distance
unsigned long lastMoveTime = 0;
const unsigned long MOVE_TIMEOUT = 5000;  // 5 seconds max movement time

// Enhanced Obstacle Detection and Avoidance
float measureDistance() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  float duration = pulseIn(echo, HIGH);
  return duration / 58.2;  // Convert to cm
}

void signalAlert(float distance) {
  lcd.clear();
  lcd.print("Dist: ");
  lcd.print(distance);
  lcd.print(" cm");
  lcd.setCursor(0, 1);
  
  // Different alerts based on proximity
  if (distance < CRITICAL_DISTANCE) {
    lcd.print("CRITICAL STOP!");
    digitalWrite(buzzer, HIGH);
    stop1();
    delay(500);
    digitalWrite(buzzer, LOW);
  } else {
    lcd.print("OBSTACLE AHEAD!");
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
  }
}

void obstacleAvoidance() {
  // Scan for alternative paths
  myservo.write(0);   // Center
  delay(500);
  float centerDistance = measureDistance();
  
  myservo.write(45);  // Right scan
  delay(500);
  float rightDistance = measureDistance();
  
  myservo.write(135); // Left scan
  delay(500);
  float leftDistance = measureDistance();
  
  myservo.write(90);  // Servo middle position
  
  // Decision making for path selection
  if (centerDistance > SAFE_DISTANCE) {
    front();  // Primary path clear
  } else if (rightDistance > leftDistance) {
    right();  // More space on right
  } else {
    left();   // More space on left
  }
}

void setup() {
  Serial.begin(9600);
  
  // LCD Initialization
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("OMNIDIRECTIONAL");
  lcd.setCursor(0, 1);
  lcd.print("PATHFINDING ROBOT");
  delay(1500);
  
  // Motor Pin Setup
  pinMode(m1, OUTPUT);
  pinMode(m2, OUTPUT);
  pinMode(m3, OUTPUT);
  pinMode(m4, OUTPUT);
  
  // Ultrasonic Sensor Pins
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  
  // Buzzer and Servo Setup
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, LOW);
  
  myservo.attach(servoPin);  
  myservo.write(90);  // Center position
  
  // Initial Motor State
  stop1();
  
  // Startup Indication
  digitalWrite(buzzer, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
}

void front() {
  Serial.println("MOVING FORWARD");
  lcd.clear();
  lcd.print("FORWARD");
  
  digitalWrite(m1, HIGH);
  digitalWrite(m2, LOW);
  digitalWrite(m3, HIGH);
  digitalWrite(m4, LOW);
  
  lastMoveTime = millis();
}

void back() {
  Serial.println("MOVING BACKWARD");
  lcd.clear();
  lcd.print("BACKWARD");
  
  digitalWrite(m1, LOW);
  digitalWrite(m2, HIGH);
  digitalWrite(m3, LOW);
  digitalWrite(m4, HIGH);
  
  lastMoveTime = millis();
}

void left() {
  Serial.println("TURNING LEFT");
  lcd.clear();
  lcd.print("LEFT TURN");
  
  digitalWrite(m1, HIGH);
  digitalWrite(m2, LOW);
  digitalWrite(m3, LOW);
  digitalWrite(m4, HIGH);
  
  lastMoveTime = millis();
}

void right() {
  Serial.println("TURNING RIGHT");
  lcd.clear();
  lcd.print("RIGHT TURN");
  
  digitalWrite(m1, LOW);
  digitalWrite(m2, HIGH);
  digitalWrite(m3, HIGH);
  digitalWrite(m4, LOW);
  
  lastMoveTime = millis();
}

void stop1() {
  Serial.println("STOPPED");
  lcd.clear();
  lcd.print("STOPPED");
  
  digitalWrite(m1, LOW);
  digitalWrite(m2, LOW);
  digitalWrite(m3, LOW);
  digitalWrite(m4, LOW);
}

void loop() {
  // Check for timeout to prevent prolonged movement
  if (millis() - lastMoveTime > MOVE_TIMEOUT) {
    stop1();
  }
  
  // Obstacle Detection
  float distance = measureDistance();
  
  if (distance < CRITICAL_DISTANCE) {
    signalAlert(distance);
    obstacleAvoidance();
  }
  
  // Serial Command Processing
  while (Serial.available()) {
    data = Serial.readString();
    delay(100);
    
    lcd.clear();
    lcd.print("CMD: " + data);
    
    // Command Processing
    switch(data[0]) {
      case 'f': front(); break;
      case 'b': back(); break;
      case 'l': left(); break;
      case 'r': right(); break;
      case 's': stop1(); break;
      default: 
        Serial.println("Invalid Command");
        lcd.clear();
        lcd.print("INVALID CMD");
        break;
    }
  }
  
  delay(100);  // Small delay for stability
}
