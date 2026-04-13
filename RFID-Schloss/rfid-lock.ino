//RFID Library
#include <MFRC522.h>
//Serial Peripheral Interface communication with Peripheral Devices (RFID Card Reader)
#include <SPI.h>
//Liqid crystal Library
#include <LiquidCrystal_I2C.h>
//Servo Library
#include <Servo.h>

//Define RFID Pins
#define SS_PIN 10
#define RST_PIN 5

//Define Pin the Servo Is connected tp
#define SERVO_PIN A5

#define ALARM_PIN 3

//Define Rfid parameters
MFRC522 rfid(SS_PIN, RST_PIN);

//Define Liquid Crystal parameters
LiquidCrystal_I2C lcd(0x27, 16, 2);


//Allowed Key UIDs, Erlaubte Schlüssel UIDs
byte masterkeyUID[4] = {};
byte fragenUID2[4] = {};

//Create struct that defines how a question(arry part) should look like
struct Frage {
  const char* frage;
  const char* antwort1;
  const char* antwort2;
  const char* antwort3;
  int richtigeAntwort;
};

//Define questions in an array with array parts consisting of Fragen structure
Frage fragen[] = {
  { "aaaaaaaaaaa",
    "aaaaaa",
    "aaaaa",
    "aaaaaaa",
    1 },
  { "aaaaaaaaaaa",
    "aaaaaa",
    "aaaaa",
    "aaaaaaa",
    1 },
  { "aaaaaaaaaaa",
    "aaaaaa",
    "aaaaa",
    "aaaaaaa",
    1 },
  { "aaaaaaaaaaa",
    "aaaaaa",
    "aaaaa",
    "aaaaaaa",
    1 }
};

//Total number of questions
const int numFragen = sizeof(fragen) / sizeof(fragen[0]);
int failed_attempts = 0;
bool alarm_reset = false;
bool alarm_fragen_reset = true;
bool benutzt[numFragen] = { false };  // Array which stores already asked questions
int button1 = 1;                      // Button1 Pin define
int button2 = 2;                      // Button2 Pin define
int button3 = 3;                      // Button3 Pin define
Servo servo;

//Define interger type
int warteAufButton() {
  while (true) {
    if (digitalRead(button1) == LOW) {
      while (digitalRead(button1) == LOW)
        ;
      //return that response from user is answer a
      return 1;
    } else if (digitalRead(button2) == LOW) {
      while (digitalRead(button2) == LOW)
        ;
      //return that response from user is answer b
      return 2;
    } else if (digitalRead(button3) == LOW) {
      while (digitalRead(button3) == LOW)
        ;
      //return that response from user is answer c
      return 3;
    }
  }
}

void setup() {
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  Serial.begin(9600);
  SPI.begin();      // initialize SPI bus
  rfid.PCD_Init();  // initialize MFRC522

  lcd.init();
  lcd.backlight();
  lcd.print("Bitte Schlüssel");
  lcd.setCursor(0, 1);
  lcd.print("Präsentieren");

  servo.attach(SERVO_PIN);
  servo.write(0);  // rotate servo motor to 0°

  randomSeed(analogRead(A0));


  Serial.println("Tap RFID/NFC Tag on reader");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent()) {
    if (rfid.PICC_ReadCardSerial()) {
      MFRC522::PICC_Type picc_type = rfid.PICC_GetType(rfid.uid.sak);

      //Check if presented Key is MasterKey
      if (rfid.uid.uidByte[0] == masterkeyUID[0] &&
          rfid.uid.uidByte[1] == masterkeyUID[1] &&
          rfid.uid.uidByte[2] == masterkeyUID[2] &&
          rfid.uid.uidByte[3] == masterkeyUID[3]) {
        Serial.println("Master Schlüssel");
        resetAlarm("MasterKey");
        masterkey();
        } // Check if presented Key is FragenKey
      else if (rfid.uid.uidByte[0] == fragenUID2[0] &&
               rfid.uid.uidByte[1] == fragenUID2[1] &&
               rfid.uid.uidByte[2] == fragenUID2[2] &&
               rfid.uid.uidByte[3] == fragenUID2[3]) {
        Serial.println("Fragen Schlüssel");
        resetAlarm("FragenKey");
        fragen_stellen();
        }
      //If any other Key which is not allowed
      else {
        failed_attempts++;
        Serial.print("Unzulässiger Schlüssel:");
        lcd.setCursor(0,0);
        lcd.print("Unerlaubter");
        lcd.setCursor(0,1);
        lcd.print("Schlüssel");
        delay(2000);
        lcd.setCursor(0,0);
        lcd.print("Falscher Versuch");
        lcd.setCursor(0,1);
        lcd.print("Nr");
        lcd.print(failed_attempts);
        lcd.print("");
        for (int i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(rfid.uid.uidByte[i], HEX);
        }
        Serial.println();

        if (failed_attempts >= 3) {
          alarm();
        }
      }
      rfid.PICC_HaltA();       // halt PICC
      rfid.PCD_StopCrypto1();  // stop encryption on PCD
    }
  }
}

