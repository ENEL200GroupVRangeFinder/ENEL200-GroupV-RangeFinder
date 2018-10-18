//Final Arduino code listing

#include <math.h>
#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>

// define pin numbers
// SR sensor
const int trigPin = 9;
const int echoPin = 10;
// serial 7 segment display
const int softwareTx = 8;
const int softwareRx = 7;
SoftwareSerial s7s(softwareRx, softwareTx);
// serial audio player
const int playerTx = 4;
const int playerRx = 3;
SoftwareSerial mySoftwareSerial(playerRx, playerTx); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

//inputs
const int voiceActive = 5;
const int setDistance = A0;

// define variables
long duration = 0.0;
float distance = 0.0;
double distanceDisplayed = 0.0;
int disTenth = 0;
int distanceMSD = 0;
bool buttonPressed = true;
bool rangeError = false;
char tempString[10] = {0};  // Used with sprintf to create strings for the 7 seg display

void setup() {
  // set up SR sensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input

  // set up button and distance set
  pinMode(voiceActive, INPUT);  // Sets voiceActive pin as Input
  pinMode(setDistance, INPUT);  // Sets setDistance pin as an Input 
  
  // set up serial monitor
  Serial.begin(9600); // Starts the serial communication
  
  //set up s7s display at default 9600 baud
  s7s.begin(9600);
  clearDisplay();         //Clears display, resets cursor
  s7s.print("-HI-");      // Displays -HI- on all digits
  setDecimals(0b111111);  // Turn on all decimals, colon, apos
  setBrightness(250);     // High'ish brightness
  delay(2000);
  clearDisplay();  

  // set up DF player
  mySoftwareSerial.begin(9600);
  myDFPlayer.begin(mySoftwareSerial);
  myDFPlayer.volume(20);
  delay(100);
}

void getDistance(void)
{
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(100);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.00034 / 2;
}

void displayResults(void)
{
  if (distance > 4.00) {
    Serial.print("Sensor Blocked or Out of Range\n");
    //play(12) where 12 represents the words "Sensor Blocked or Out of Range"
    //Display "ERR" on screen
    rangeError = true;
    //delay(750); // need to callibrate the timings through testing
  }
  else
  {
    // Prints the distance on the Serial Monitor in metres.. to print on screen or send signal to it later
    rangeError = false;
    distanceMSD = (int) floor(distance);
    disTenth = (int) round((distance - distanceMSD) * 10);
    Serial.print("\nDistance: ");
    Serial.println(distance);
    Serial.print("MSD: ");
    Serial.print(distanceMSD);
    Serial.print("\nLSD: ");
    Serial.print(disTenth);
    Serial.print("\n");
    Serial.print((int) floor(1000*distance));
    Serial.print("\n");
    Serial.print("Button:");
    Serial.print(buttonPressed);
    Serial.print("\n");
    delay(750); // Callibrate timings through testing
  }
}

void playVoice(int firstDigit, int secondDigit)
{
  if (rangeError) {
    return;
  }
  if (firstDigit == 0) {
    firstDigit = 10;
  }
  if (secondDigit == 0) {
    secondDigit = 10;
  }

  myDFPlayer.play(firstDigit);
  delay(500);
  myDFPlayer.play(11); // point
  delay(450);
  myDFPlayer.play(secondDigit);
  delay(500);
  myDFPlayer.volume(15); // lower volume for metres
  delay(50);
  myDFPlayer.play(12); // metres
  delay(650);
  myDFPlayer.volume(20); // increase volume again
}

void loop() {
  getDistance();          //gets the distance from the sensor
  //getVoicedDistance();  //gets the 1 d.p. distance value used to reference the voice playlist
  //delay(50);
  displayResults();       //displays the results on the screen (rn its displaying to the serial monitor)
  delay(50);
  display_s7s();
  check_button();       //checks whether the button is pressed
  if (buttonPressed) {
    playVoice(distanceMSD, disTenth);
  }
  delay(50);
}

/* Functions for the s7s display */

void display_s7s(void)
{
  s7s.write(0x76);
  if (rangeError == false)
  {
  sprintf(tempString, "%4d", (int) floor(1000*distance) );
  s7s.print(tempString);  
  setDecimals(0b000001);
  }
  else
  {
  s7s.print("ERR");
  setDecimals(0b000100);
  }
  delay(25);
}

// Send the clear display command (0x76)
//  This will clear the display and reset the cursor
void clearDisplay()
{
  s7s.write(0x76);  // Clear display command
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightness(byte value)
{
  s7s.write(0x7A);  // Set brightness command byte
  s7s.write(value);  // brightness data byte
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimals(byte decimals)
{
  s7s.write(0x77);
  s7s.write(decimals);
}

/* check for button press / switch*/
void check_button(void) //check button which pin etc?
{
  for (int i = 0; i < 5; i++) //for loop does very ugly debounce
  {
    if (digitalRead(voiceActive) == HIGH)
      {
        buttonPressed = true;
      }
      else
      {
      buttonPressed = false;
      }
  delay (10);
  }
 } 
