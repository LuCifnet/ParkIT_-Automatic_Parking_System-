#include <Servo.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

#define trigPin1 10
#define echoPin1 10
#define trigPin2 9
#define echoPin2 9
#define trigPin3 8
#define echoPin3 8
#define trigPin4 7
#define echoPin4 7
#define trigPin5 13
#define echoPin5 13

Servo myservo;

int distanceThreshold = 100;      // Distance threshold in cm for parking slots
int parkingAvailable = 3;         // Initial available parking spots
int barrierState = 0;             // Barrier state: 0=closed, -1=entry, 1=exit
unsigned long debounceTime = 0;   // Time for debouncing
const int debounceDelay = 500;    // Delay for stable detection in milliseconds

void setup() {
  lcd.begin(16, 2);               // Initialize LCD with 16 columns and 2 rows
  Serial.begin(9600);             // Begin serial communication
  myservo.attach(6);              // Attach servo to pin 6
  myservo.write(0);               // Initialize barrier in closed state
}

long readDistance(int triggerPin, int echoPin) {
  // Function to read distance from ultrasonic sensor
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH);
}

void loop() {
  // Read distances from each sensor
  float d1 = 0.01723 * readDistance(trigPin1, echoPin1);
  float d2 = 0.01723 * readDistance(trigPin2, echoPin2);
  float d3 = 0.01723 * readDistance(trigPin3, echoPin3);
  float d4 = 0.01723 * readDistance(trigPin4, echoPin4);
  float d5 = 0.01723 * readDistance(trigPin5, echoPin5);

  Serial.println("d1 = " + String(d1) + "cm");
  Serial.println("d2 = " + String(d2) + "cm");
  Serial.println("d3 = " + String(d3) + "cm");
  Serial.println("d4 = " + String(d4) + "cm");
  Serial.println("d5 = " + String(d5) + "cm");

  unsigned long currentMillis = millis();

  // Control barrier for car entry and exit
  if (barrierState == 0) {
    if (d4 < distanceThreshold && d5 >= distanceThreshold && parkingAvailable > 0 && currentMillis - debounceTime > debounceDelay) {
      // Open barrier for entry
      parkingAvailable -= 1;
      barrierState = -1;
      myservo.write(90);  
      debounceTime = currentMillis;
    } else if (d4 >= distanceThreshold && d5 < distanceThreshold && parkingAvailable < 3 && currentMillis - debounceTime > debounceDelay) {
      // Open barrier for exit
      barrierState = 1;
      myservo.write(90);  
      debounceTime = currentMillis;
    }
  } 
  // Car entered and closing gate
  else if (barrierState == -1) {
    if (d4 >= distanceThreshold && d5 < distanceThreshold && currentMillis - debounceTime > debounceDelay) {
      barrierState = -2;
      myservo.write(0);  // Close barrier after entry
      debounceTime = currentMillis;
    }
  }
  // Car exited and closing gate
  else if (barrierState == 1) {
    if (d5 >= distanceThreshold && d4 >= distanceThreshold && currentMillis - debounceTime > debounceDelay) {
      myservo.write(0);  // Close barrier after exit
      parkingAvailable += 1;
      barrierState = 0;  
      debounceTime = currentMillis;
    }
  } 
  // Reset after entry completion
  else if (barrierState == -2 && d5 >= distanceThreshold && currentMillis - debounceTime > debounceDelay) {
    barrierState = 0;
    debounceTime = currentMillis;
  }

  // Close barrier if any slot is occupied
  if (d1 < distanceThreshold || d2 < distanceThreshold || d3 < distanceThreshold) {
    myservo.write(0);
  }

  // Display parking availability on LCD
  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 0);
  if (parkingAvailable == 0) {
    lcd.print("Parking Full   ");
  } else {
    lcd.print("Parking left: ");
    lcd.print(parkingAvailable);
    lcd.print("   ");
  }

  // Display free slots on LCD
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  if (d1 > distanceThreshold && d2 > distanceThreshold && d3 > distanceThreshold) {
    lcd.print("Slot 1 2 3 Free ");
  } else if (d1 > distanceThreshold && d2 > distanceThreshold) {
    lcd.print("Slot 1 & 2 Free ");
  } else if (d1 > distanceThreshold && d3 > distanceThreshold) {
    lcd.print("Slot 1 & 3 Free ");
  } else if (d2 > distanceThreshold && d3 > distanceThreshold) {
    lcd.print("Slot 2 & 3 Free ");
  } else if (d1 < distanceThreshold && d2 < distanceThreshold && d3 < distanceThreshold) {
    lcd.print("Parking Full    ");
  } else if (d1 > distanceThreshold) {
    lcd.print("Slot 1 is Free  ");
  } else if (d2 > distanceThreshold) {
    lcd.print("Slot 2 is Free  ");
  } else if (d3 > distanceThreshold) {
    lcd.print("Slot 3 is Free  ");
  }
  
  delay(100);  // Delay to prevent LCD flicker
}
