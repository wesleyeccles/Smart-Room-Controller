

const int Relay  =21;

void setup() {
  pinMode(Relay, OUTPUT);
  
}

void loop() {
  digitalWrite(Relay, HIGH);
  delay(10);
  digitalWrite(Relay, LOW);
  delay(10);
  digitalWrite(Relay, HIGH);
  delay(10);
  digitalWrite(Relay, LOW);
  delay(10); 
  digitalWrite(Relay,HIGH );
  delay(2000); 
     
}
