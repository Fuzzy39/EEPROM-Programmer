  int pins[] = { 2, 3, 4, 5 };
bool data[4][400];

void setup() {
  // put your setup code here, to run once:
   for(int i = 0; i<sizeof(pins)/sizeof(int); i++)
   {
    pinMode(pins[i], INPUT);
   }
   Serial.begin(19200);
}

void loop() {
  // put your main code here, to run repeatedly:

   //trigger on tx low (start bit)
  if(digitalRead(3)) return;

  // capture data
  for(int j = 0; j<400; j++)
     {
       for(int i = 0; i<sizeof(pins)/sizeof(int); i++)
        {
    
     
        data[i][j] = digitalRead(pins[i]);
        }
     }
  

   // spit out the data
    for(int j = 0; j<400; j++)
    {
         char* str = "x,x,x,x";

         for(int i = 0; i<sizeof(pins)/sizeof(int); i++)
         {
     
            str[i*2] = (data[i][j]?1:0) + 2*i + '0';
         }

         Serial.println(str);
     }
     
     for(int i = 0; i<50; i++)
     {
      Serial.println("0,3,5,6");  
     }
}
