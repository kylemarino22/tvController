#include<SPI.h>
#include <SD.h>
 
 File myFile;

char rxByte = 0;

#define IRpin 2
#define IRpin_PIN PIND
#define MAXPULSE 8000
#define RESOLUTION 100
byte bytebuf[100];
byte bytebuf2[100];
uint8_t currentpulse = 0;
String correctionString;
uint16_t initialPulse = 0;
int ArduinoBusyPin = 8;

bool initialized = false;
bool checked = false;

//======================================================================================

void setup()
{
  Serial.begin(9600);
   pinMode(9,INPUT);
   pinMode(ArduinoBusyPin, OUTPUT);
   digitalWrite(ArduinoBusyPin, LOW);
  

 
}
//======================================================================================

//This is the function that gets the handle from the SD card
String receiver(){
  String finalString = "";
  char incomingChar = ' ';
  bool hasread = false; 
  bool failed = false;
  byte attemptcounter = 0;

  while(hasread == false){

      if( !SD.exists("CODE.TXT")){
        Serial.println("Code.TXT is empty, Not doing anything");
        return "empty";
      }
      myFile = SD.open("CODE.TXT");
      if (myFile) {
        Serial.println("myFile");
        // read from the file until there's nothing else in it:
        
        while (myFile.available()) {
          Serial.println("available");
          incomingChar = myFile.read();
          Serial.println(incomingChar);
          failed = false;
          if( !((int)incomingChar >= 0)){
            Serial.println("broken");
            failed = true;
            break;
          }
          
          finalString += incomingChar;
          
        }
    
        // close the file:
        if(failed == false){
          hasread = true;
        }
        myFile.close();
      } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
        myFile.close();
        attemptcounter++;
      }
      if(attemptcounter>10){
        Serial.println("failed to read from Code.txt 10 times");
        hasread = true;
      }
  }
  Serial.println(finalString);
  return finalString;

}
//======================================================================================
//This is the function that uses the handle to read the intended code from the SD card
void handler(String str){
 byte counter = 1; 
 int tempIncomingInt = 0;
 
  myFile = SD.open(str);
  if (myFile) {


   
    while (myFile.available()) {
      
      char incomingInt = myFile.read();
      if(incomingInt == ','|| incomingInt == '@'){
        bytebuf[counter] = (byte)(tempIncomingInt);
        tempIncomingInt = 0;
        counter++;
      }
      else if(incomingInt == ' '){
        continue;
      }
      else{
        tempIncomingInt = (tempIncomingInt * 10) + (incomingInt - 0x30);
        
      }
    }
  }
  myFile.close();
  bytebuf[0] = counter - 1;
  

}

//======================================================================================

byte pathDecider(String str){
  if(str == "NEW1.TXT"||str == "NEW2.TXT"||str == "NEW3.TXT"){
    return 0;
  }
  else if(str == ""){
    return 1;
  }
  else if(str == "empty"){
//    initialized = false;
    return 3;
  }
  else {
    return  2;
  }
  
}

//======================================================================================
// Code for IR detection

void PulseDetector(){
bool finished = false;
Serial.println("IRDetection");
  while(finished == false){
    
    uint16_t highpulse, lowpulse; // temporary storage timing
    highpulse = lowpulse = 0; // start out with no pulse length
    
    
  // while (digitalRead(IRpin)) { // this is too slow!
      while ( ((IRpin_PIN & _BV(IRpin)) != 0) && (finished == false) ) {
       // pin is still HIGH
  
       // count off another few microseconds
       highpulse++;
       delayMicroseconds(RESOLUTION);
       
       if ((highpulse * RESOLUTION >= MAXPULSE) && (currentpulse >3)) {
         
         
         finished = true;
         Serial.println("worked");
         currentpulse=0;
         
         
         
         
       }
    }
    
    if(finished == true){
       break;
    }
    // we didn't time out so lets stash the reading
    
    bytebuf2[currentpulse] = highpulse;
    if(currentpulse == 0){
      initialPulse = highpulse;
      Serial.println(highpulse);
    }
   
    currentpulse++;
    
    // same as above
    while ( !(IRpin_PIN & _BV(IRpin)) && (finished == false) ) {
       // pin is still LOW
       lowpulse++;
       delayMicroseconds(RESOLUTION);
       if ((lowpulse * RESOLUTION >= MAXPULSE) && (currentpulse > 3)) {
         
        
         Serial.println("worked");
         currentpulse=0;
         finished = true;
       }
    }

    if(finished == true){
       break;
    }
    
    bytebuf2[currentpulse] = lowpulse;
    
    if(currentpulse == 0){
      initialPulse = lowpulse;
      Serial.println(lowpulse);
    }
    currentpulse++;
  }
  Serial.println("Pulse detector Finishes");
 return;
 
}



