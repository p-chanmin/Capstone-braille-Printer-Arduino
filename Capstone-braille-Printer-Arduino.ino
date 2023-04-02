
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
  // 블루투스를 통해 데이터 수신 받는 경우
  if (blueSerial.available()) {
    String receivedData = blueSerial.readString();
    while(blueSerial.available()){
      receivedData += blueSerial.readString();
    }
    Serial.println("receivedData");
    Serial.println(receivedData);

    char code = getCode(receivedData);

    if(code == 'I'){  // 조정값 설정
      Serial.println("Code I");
    }
    else if(code == 'P'){ // 프린트 데이터가 들어왔을 때
      PrintStart(receivedData);
    }
    
  }
  // if (Serial.available()) {     
  //   String msg = Serial.readString();    

  //   if(msg == "mf"){
  //     Serial.println("m front");
  //     MainMotorMove('f');
  //   }
  //   else if(msg == "mb"){
  //     Serial.println("m back");
  //     MainMotorMove('b');
  //   }
  //   else if(msg == "pf"){
  //     Serial.println("p front");
  //     PageMotorMove('f');
  //   }
  //   else if(msg == "pb"){
  //     Serial.println("p back");
  //     PageMotorMove('b');
  //   }
  //   else if(msg[0] == 'm'){
  //     Serial.println("m move : " + msg.substring(1));
  //     MainMotorMove(msg.substring(1).toInt());
  //   }
  //   else if(msg[0] == 'p'){
  //     Serial.println("p move : " + msg.substring(1));
  //     MainMotorMove(msg.substring(1).toInt());
  //   }
  //   else if(msg == "data"){
  //     String myData = "Hello, world!";
  //     char myDataArray[myData.length() + 1];
  //     myData.toCharArray(myDataArray, sizeof(myDataArray));
  //     blueSerial.write(myDataArray);
  //   }
    // blueSerial.write();  //시리얼 모니터 내용을 블루투스 측에 WRITE
  // }
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

/// @brief 수신받은 데이터에서 구분코드를 반환하는 함수
/// @param data 수신받은 데이터
/// @return 구분 코드
char getCode(String data){
    return data[0];
}

void PrintStart(String receivedData){
  int first = receivedData.indexOf('|');
  int last = receivedData.lastIndexOf('|');
  
  int print_id = receivedData.substring(first+1, last).toInt();
  int total_size = receivedData.substring(last+1, receivedData.length()).toInt();

  Serial.print(receivedData);

  int received_size = 0;  // 전달 받은 데이터 크기
  int total_lines = 0;
  
  DataNotify();

  while(received_size < total_size){
    // 데이터 수신
    if(blueSerial.available()){
      String brailleData = blueSerial.readString();
      while(blueSerial.available()){
        brailleData += blueSerial.readString();
      }

      // 받은 데이터 크기 갱신
      received_size += brailleData.length();

      // 라인 계산
      int lines = countNewlines(brailleData);

      // 점자 데이터 리스트 초기화
      bool dataArray[lines][64];
      for (int i = 0; i < lines; i++) {
          for (int j = 0; j < 64; j++) {
            dataArray[i][j] = 0;
        }
      }

      // 점자 데이터 리스트화
      splitBrailleData(brailleData, dataArray);

      // 점자 데이터 출력
      Serial.println("dataArray : ");
      for (int i = 0; i < lines; i++) {
        for (int j = 0; j < 64; j++) {
          Serial.print(dataArray[i][j]);
          Serial.print(" ");
        }
        Serial.println();
      }

      // 인쇄 용지 이동(줄간격)

      // 추가 데이터 필요 notify
      Serial.println("printing...");
      total_lines++;
      LineNotify(total_lines);
      delay(400);
      total_lines++;
      LineNotify(total_lines);
      delay(400);
      total_lines++;
      LineNotify(total_lines);
      delay(400);

      if(received_size < total_size){
        DataNotify();
      }
      
    }
    // 인쇄 용지 이동(칸간격)
  }
  Serial.println("Complete Print");
  CompleteNotify();

}
void LineNotify(int line){
  String noti = "Line";
  noti += line;
  char notiArray[noti.length() + 1];
  noti.toCharArray(notiArray, sizeof(notiArray));
  blueSerial.write(notiArray);
}
void DataNotify(){
  String noti = "Send_Data";
  char notiArray[noti.length() + 1];
  noti.toCharArray(notiArray, sizeof(notiArray));
  blueSerial.write(notiArray);
}
void CompleteNotify(){
  String noti = "Complete_Print";
  char notiArray[noti.length() + 1];
  noti.toCharArray(notiArray, sizeof(notiArray));
  blueSerial.write(notiArray);
}

// 점자 데이터 라인 갯수 계산
int countNewlines(String data) {
  int count = 0;
  for (int i = 0; i < data.length(); i++) {
    if (data[i] == '+') {
      count++;
    }
  }
  return count+1;
}


// 점자 데이터 리스트화 함수
void splitBrailleData(String data, bool dataArray[][64]) {
  int row = 0;
  int col = 0;
  for (int i = 0; i < data.length(); i++) {
    char c = data[i];
    if (c == '+') {
      row++;
      col = 0;
    } else {
      if(c == '0'){
        dataArray[row][col] = 0;
      }
      else if(c == '1'){
        dataArray[row][col] = 1;
      }
      col++;
    }
  }
}