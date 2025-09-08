// wow it's an rs latch
bool latched = false;
bool lastClock = false;
// pins
const int Clock = 10;
const int S =11;
const int R = 12;
const int Q = 13;

void setup() {
  // put your setup code here, to run once:
  pinMode(Clock, INPUT);
  pinMode(S, INPUT);
  pinMode(R, INPUT);
  pinMode(Q, OUTPUT);
}

void loop() {
  if(digitalRead(R))
  {
    latched = false;
    digitalWrite(Q, latched);
  }
  bool thisClock = digitalRead(Clock);
  // trigger on rising edge.
  bool shouldTrigger = thisClock && ! lastClock;
  lastClock = thisClock;
  if(!shouldTrigger) return;

  //if(digitalRead(S)&&digitalRead(R))latched = !latched;
  //else
  {
 
    if(digitalRead(S))latched = true;
    if(digitalRead(R))latched = false;   
  }
  digitalWrite(Q, latched);
}
