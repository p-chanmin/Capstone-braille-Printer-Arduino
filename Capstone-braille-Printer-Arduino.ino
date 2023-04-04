
#include <SoftwareSerial.h> //시리얼통신 라이브러리 호출

#define BlueRX 2          // 블루투스 Rx핀
#define BlueTX 3          // 블루투스 Tx핀
#define MainMotorDIR 4    // 솔레노이드 위치 제어하는 모터 DIR핀
#define MainMotorSTEP 5   // 솔레노이드 위치 제어하는 모터 STEP핀
#define PageMotorDIR 6    // 페이지 위치 제어하는 모터 DIR핀
#define PageMotorSTEP 7   // 페이지 위치 제어하는 모터 STEP핀
#define SOLENOID 8        // 솔레노이드 제어 핀


SoftwareSerial blueSerial(BlueTX, BlueRX);  //시리얼 통신을 위한 객체선언

// 64개 점의 절대 위치 값
int dot_point[] = {0, 7, 14, 21, 28, 35, 42, 49, 56, 63, 70, 77, 84, 91, 98, 105, 112, 119, 126, 133, 140, 147, 154, 161, 168, 175, 182, 189, 196, 203, 210, 217, 225, 232, 239, 246, 253, 260, 267, 274, 281, 288, 295, 302, 309, 316, 323, 330, 337, 344, 351, 358, 365, 372, 379, 386, 393, 400, 407, 414, 421, 428, 435, 442, 449};

// 위치 값
int current_point = 0;

int MainMotorSpeed = 1400;  //메인 모터 속도값
int PageMotorSpeed = 1400;  //페이지 모터 속도값
 
void setup() 
{
  Serial.begin(9600);   //시리얼모니터
  blueSerial.begin(9600); //블루투스 시리얼

  // 솔레노이드 디지털핀 Output 초기화
  pinMode(SOLENOID,OUTPUT);
  digitalWrite(SOLENOID, LOW);

  // 메인모터 디지털핀 Output 초기화
  pinMode(MainMotorDIR,OUTPUT);
  pinMode(MainMotorSTEP,OUTPUT);
  // 페이지모터 디지털핀 Output 초기화
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
  if (Serial.available()) {     
    String msg = Serial.readString();    

    if(msg == "mf"){
      Serial.println("m front");
      MainMotorTurn('f');
    }
    else if(msg == "mb"){
      Serial.println("m back");
      MainMotorTurn('b');
    }
    else if(msg == "pf"){
      Serial.println("p front");
      PageMotorTurn('f');
    }
    else if(msg == "pb"){
      Serial.println("p back");
      PageMotorTurn('b');
    }
    else if(msg[0] == 'm'){
      Serial.println("m move : " + msg.substring(1));
      MainMotorMove(msg.substring(1).toInt());
    }
    else if(msg[0] == 'p'){
      Serial.println("p move : " + msg.substring(1));
      PageMotorMove(msg.substring(1).toInt());
    }
    else if(msg == "son"){
      Serial.println("SOLENOID ON");
      Solenoid_ON();
    }
    else if(msg == "soff"){
      Serial.println("SOLENOID OFF");
      Solenoid_OFF();
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

// 솔레노이드 HIGH값
void Solenoid_ON(){
  digitalWrite(SOLENOID, HIGH);
}
// 솔레노이드 LOW값
void Solenoid_OFF(){
  digitalWrite(SOLENOID, LOW);
}

// 솔레노이드 초기화 함수(스위치 닿는 위치로)

// 솔레노이드 zero_point이동 함수(첫번째 위치로)

// 페이지 모터제어 (인쇄 시작시, 줄간격, 칸간격, 인쇄 종료시)

// 절대 위치를 통해 메인 모터 제어
void MainMotorMoveFromZeroPoint(int p){

  int move = current_point - p;

  if(move < 0){ // 시계방향 회전
    digitalWrite(MainMotorDIR,LOW);
    for(int i = 0; i > move; i--){
      digitalWrite(MainMotorSTEP,HIGH);
      delayMicroseconds(MainMotorSpeed);
      digitalWrite(MainMotorSTEP,LOW);
      delayMicroseconds(MainMotorSpeed);
    }
  }
  else if(move > 0){ // 반시계방향 회전
    digitalWrite(MainMotorDIR,HIGH);
    for(int i = 0; i < move; i++){
      digitalWrite(MainMotorSTEP,HIGH);
      delayMicroseconds(MainMotorSpeed);
      digitalWrite(MainMotorSTEP,LOW);
      delayMicroseconds(MainMotorSpeed);
    }
  }
  
  current_point = p;
}

/// @brief 수신받은 데이터에서 구분코드를 반환하는 함수
/// @param data 수신받은 데이터
/// @return 구분 코드
char getCode(String data){
    return data[0];
}

/// @brief 프린트 전체 과정의 함수
/// @param receivedData 전달받은 프린트 시작 코드 데이터(ex: P|인쇄번호|데이터크기)
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

      // !! 솔레노이드 ON
      Solenoid_ON();
      delay(1000);
      // !! 솔레노이드 제로 포인트 이동(current_point = 0 초기화)
      // !! 페이지 모터 인쇄 시작시
      
      // 점자 데이터 출력
      Serial.println("printing...");
      Serial.println("dataArray : ");
      for (int i = 0; i < lines; i++) {
        for (int j = 0; j < 64; j++) {
          Serial.print(dataArray[i][j]);
          Serial.print(" ");
          if(dataArray[i][j] == 1){ // 찍어야 하는 위치면
            // 인덱스 위치로 이동
            MainMotorMoveFromZeroPoint(dot_point[j]);
            delay(300);
            Solenoid_OFF();
            delay(300);
            Solenoid_ON();
            delay(300);
          }
        }
        Serial.println();
        // 인쇄 정보 notify 송신
        total_lines++;
        LineNotify(total_lines);
        delay(400);
        // !! 페이지 모터 인쇄 용지 이동(줄간격)
      }

      // !! 페이지 모터 인쇄 용지 이동(칸간격)

      // 추가 데이터 필요 notify
      if(received_size < total_size){
        DataNotify();
      }
      
    }
  }
  // !! 페이지 모터 인쇄 종료 시
  Serial.println("Complete Print");
  CompleteNotify();
  // !! 솔레노이드 초기화
  // !! 솔레노이드 OFF
  Solenoid_OFF();

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