//======================================================================================
//Code that converts array to String


void toString(byte arr[],int arraySize){
//Converts Array to string
correctionString = "";
    for(int i = 0; i < arraySize; i++){
      int tempByte = (int)arr[i];
      
      if(tempByte < 10){
        correctionString += (char)(arr[i] + 0x30);
          if(i == arraySize-1){
          correctionString += '@';
        }
        else{
          correctionString += ',';
        }
      }
      else{
        byte temp = 0;
        temp = arr[i] / 10;
        correctionString += (char)(temp + 0x30);
        arr[i] -= temp *10;
        i -= 1;
        
      }
      
    }
    
  
}

//======================================================================================

void toSDCard(String q, bool state){
  bool fileValid = false;
  byte counter = 1;
  String a;
  Serial.println(correctionString);
  if(q == "reset"){
     
    a = "CODE.TXT";
    Serial.println(a);
    Serial.println("reset");
    
    
  }else{
    if(state == 0){
      q.replace("#","");
      a = '#' + q;
      Serial.println(a);
      Serial.println("code");
    }
    else{
      a = q; 
    }
  }
  
    while(fileValid == false){
  SD.remove(a);
  if(SD.exists(a)){
   
  }
  else{
    fileValid = true;
  }
}

fileValid = false;
   myFile = SD.open(a, FILE_WRITE);
   while(fileValid == false){
    if (myFile) {

Serial.println("there");
      Serial.println("This is the correctionString being sent");

      if(state == 1){
        myFile.print(correctionString);
      }
      
      
      
    fileValid = true;
    
    
    myFile.close();
     }
     else{
      counter ++;
      myFile.close();
      SD.remove(a);
      myFile = SD.open(a, FILE_WRITE);
     }

     if(counter > 9){
      fileValid = true;
      while(digitalRead(9) == 1){
        Serial.println("waiting for ESp to disconnect");
      }
      
      Serial.println("fail after 10 attempts");
      myFile.close();
     }
     
   }
   
  Serial.println("reading");
myFile = SD.open(a);
  if (myFile) {


    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
    myFile.close();
  }
  Serial.println("read");
   
}

//======================================================================================



