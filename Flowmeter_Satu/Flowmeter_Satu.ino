#include <Arduino.h> 
#include <EEPROM.h> 
#include <UbidotsESPMQTT.h>

/****************************************
 * Define Constants
 ****************************************/
#define TOKEN "BBFF-cu3D8dlx7oaan9S6ZIwuqHHPIHiai1" // Your Ubidots TOKEN
#define WIFINAME "www.interactiverobotics.club" //Your SSID
#define WIFIPASS "cilibur2019" // Your Wifi Pass
#define MQTTLabelName "Flowmeter1"
Ubidots client(TOKEN,MQTTLabelName);
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

byte statusLed    = D5;
byte sensorInterrupt = D2;  // 0 = digital pin 2
byte sensorPin       = D2;

// The hall-effect flow sensor outputs approximately 4.5 pulses per second per
// litre/minute of flow.
float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

unsigned long oldTime;


void setup() {
    Serial.begin(115200); // Start the Serial communication to send messages to the computer
    delay(10);
    Serial.println('\n');

    client.setDebug(true); // Pass a true or false bool value to activate debug messages
    client.wifiConnection(WIFINAME, WIFIPASS);
    client.begin(callback);

    // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

  // EEPROM.put(1,totalMilliLitres);
  EEPROM.put(1,0); 
  delay(1000);
// ############################################
// ############################################
// Initiaize the value of totalMilliLiters 
  if (EEPROM.read(0) != 0xFF){            
      EEPROM.put(1,totalMilliLitres);  
} else {
      EEPROM.get(1,totalMilliLitres);
}
    Serial.println("Startup EPROM Value  :");
    Serial.print("Output Liquid Quantity: ");        
    Serial.println(totalMilliLitres);
// ############################################
    }

void loop() {
   if(!client.connected()){
      client.reconnect();
      }
        if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
      
    unsigned int frac;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");        
    Serial.print(totalMilliLitres);
    Serial.print("mL"); 
    Serial.print("\t");       // Print tab space
    Serial.print(totalMilliLitres/1000);
    Serial.println("L");
    client.add("FLOWMETER1", totalMilliLitres);
    client.ubidotsPublish("GrupData");
    client.loop();

  // ######################################################
  // ######################################################
  // Save the Value in EEPROM
    EEPROM.put(1,totalMilliLitres);           //  Was read value  EEPROM.get(1,totalMilliLitres);
  // ######################################################
  // ###################################################### 
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  } 
}

/*
Insterrupt Service Routine
 */
void pulseCounter() {
    // Increment the pulse counter
    pulseCount++;
   
}
