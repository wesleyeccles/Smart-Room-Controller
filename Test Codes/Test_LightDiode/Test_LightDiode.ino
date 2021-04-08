int LIGHTDIODE  = 23;
int LightVal;
void setup() {
 pinMode(LIGHTDIODE, INPUT);
}

void loop() {
 delay(10);
 LightVal = analogRead(LIGHTDIODE);   
 Serial.println(LightVal);
}