void loop()
{
   String codeLocation;
   

   if(digitalRead(9) == HIGH) {
      delay(10);
      
      if(digitalRead(9) == LOW) {
        delay(10);
        while(initialized == false){
          delay(100);
          if (!SD.begin(10)) {
            Serial.println("initialization failed!");
            delay(10);
          }
          else{
            initialized = true; 
            break;
            
          }
        }
        


         //======================================================================================
         //Code to be processed after Esp has checked sd card

         digitalWrite(ArduinoBusyPin, HIGH);
         Serial.println("ArduinoBusy");
        
         codeLocation = receiver();
            
         switch(pathDecider(codeLocation)){
              
            //===============================================================================
              case 0:
              Serial.println("new code being Added");
              
              PulseDetector();

              byte noP;
              for(byte i = 0; i < 2; i++){
                if(bytebuf2[noP+2] == 0){
                  if(codeLocation == "NEW3.TXT"){
                    break;
                  }
                  else{
                  toString(bytebuf2,noP);
                  Serial.println(noP);
                  break;
                  }
                }
                bytebuf2[noP] = bytebuf2[noP + 1];
                noP++;
                
                i = 0;
              }
              
              Serial.println(correctionString);

              while(1){
                if(digitalRead(9) == HIGH) {
                  delay(10);
                  
                  if(digitalRead(9) == LOW) {
                    delay(10);

                    break;
                  }
                }
              }

              if(codeLocation == "NEW3.TXT"){
                handler("NEW1.TXT");
                for(int i = 0; i < noP; i++){
                  Serial.print(bytebuf2[i]);
                  Serial.print("     ");
                  Serial.print(bytebuf[i]);
                  Serial.print("     ");
                  bytebuf2[i] = (bytebuf2[i] + bytebuf[i+1]) / 2;
                  Serial.println(bytebuf2[i]);
                }
                handler("NEW2.TXT");
                Serial.println("second avg");
                for(int i = 0; i < noP; i++){
                  Serial.print(bytebuf2[i]);
                  Serial.print("     ");
                  Serial.print(bytebuf[i]);
                  Serial.print("     ");
                  bytebuf2[i] = (bytebuf2[i] + bytebuf[i+1]) / 2;
                  Serial.println(bytebuf2[i]);
                }
                toString(bytebuf2, noP);
                Serial.println(codeLocation);
                toSDCard("NAME.TXT", 1);
                SD.remove("NEW1.TXT");
                SD.remove("NEW2.TXT");
                SD.remove("NEW3.TXT");
                
              }
              else{
                toSDCard(codeLocation, 1);
              }
              
              toSDCard("reset",0);
              initialized = false;
              
              break;
            

  
             
                
              case 1:
       //===============================================================================
              
        
              break;
                
              case 2:
       //===============================================================================
              PulseDetector();

              Serial.println("initialPulse");
              Serial.println(initialPulse);
              handler(codeLocation);
              
              
              
              byte len = bytebuf[0];
              //bytebuf[0] holds the size of the array
              //bytebuf2[0] holds two extra timings
              Serial.println("Code Information");
              Serial.println();
              Serial.println(bytebuf[1]);
              for(int i = 0; i < len + 2; i++){
                Serial.print(bytebuf[i+1]);
                Serial.print("     ");
                Serial.print(bytebuf2[i+1]);
                Serial.print("     ");

                if((bytebuf[i+1])- (bytebuf2[i+1]) < 2 && (bytebuf[i+1])- (bytebuf2[i+1]) > -2){
                  bytebuf2[i] = 255;
                  
                }
                else if((bytebuf[i+1])- (bytebuf2[i+1]) > 5 || (bytebuf[i+1])- (bytebuf2[i+1]) < -5){
                  bytebuf2[i] = 100 +  (bytebuf[i+1])- (bytebuf2[i+1]) + (bytebuf[i+1]);
                  
                }
                else{
                  bytebuf2[i] = (bytebuf[i+1])- (bytebuf2[i+1]) + (bytebuf[i+1]);
                  
                }
                
                Serial.print(bytebuf2[i]);
                Serial.println();
                  
              }
              
              // first step of calculations have been done, now to the second step
              // download what was sent to bytebuf
              
              //Check to see if a # version exists and get new bytebuf
              //If a # version doesn't exist, no need for new bytebuf
              if(SD.exists("#" + codeLocation)){
                codeLocation = "#" + codeLocation;
                handler(codeLocation);
              }
              //get length of bytebuf from index 0
              
              len = bytebuf[0];
              Serial.println("Final Corrections");
              //for length of bytebuf, check to see if any codes are 255
              for(int i = 0; i < len; i++){
                if ( bytebuf2[i] > 100 && bytebuf2[i] < 250){
                  bytebuf2[i] = bytebuf2[i] - 200 + bytebuf2[i] - bytebuf[i +1];
                  Serial.println(bytebuf2[i]);
                  
                }
                if(bytebuf2[i] == 255){
                  //bytebuf2 location equales corresponding bytebuf location
                  bytebuf2[i] = bytebuf[i+1];
              
                }
                Serial.println(bytebuf2[i]);
              }
              

            
             
             

             
             toString(bytebuf2,bytebuf[0]);
             //toSDCard takes in another parameter, correctionString, but it is a global
             //variable
             toSDCard(codeLocation,0);
             correctionString = "";
             
             toSDCard("reset",1);
             
                 

                
              break;
              
            }
          digitalWrite(ArduinoBusyPin, LOW);
          Serial.println("ArduinoNotBusy");
            
//======================================================================================        
      }
    }

  
}
    
//======================================================================================

