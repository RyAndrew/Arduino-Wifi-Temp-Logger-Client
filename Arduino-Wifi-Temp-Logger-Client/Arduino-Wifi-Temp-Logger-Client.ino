//For OneWire Temp Sensors
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

boolean logTemps;
long tempLogLastMillis = 0;  
long tempLogInterval = 60000;  

void setup() {

  Serial1.begin(115200);

  sensors.begin();
  
  delay(3000);
  enterCommandMode();
  
  logTemps = true;
  tempLogLastMillis = millis();
}

void loop() {
  
  if(logTemps){
    //Serial.println("Log Temps!");

    unsigned long currentMillis = millis();

    //Serial.print(currentMillis - tempLogLastMillis);
    //Serial.println("Elapsed");

    if(currentMillis - tempLogLastMillis > tempLogInterval) {
      tempLogLastMillis = currentMillis;
      logTempsToCloud();
    }
  }

}

void logTempsToCloud()
{
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  //Serial.print("Temperature 1 is: ");
  //Serial.print((sensors.getTempCByIndex(0) * 9 / 5) + 32); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  //Serial.println(" F");
  //Serial.print("Temperature 2 is: ");
  //Serial.print((sensors.getTempCByIndex(1) * 9 / 5) + 32);
  //Serial.println(" F");

  //Serial.println("t1=" + (String)((sensors.getTempCByIndex(0) * 9 / 5) + 32) + "&t2=" + (String)((sensors.getTempCByIndex(1) * 9 / 5) + 32));

  sendHttpFormPostRequest("t1=" + (String)((sensors.getTempCByIndex(0) * 9 / 5) + 32) + "&t2=" + (String)((sensors.getTempCByIndex(1) * 9 / 5) + 32));
}

void sendHttpFormPostRequest(String formDataString)
{

  sendWifiSerialCmdWaitForOk("AT+TCPDISB=off");
  
  sendWifiSerialCmdWaitForOk("AT+SOCKB=TCP,80,2xod.com");
  
  sendWifiSerialCmdWaitForOk("AT+TCPDISB=on\r");
  
  waitForSocketBToConnect();
  
  String   HttpRequest = "POST /api/log2Temps/ HTTP/1.1\r\n";
    HttpRequest += "Host: 2xod.com\r\n";
    HttpRequest += "User-Agent: Arduino\r\n";
    HttpRequest += "Content-Type: application/x-www-form-urlencoded\r\n";
    HttpRequest += "Content-Length: "+(String)formDataString.length()+"\r\n";
    HttpRequest += "\r\n";
    HttpRequest += formDataString;
  unsigned int HttpRequestLength = HttpRequest.length();
  
  Serial1.print("AT+SNDB=" + (String)HttpRequestLength  + "\r" );
  Serial1.find(">");
  
  Serial1.print(HttpRequest);
  Serial1.find("+ok");

  sendWifiSerialCmdWaitForOk("AT+TCPDISB=off\r");

}

boolean waitForSocketBToConnect(){
  unsigned long waitTimeout = 1000; // wait 1 second to connect
  unsigned long start = millis();
  char lastCmdResultChar;
 
  Serial1.print("AT+TCPLKB\r");
    
  while( millis() - start < waitTimeout ){
    
    if (Serial1.available()) { //Output should be "+ok=on" so skip 1 char
        lastCmdResultChar = Serial1.read();
        //uncomment this for debugging. Will show "+ok=off" until a connection is made
        //Serial.write(lastCmdResultChar);
    }
    if(lastCmdResultChar == 'n'){ // Command Returned "+ok=on"
       //Serial.println("Connected in " + (String)(millis() - start) + " ms");
      return true;
    }else if(lastCmdResultChar == 'f'){ // Command Returned "+ok=off"
      lastCmdResultChar = 'x'; //Reset the character so we don't issue this command repeatedly
      Serial1.print("AT+TCPLKB\r");
    }
  }
  return false;
}
boolean enterCommandMode()
{
  //Serial.println("Enter Command Mode");
  
  Serial1.write('+');
  delay(10);
  Serial1.write('+');
  delay(10);
  Serial1.write('+');
  if(!Serial1.find("a")){
    //Serial.println("Error! No Ack from wifi module!");
    return false;
  }
  
  Serial1.write('a');
  if(!Serial1.find("+ok")){
    //Serial.println("Error! No OK received from wifi module!");
    return false;
  }  
  //Serial.println("Ready for commands!");
  return true;
}
boolean sendWifiSerialCmd(String command){
  Serial1.print(command + "\r");
}
boolean sendWifiSerialCmdWaitForOk(String command){
  
  sendWifiSerialCmd(command);
  
  if(Serial1.find("+ok")){
    return true;
  }else{
    //Serial.println("Error! No +ok received from command!");
    return false;
  }
  
}
