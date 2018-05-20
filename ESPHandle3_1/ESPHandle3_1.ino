#include <SPI.h>
#include <SD.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

File myFile;
int tempIncomingInt;
byte finalValArray[99];
int IRledPin = 5;
char rxByte = 0;


//wifi
const char wifissid[] = "VaporTrails";
const char wifipsk[] = "-------------";
byte reqCounter = 0;
String req = "";
char incomingChar;
String sendRequest = "Code1";
bool newButtonInitiated = false;
String Stringbuf1 = "";

WiFiServer server(80);
//======================================================================================

void connectWiFi() {
  Serial.println();
  Serial.println("Connecting to: " + String(wifissid));
  WiFi.mode(WIFI_STA);

  WiFi.begin(wifissid, wifipsk);

  while (WiFi.status() != WL_CONNECTED) {

    delay(100);
  }
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

//======================================================================================
void setupMDNS()
{
  //Call mdns to setup mdns to pint to domain.local
  if (!MDNS.begin("thing"))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
}

void initHardWare()
{
  Serial.begin(115200);
}

//======================================================================================

void sender(String str, String location) {
  Serial.println("does this work");

  
  digitalWrite(15, HIGH);
  delay(10);

  
  
  
  bool fileValid = false;
  int counter = 1;
  while (fileValid == false) {
    if ( location != "CODE.TXT" ) {
      fileValid = true;
      Serial.println("does this work1");
      break;
    }
    
    if (SD.exists("CODE.TXT")) {
      SD.remove("CODE.TXT");
    }
    else {
      fileValid = true;
    }
  }

  fileValid = false;
  Serial.println("does this work2");
  myFile = SD.open(location, FILE_WRITE);
  Serial.println("does this work3");
  while (fileValid == false) {
    if (myFile) {

      Serial.println("there");


      myFile.print(str);


      fileValid = true;


      myFile.close();
    }
    else {
      counter ++;
      myFile.close();
      SD.remove(location);
      myFile = SD.open(location, FILE_WRITE);
    }

    if (counter > 9) {
      fileValid = true;
      Serial.println("fail after 10 attempts");
      myFile.close();
    }

  }
  delay(10);
}


//======================================================================================


void setup()
{
  Serial.begin(115200);
  pinMode(15, OUTPUT);

  digitalWrite(15, HIGH);
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  digitalWrite(15, LOW);

  pinMode(IRledPin, OUTPUT);



  initHardWare();
  connectWiFi();
  server.begin();
  setupMDNS();




}
//======================================================================================

String requestDecoder(String request) {
  request.replace("GET /NEWNAME" , "");
  request.replace(" HTTP/1.1" , "");
  return request;
}
//======================================================================================

String memLocationReader(String location) {
  //download unnamed code into memory
  digitalWrite(15,HIGH);
  String finalString = "";
  char incomingChar = ' ';
  bool hasread = false;
  bool failed = false;
  byte attemptcounter = 0;

  while (hasread == false) {

    myFile = SD.open(location);
    if (myFile) {
      Serial.println("myFile");
      // read from the file until there's nothing else in it:
      while (myFile.available()) {
        Serial.println("available");
        incomingChar = myFile.read();
        Serial.println(incomingChar);
        failed = false;
        if ( !((int)incomingChar >= 0)) {
          Serial.println("broken");
          failed = true;
          break;
        }

        finalString += incomingChar;

      }

      // close the file:
      if (failed == false) {
        hasread = true;
      }
      myFile.close();
    } else {
      // if the file didn't open, print an error:
      Serial.println("error opening test.txt");
      myFile.close();
      attemptcounter++;
    }
    if (attemptcounter > 10) {
      Serial.println("failed to read from Code.txt 10 times");
      hasread = true;
    }
  }
  Serial.println(finalString);
  return finalString;

}
//======================================================================================

void loop()
{
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  //Read the first line of the req.
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  if (req.indexOf("NEWBUTTON") > 0 || newButtonInitiated == true) {


    newButtonInitiated = true;
    digitalWrite(15, HIGH);

    delay(50);
    if (SD.exists("NAME.TXT") ) {
      Serial.println("have all 3 codes, time for average");
      sendRequest = "Code3";
      newButtonInitiated = false;
    }
    else if ( SD.exists("NEW2.TXT") ) {

      sender("NEW3.TXT", "CODE.TXT");
      Serial.println("wrote to 3");
      sendRequest = "Code2";

    }

    else if ( SD.exists("NEW1.TXT") ) {
      sender("NEW2.TXT" , "CODE.TXT");
      Serial.println("wrote to 2");
      sendRequest = "Code1";
    }
    else if (sendRequest != "Code2"){
      sender("NEW1.TXT" , "CODE.TXT");
      Serial.println("wrote to 1");
      sendRequest = "Code0";
    }
    digitalWrite(15, LOW);
  }

  if (req.indexOf("NEWNAME") > 0) {
    
    digitalWrite(15, HIGH);
    Serial.println("BigBigMoney");
    delay(50);
    sender(memLocationReader("NAME.TXT"), requestDecoder(req));
    SD.remove("NAME.TXT");
    Serial.println("it works");
    sendRequest = "Code0";
    newButtonInitiated = false;

    digitalWrite(15, LOW);
  }


  String s = "HTTP/1.1 200 OK\r\n";
  s += "Content Type: text/html\r\n\r\n";
  s += "<!DOCTYPE HTML>\r\n<html>\r\n";
  s += sendRequest;

  s += "</html>\n";
  //send response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disconnected");


}




//======================================================================================

void IRSender(String str) {
  digitalWrite(15, HIGH);
  myFile = SD.open(str);

  if (myFile) {
    int counter = 0;
    if ( !myFile.available() ) {
      Serial.println("#version is empty");
      str.replace("#", "");
      IRSender(str);
      return;
    }
    while ( myFile.available() ) {
      char incomingInt = 'A';
      incomingInt = myFile.read();
      Serial.println(incomingInt);

      if ( !(incomingInt == ',') && !(incomingInt == '@') ) {
        //checks for commas and end of string int (@)
        Serial.println("comma");

        if ( !((int)incomingInt >= 48) || !((int)incomingInt <= 57 )) {
          //Checks for incoming int to be between and to be less than 9
          Serial.println("wrong");

          if ( str.indexOf('#') >= 0 ) {
            //Checks if it is reading from the # file
            str.replace("#", "");
            Serial.println("# version doesn't work, trying original");
            IRSender(str);
            return;
          }
          else {
            Serial.println("error in original file");
          }
        }
      }


      if ( incomingInt == ',' || incomingInt == '@' ) {
        finalValArray[counter] = (byte)(tempIncomingInt);
        tempIncomingInt = 0;
        counter++;
      }

      else if (incomingInt == ' ') {
        continue;
      }

      else {
        tempIncomingInt = (tempIncomingInt * 10) + (incomingInt - 0x30);
      }

    }

    for (int i = 0; i < counter; i++) {

      if ( finalValArray[i] > 70 ) {
        str.replace("#", "");
        Serial.println("# version doesn't work, too long of a pulse inside");
        IRSender(str);
        return;
      }
      myFile.close();
      digitalWrite(15, LOW);
      Serial.println(finalValArray[i]);

      if (1 > finalValArray[i]) {
        break;
      }

      if (i % 2 == 0) {
        pulseIR(finalValArray[i] * 100);
      }

      else {
        delayMicroseconds(finalValArray[i] * 100);
      }

    }
    Serial.println("sent");


  }
}
//======================================================================================

void pulseIR(long microsecs) {

  cli();

  while (microsecs > 0) {
    digitalWrite(IRledPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(IRledPin, LOW);
    delayMicroseconds(10);

    microsecs -= 26;
  }

  sei();
}



