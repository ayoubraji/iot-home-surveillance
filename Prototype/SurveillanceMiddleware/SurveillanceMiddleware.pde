import mqtt.*;

/*
 * Ayoub Raji
 * APPLICATION MIDDLEWARE
 * 
 */
import processing.serial.*;
import processing.net.*;
Client myClient; 

Serial mySerialPort=null;
MQTTClient client;

String messageToSend = "";

boolean stop_requested = false;

String bot_token = "xxx";
String chat_id = "xxx";

void setup() {
  client = new MQTTClient(this);
  
  //The real mqtt broker is broker.shiftr.io
  client.connect("mqtt://try:try@broker.shiftr.io", "processing");
  
  //Subscribing to topics
  client.subscribe("/doorSensor");
  client.subscribe("/alarm");
  
  //Init serial port to comunicate with ArduinoAlarmStation
  String portName="COM4";
  mySerialPort = new Serial(this,  portName, 9600);
}

void draw() 
{
  
  //read from serial the new status of ArduinoAlarmStation [1:Ringing, 0:Quiet]
  int alarmStatus = readFromSerial();
  
  if(alarmStatus == 1)
  {
    println("Alarm status: "+alarmStatus);
    String command = "curl -d '' -X POST " + " http://localhost:80/surveillanceStatus/?value=Ringing&sensorid=1&type=alarm";
    execCurl(command);
  }
  else if(alarmStatus == 0)
  {
    println("Alarm status: "+alarmStatus);
    String command = "curl -d '' -X POST " + " http://localhost:80/surveillanceStatus/?value=Quiet&sensorid=1&type=alarm";
    execCurl(command);
  }
  
}

//Receive update from subscribed topics
void messageReceived(String topic, byte[] payload) {
  String message = new String(payload);
  String command;
  
  switch(message)
  {
    case "opened":
    
    //Update door sensor value on the server
    command = "curl -d '' -X POST " + " http://localhost:80/surveillanceStatus/?value=Opened&sensorid=1&type=door";
    execCurl(command);
    
    //Send message to the bot
    String text = "DoorOpened!";
    command = "curl -i -X GET https://api.telegram.org/bot"+bot_token+"/sendMessage?chat_id="+chat_id+"&text="+text;
    execCurl(command);
    
    //send to arduino a signal to emit the alarm
    if(!stop_requested)
    {
      client.publish("/alarm", "ring");
      sendBySerial(1);
    } 
    
    break;
    
    case "closed":
    
    //Update door sensor value on the server
    command = "curl -d '' -X POST " + " http://localhost:80/surveillanceStatus/?value=Closed&sensorid=1&type=door";
    execCurl(command);
    
    //send to arduino a signal to stop the alarm
    client.publish("/alarm", "stop");
    sendBySerial(2);
    stop_requested = false;
    
    break;
    
    case "stop_alarm":
    
    stop_requested = true;
    //send to arduino a signal to stop the alarm
    sendBySerial(0);
    
    break;
  }

}

void sendBySerial(int value)
{
  if (mySerialPort!= null)
  {
     mySerialPort.write(value);
  }
}

int readFromSerial()
{
  int result = -1;
  if (mySerialPort.available() > 0)
  {
     result = mySerialPort.read();
  }
  
  return result;
}

void execCurl(String command)
{
  try
  {
    Runtime.getRuntime().exec(command);
  } catch (IOException e1) {
        e1.printStackTrace();
  }
}