//Turn the servo motor ("open" the door)
void turnServo(int wait_sec, int first_angle, int second_angle) {
  servo.write(first_angle);  //turn servo
  delay(wait_sec * 1000);    //wait sec
  servo.write(second_angle);
}

//Executed if MasterKey is presented
void masterkey() {
  failed_attempts = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Master Key");
  lcd.setCursor(0, 1);
  lcd.print("Zugang gewährt");
  turnServo(5, 90, 0);

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bitte Schlüssel");
  lcd.setCursor(0, 1);
  lcd.print("Präsentieren");
}

//Executed if FragenKey is presented
void fragen_stellen() {
  lcd.setCursor(0, 0);
  lcd.print("Fragen Schlüssel");
  delay(2000);
  lcd.clear();

  for (int i = 0 ; i < numFragen; i++) {

    int index;

    //Zufällige Frage suchen, die noch nicht verwendet wurde
    do {
      index = random(0, numFragen);
    } while (benutzt[index] == true);

    benutzt[index] = true;

    //Ausgewählte Frage anzeigen
    lcd.setCursor(0, 1);
    lcd.print(fragen[index].frage);
    delay(2000);

    //Antworten anzeigen
    clearLCDLine(1);
    lcd.print("A: ");
    lcd.print(fragen[index].antwort1);
    delay(1500);

    clearLCDLine(1);
    lcd.print("B: ");
    lcd.print(fragen[index].antwort2);
    delay(1500);

    clearLCDLine(1);
    lcd.print("C: ");
    lcd.print(fragen[index].antwort3);
    delay(1500);

    //create integer of type warteAufbutton
    int antwort = warteAufButton();

    if (antwort != fragen[index].richtigeAntwort) {
      alarm_fragen_reset = false;
      alarm();
      break;
    } else if (antwort == fragen[index].richtigeAntwort) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Richtige Antwort");
      delay(3000);
      lcd.clear();
    }
  }
}

//Trigger Alarm
void alarm() {
  while (alarm_reset == false)  //Alarm stays on til reset
  {
    tone(ALARM_PIN, 20);
    delay(40);
    tone(ALARM_PIN, 20);
  }
}

//Reset Alarm
void resetAlarm(String key) {
  if (key == "MasterKey" || (key == "FragenKey" && !alarm_fragen_reset))  //only if MasterKey or (Fragen key and alarm not executed by Fragen)
  {
    //set alarm to stop(reset)
    alarm_reset = true;
    delay(2000);
    //alarm can be triggered again
    alarm_reset = false;
    //reset number of wrong key attempts
    failed_attempts = 0;
  }
  //If Reset is locked because alarm was triggered by fragen or other influence
  else {
    lcd.clear();
    lcd.print("Reset nicht");
    lcd.setCursor(0, 1);
    lcd.print("erlaubt");
  }
}
//                    Utils                       \\

//Clears a specified line of the LCD Display
void clearLCDLine(int line) {
  lcd.setCursor(0, line);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }
}