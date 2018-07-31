/*
 * Ayoub Raji
 * Buzzer managed by Arduino Uno
 * 
 */

int buzzer = 12;//the pin of the active buzzer
unsigned char i;
int command;
bool buzz = false;
bool immediate_stop = false;
bool to_stop = false;
bool was_buzzing = false;
long timer;
int msg;

void setup()
{ 
  // setup serial port
  Serial.begin(9600); 
  pinMode(buzzer,OUTPUT);//initialize the buzzer pin as an output
}
void loop()
{
  if (Serial.available()>0)
  {
    command = Serial.read();   

    switch(command)
    {
      case 0:
        buzz = false;
        immediate_stop = true;
        to_stop = false;
      break;
      
      case 1:
        buzz = true;
        timer = millis();
        immediate_stop = false;
        to_stop = false;
      break;
      
      case 2:
        buzz = false;
        immediate_stop = false;
        to_stop = true;
      break;
    }

    //If the state has changed and it is buzzing or it has to stop immediately, 
    //we have to comunicate with the middleware
    if(buzz != was_buzzing)
    {
      if(buzz || immediate_stop)
      {
        writeBySerial();
      } 
    }
    
  }

 if(buzz || to_stop && millis() - timer <= 10000 && !immediate_stop)
 {
   //output a frequency
   for(i=0;i<80;i++)
    {
      digitalWrite(buzzer,HIGH);
      delay(1);//wait for 1ms
      digitalWrite(buzzer,LOW);
      delay(1);//wait for 1ms
    }
    
   //output another frequency
   for(i=0;i<100;i++)
    {
      digitalWrite(buzzer,HIGH);
      delay(2);//wait for 2ms
      digitalWrite(buzzer,LOW);
      delay(2);//wait for 2ms
    }
    
  }
  else if(to_stop && millis() - timer > 10000)
  {
    //The timer expired, so the alarm is not ringing and this must be comunicated to the middleware
    writeBySerial();
    to_stop = false;
  }
  
} 

void writeBySerial()
{
  msg = buzz ? 1 : 0 ;
  Serial.write(msg);
  was_buzzing = buzz;
}

