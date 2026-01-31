void setup() {
  // put your setup code here, to run once:
  pinMode(3, INPUT); 
  Serial.begin(19200);

}

void loop() {
  // put your main code here, to run repeatedly:
  while(!digitalRead(3)){}
  Serial.println("Pulse");
  while(digitalRead(3)){}
  
}
