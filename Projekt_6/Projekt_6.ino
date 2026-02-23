int sensorValue; //Aktuelle Senor Value
int sensorLow = 1023; //Min SensorValue
int sensorHigh = 0; //Max SensorValue

const int ledPin = 13; //LED 13 Leuchtet Während der Kalibrierunng


void setup() {
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin, HIGH);

  while(millis() < 5000) {
    sensorValue = analogRead(A0);
    if(sensorValue > sensorHigh) {
      sensorHigh = sensorValue;
    }
    if(sensorValue < sensorLow) {
      sensorHigh = sensorValue;
    }

  }
  digitalWrite(ledPin, LOW);
}

void loop() {
  sensorValue = analogRead(A0);

  int pitch =
      map(sensorValue, sensorLow, sensorHigh, 50, 4000);

      tone(8,pitch, 20);
      delay(5);

}
