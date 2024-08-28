#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define RST_PIN         9          
#define SS_PIN          10 //RC522卡上的SDA

MFRC522 mfrc522;   // 建立MFRC522實體

char *reference;
bool status = false,donothing = false;
byte uid[]={0x82, 0x1E, 0xB1, 0x1B};  //這是我們指定的卡片UID，可由讀取UID的程式取得特定卡片的UID，再修改這行
const int buttonPin = 2;     // the number of the pushbutton pin
const int ledr =  5;
const int ledg =  4;

int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

void setup()
{
  Serial.begin(9600);   
  status = false;
  donothing = false;
  lcd.begin();
  
  lcd.backlight();
  SPI.begin();
  pinMode(7,OUTPUT);
  pinMode(buttonPin,INPUT);
  pinMode(ledr,OUTPUT);
  pinMode(ledg,OUTPUT);
  digitalWrite(7,HIGH);//relay
  digitalWrite(ledr,LOW);
  digitalWrite(ledg,HIGH);
  mfrc522.PCD_Init(SS_PIN, RST_PIN); // 初始化MFRC522卡
  Serial.print(F("Reader "));
  Serial.print(F(": "));
  mfrc522.PCD_DumpVersionToSerial(); // 顯示讀卡設備的版本
}

void loop() {

  buttonState = digitalRead(buttonPin);
  
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter++;
      Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter);
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  // save the current state as the last state, for next time through the loop
  lastButtonState = buttonState;


  // turns on the LED every four button pushes by checking the modulo of the
  // button push counter. the modulo function gives you the remainder of the
  // division of two numbers:
  if (digitalRead(buttonPin)==HIGH & buttonPushCounter % 2 == 0) {
    while(digitalRead(buttonPin)==HIGH)
    digitalWrite(ledg, HIGH);
    digitalWrite(ledr, LOW);
    status = false;
    donothing = false;
  } else if(digitalRead(buttonPin)==HIGH & buttonPushCounter % 2 == 1){
    while(digitalRead(buttonPin)==HIGH)
    digitalWrite(ledg, LOW);
    digitalWrite(ledr, HIGH);
    status = true;
    donothing = false;
  }
  
  digitalWrite(7,HIGH);
  /*
  lcd.setCursor(2, 0); // (colum, row)從第一排的第三個位置開始顯示
  lcd.print("Hello World!"); 
  lcd.setCursor(2, 1); // (colum,row)從第二排第三格位置開始顯示
  lcd.print("Crazy Maker!");
  */
  lcd.setCursor(0, 0);
  lcd.print("reading...");
  // 檢查是不是偵測到新的卡
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      // 顯示卡片的UID
      lcd.clear();
      lcd.begin();
      lcd.backlight();
      lcd.print("Card UID:");
      //lcd.print("UID tag :");
      lcd.setCursor(0,1);
      
      String content= "";
      byte letter;
      String word;
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
          // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
          // Serial.print(mfrc522.uid.uidByte[i], HEX);
          content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
          word = String(mfrc522.uid.uidByte[i], HEX);
          word.toUpperCase();
          lcd.print(word);
          lcd.print(" ");
          Serial.print(word);
      }
      delay(1000);

      lcd.clear();
      lcd.begin();
      lcd.backlight();
      lcd.setCursor(0, 0);
      //lcd.print(); // 顯示卡片的UID
      // Serial.println();
      //Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
      //Serial.println(mfrc522.PICC_GetTypeName(piccType));  //顯示卡片的類型
      
      //把取得的UID，拿來比對我們指定好的UID
      bool they_match = true; // 初始值是假設為真 
      for ( int i = 0; i < 4; i++ ) { // 卡片UID為4段，分別做比對
        if ( uid[i] != mfrc522.uid.uidByte[i] ) { 
          they_match = false; // 如果任何一個比對不正確，they_match就為false，然後就結束比對
          break; 
        }
      }
      
      //在監控視窗中顯示比對的結果
      if(they_match){
        lcd.print(F("Access Granted!"));
        if(!status&!donothing) //green mode
        {
          digitalWrite(7,LOW);
          delay(100);
          digitalWrite(7,HIGH);
          status = true;
          donothing = true;
          digitalWrite(ledg, LOW);
          digitalWrite(ledr, LOW);
        }else if(status&!donothing){ //red mode
          digitalWrite(7,LOW);
          delay(8000);
          digitalWrite(7,HIGH);
          status = false;
          donothing = false;
          digitalWrite(ledg, HIGH);
          digitalWrite(ledr, LOW); 
        }
        // digitalWrite(7,LOW);
        // delay(500);
        // digitalWrite(7,HIGH);
      }else{
        lcd.print(F("Access Denied!"));

      }
      mfrc522.PICC_HaltA();  // 卡片進入停止模式
      delay(1000);
      lcd.clear();
      lcd.begin();
      lcd.backlight();
      lcd.setCursor(0, 0);
    }
}

/**
 * 這個副程式把讀取到的UID，用16進位顯示出來
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}