int clk = 2;
int trigger = 3;
int out = 4;
int triggerOut = 5;

// default output when nothing is happening.
bool defaultOut = true;

// whether to start transmission on the rising or falling edge of the clock cycle.
bool startRising = false;

// data to send for SH/LD signal, per half clock cycle.
// low is load, high is shift.
uint16_t data = 0b1100111111111111;

void setup() 
{
  // setup pins
  pinMode(clk, INPUT);
  pinMode(trigger, INPUT);
  pinMode(out, OUTPUT);
  pinMode(triggerOut, OUTPUT);

  Serial.begin(19200);

}

void loop() 
{
  // Write default output.
  digitalWrite(out, defaultOut);
  digitalWrite(triggerOut, HIGH);
  // wait for trigger.
  while(digitalRead(trigger)){}
  while(!digitalRead(trigger)){}
  
  // wait for clock to start
  while(digitalRead(clk) == startRising) {}
  while(digitalRead(clk) != startRising) {}
  
  Serial.println("Hey!");
  digitalWrite(triggerOut, LOW);
  
  // Time for action
  bool clockNow = startRising;
  for(int i = 0; i<16; i++)
  {
    
    // write the next bit.
    digitalWrite(out, (0x8000 >> i)&data?HIGH:LOW);
    
    Serial.println(i);
    
    // wait for clock to change
    while(clockNow == digitalRead(clk)){}
    clockNow = !clockNow;
    
  }

}
