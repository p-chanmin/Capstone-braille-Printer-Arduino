
#include <SoftwareSerial.h> //시리얼통신 라이브러리 호출

#define BlueRX 2          // 블루투스 Rx핀
#define BlueTX 3          // 블루투스 Tx핀
#define MainMotorDIR 4    // 솔레노이드 위치 제어하는 모터 DIR핀
#define MainMotorSTEP 5   // 솔레노이드 위치 제어하는 모터 STEP핀
#define PageMotorDIR 6    // 페이지 위치 제어하는 모터 DIR핀
#define PageMotorSTEP 7   // 페이지 위치 제어하는 모터 STEP핀



SoftwareSerial blueSerial(BlueTX, BlueRX);  //시리얼 통신을 위한 객체선언

int MainMotorSpeed = 1400;  //메인 모터 속도값
int PageMotorSpeed = 1400;  //페이지 모터 속도값
 
void setup() 
{
  Serial.begin(9600);   //시리얼모니터
  blueSerial.begin(9600); //블루투스 시리얼

  pinMode(MainMotorDIR,OUTPUT);
  pinMode(MainMotorSTEP,OUTPUT);
  pinMode(PageMotorDIR,OUTPUT);
  pinMode(PageMotorSTEP,OUTPUT);

  // 메인, 페이지 모터 시계 방향 설정
  digitalWrite(MainMotorDIR,LOW);
  digitalWrite(PageMotorDIR,LOW);

}

void loop()
{
  if (blueSerial.available()) {       
    Serial.write(blueSerial.read());  //블루투스측 내용을 시리얼모니터에 출력
  }
  if (Serial.available()) {     
    String msg = Serial.readString();    

    if(msg == "mf"){
      Serial.println("m front");
      MainMotorMove('f');
    }
    else if(msg == "mb"){
      Serial.println("m back");
      MainMotorMove('b');
    }
    else if(msg == "pf"){
      Serial.println("p front");
      PageMotorMove('f');
    }
    else if(msg == "pb"){
      Serial.println("p back");
      PageMotorMove('b');
    }
    else if(msg[0] == 'm'){
      Serial.println("m move : " + msg.substring(1));
      MainMotorMove(msg.substring(1).toInt());
    }
    else if(msg[0] == 'p'){
      Serial.println("p move : " + msg.substring(1));
      MainMotorMove(msg.substring(1).toInt());
    }
    // blueSerial.write();  //시리얼 모니터 내용을 블루투스 측에 WRITE
  }
}

// 메인 모터 제어 함수
void MainMotorMove(int cnt){
  for(int i = 0; i < cnt; i++){
    digitalWrite(MainMotorSTEP,HIGH);
    delayMicroseconds(MainMotorSpeed);
    digitalWrite(MainMotorSTEP,LOW);
    delayMicroseconds(MainMotorSpeed);
  }
}
// 메인 모터 방향 제어 함수
void MainMotorTurn(char c){
  if(c == 'f'){
    digitalWrite(MainMotorDIR,LOW);
  }
  else if(c == 'b'){
    digitalWrite(MainMotorDIR,HIGH);
  }
}

// 페이지 모터 제어 함수
void PageMotorMove(int cnt){
  for(int i = 0; i < cnt; i++){
    digitalWrite(PageMotorSTEP,HIGH);
    delayMicroseconds(PageMotorSpeed);
    digitalWrite(PageMotorSTEP,LOW);
    delayMicroseconds(PageMotorSpeed);
  }
}
// 메인 모터 방향 제어 함수
void PageMotorTurn(char c){
  if(c == 'f'){
    digitalWrite(PageMotorDIR,LOW);
  }
  else if(c == 'b'){
    digitalWrite(PageMotorDIR,HIGH);
  }
}