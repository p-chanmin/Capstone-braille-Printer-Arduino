
#include <SoftwareSerial.h> //시리얼통신 라이브러리 호출
#include <AccelStepper.h> 

#define BlueRX 2          // 블루투스 Rx핀
#define BlueTX 3          // 블루투스 Tx핀
#define MainMotorDIR 4    // 솔레노이드 위치 제어하는 모터 DIR핀
#define MainMotorSTEP 5   // 솔레노이드 위치 제어하는 모터 STEP핀
#define PageMotorDIR 6    // 페이지 위치 제어하는 모터 DIR핀
#define PageMotorSTEP 7   // 페이지 위치 제어하는 모터 STEP핀
#define SOLENOID 8        // 솔레노이드 제어 핀
#define INIT_SWITCH 9     // 엔드스탑 스위치
#define MainMotorEN 10    // 메인 모터 Enable핀
#define PageMotorEN 11    // 페이지 모터 Enable핀
#define MS_PIN 12        // 메인모터 마이크로 스테핑 MS1, MS2, MS3핀


SoftwareSerial blueSerial(BlueTX, BlueRX);  //시리얼 통신을 위한 객체선언

// 64개 점의 절대 위치 값
int dot_point[] = {
  0, 188, 437, 625, 874, 1062, 1311, 1499, 1748, 1936, 2185, 2373, 2622, 2810, 3059, 3247, 3496, 3684, 3933, 4121, 4370, 4558, 4807, 4995, 5244, 5432, 5681, 5869, 6118, 6306, 6555, 6743, 6992, 7180, 7429, 7617, 7866, 8054, 8303, 8491, 8740, 8928, 9177, 9365, 9614, 9802, 10051, 10239, 10488, 10676, 10925, 11113, 11362, 11550, 11799, 11987, 12236, 12424, 12673, 12861, 13110, 13298, 13547, 13735
  };

// 초기화 위치에서 첫번째 위치까지 모터 이동 상수
int toZeroPorint = 960;

// (인쇄 시작시, 줄간격, 칸간격, 인쇄 종료시)
int PRINT_START = 90;  // 시작 시 > 인쇄용지 끼워져 있는 상태에서 첫번쨰 라인 위치까지
int PRINT_END = 150;    // 마지막 줄 인쇄 후 용지가 빠질 때까지
int PRINT_END_VALUE = 10; // 남은 줄 * value를 통해 용지가 빠질 때까지
int PRINT_LINE = 5;  // 줄 간격
int PRINT_BLOCK = 12; // 칸 간격

// 위치 값
int current_point = 0;

int MainMotorSpeed = 4500;  //메인 모터 속도값
int MainMotorAcceleration = 30000;  //메인 모터 가속도값
int PageMotorSpeed = 1600;  //페이지 모터 속도값
 
// 메인모터 객체 생성
AccelStepper stepper(AccelStepper::DRIVER, MainMotorSTEP, MainMotorDIR);

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

  // 메인, 페이지 모터 LOW 설정
  digitalWrite(MainMotorSTEP,LOW);
  digitalWrite(PageMotorSTEP,LOW);
  // 메인, 페이지 모터 시계 방향 설정
  // 정면에서 봤을 때 LOW가 반시계, HIGH가 시계
  digitalWrite(MainMotorDIR,LOW);
  digitalWrite(PageMotorDIR,LOW);

  // 메인 모터, 페이지 모터 Enable 설정
  pinMode(MainMotorEN,OUTPUT);
  pinMode(PageMotorEN,OUTPUT);
  // 메인 모터, 페이지 모터 비활성화
  digitalWrite(MainMotorEN,HIGH);
  digitalWrite(PageMotorEN,HIGH);
  // MS2 PIN 설정
  pinMode(MS_PIN,OUTPUT);
  digitalWrite(MS_PIN,HIGH);

  // 엔드스탑 스위치 설정
  pinMode(INIT_SWITCH, INPUT);

  // 모터 속도 및 가속도 설정
  stepper.setMaxSpeed(MainMotorSpeed);
  stepper.setAcceleration(MainMotorAcceleration);

  // 초기 위치 설정
  stepper.setCurrentPosition(0);

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
      int p = receivedData.substring(2).toInt();
      Serial.println("Test Zero Point :" + String(p));
      TestZeroPoint(p);
    }
    else if(code == 'Z'){
      Serial.println("Code Z");
      ZeroNotify();
    }
    else if(code == 'P'){ // 프린트 데이터가 들어왔을 때

      // !! 솔레노이드 제로 포인트 이동

      PrintStart(receivedData);
    }
    
  }
  if (Serial.available()) {     
    String msg = Serial.readString();    

    if(msg == "s"){
      Serial.println("SOLENOID ON");
      delay(5);
      digitalWrite(SOLENOID, HIGH);
      delay(30);
      digitalWrite(SOLENOID, LOW);
      delay(5);
    }
    else if(msg == "switch"){
      Serial.println(getSwitch());
    }
    else if(msg == "init"){
      Serial.println("Init Main Motor");
      InitMainMotor();
    }
    else if(msg == "zero"){
      Serial.println("Go Zero Point");
      GoToZeroPoint();
    }
    else if(msg[0] == 'I'){
      int p = msg.substring(2).toInt();
      Serial.println("Test Zero Point :" + String(p));
      TestZeroPoint(p);
    }
    else if(msg == "test"){
      stepper.setCurrentPosition(0);  // 현재 위치를 0으로 함
      for(int i = 0; i < 64 ; i++){
        MainMotorMoveFromZeroPoint(dot_point[i]);
        delay(10);
        digitalWrite(SOLENOID, HIGH);
        delay(30);
        digitalWrite(SOLENOID, LOW);
        delay(5);
      }
    }
    else if(msg[0] == 'm'){
      int p = msg.substring(1).toInt();
      MainMotorMoveFromZeroPoint(dot_point[p]);
    }
    else if(msg == "testv"){
      stepper.setCurrentPosition(0);  // 현재 위치를 0으로 함
      for(int i = 0; i < 78 ; i++){
        if( i % 3 == 0 ){
          PageMotorMove(PRINT_BLOCK);
        }
        else{
          PageMotorMove(PRINT_LINE);
        }
        delay(300);

        delay(5);
        digitalWrite(SOLENOID, HIGH);
        delay(30);
        digitalWrite(SOLENOID, LOW);
        delay(5);

        delay(300);
      }
    }
    else if(msg == "Z"){
      ZeroNotify();
    }
    else{
      Serial.println("Page Motor Move :" + String(msg.toInt()));
      PageMotorMove(msg.toInt());
    }
    // blueSerial.write();  //시리얼 모니터 내용을 블루투스 측에 WRITE
  }
}

// 스위치 상태 확인
// 1 : OFF, 0 : ON
int getSwitch(){
  return digitalRead(INIT_SWITCH);
}

// 솔레노이드 HIGH값
void Solenoid_ON(){
  digitalWrite(SOLENOID, HIGH);
}
// 솔레노이드 LOW값
void Solenoid_OFF(){
  digitalWrite(SOLENOID, LOW);
}



// 페이지 모터제어 (인쇄 시작시, 줄간격, 칸간격, 인쇄 종료시)
void PageMotorMove(int cnt){
  // 패이지 모터 활성화
  digitalWrite(PageMotorEN,LOW);

  // 시계방향 회전
  digitalWrite(PageMotorDIR,HIGH); 

  for(int i = 0; i < cnt; i++){
    digitalWrite(PageMotorSTEP,HIGH);
    delayMicroseconds(PageMotorSpeed);
    digitalWrite(PageMotorSTEP,LOW);
    delayMicroseconds(PageMotorSpeed);
  }

  digitalWrite(PageMotorDIR,LOW); 
  
  // 페이지 모터 비활성화
  digitalWrite(PageMotorEN,HIGH);

    // 페이지 모터 비활성화
  digitalWrite(PageMotorEN,HIGH);

}

void TestZeroPoint(int p){
  
  // 입력받은 값으로 toZeroPoint 값 변경
  toZeroPorint = p;

  ZeroNotify();

  InitMainMotor();
  delay(500);

  // 메인모터 이동
  MainMotorMoveFromZeroPoint(toZeroPorint);

  delay(500);
  Solenoid_ON();
  delay(1000);
  Solenoid_OFF();
  delay(500);

  InitMainMotor();

  TestEndNotify();
}

// 솔레노이드 zero_point이동 함수(첫번째 위치로)
void GoToZeroPoint(){

  Serial.println("Go To Zero Point");

  InitMainMotor();
  delay(500);

  MainMotorMoveFromZeroPoint(toZeroPorint);

    // 메인 모터 비활성화
  digitalWrite(MainMotorEN,HIGH);
  stepper.setCurrentPosition(0);  // 현재 위치를 0으로 함

}

// 솔레노이드 초기화 함수(스위치 닿는 위치로)
void InitMainMotor(){

  Serial.println("Init Main Motor");

  // 메인 모터 활성화
  digitalWrite(MainMotorEN,LOW);

  if( 0 == getSwitch() ){
    stepper.setCurrentPosition(0);
    stepper.moveTo(200);
    stepper.runToPosition();
    delay(500);
    while( 1 == getSwitch() ){
      stepper.setSpeed(-5000);
      stepper.runSpeed();
    }
  }
  else{
    while( 1 == getSwitch() ){
      stepper.setSpeed(-5000);
      stepper.runSpeed();
    }
  }
  stepper.stop();

  stepper.setCurrentPosition(0);

  // 메인 모터 비활성화
  digitalWrite(MainMotorEN,HIGH);
  
}

// 절대 위치를 통해 메인 모터 제어
void MainMotorMoveFromZeroPoint(int p){

  // 메인 모터 활성화
  digitalWrite(MainMotorEN,LOW);

  stepper.moveTo(p);
  stepper.runToPosition();

  // 메인 모터 비활성화
  digitalWrite(MainMotorEN,HIGH);

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

  // 솔레노이드 설정한 zero_point 이동 ( 초기화 포함 )
  GoToZeroPoint();

  current_point = 0;  // 현재 위치를 0으로 함
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

      if(total_lines % 78 == 0){  // 새로운 페이지 인쇄 시작 시
        // 페이지 모터 인쇄 용지 첫 라인으로 이동
        Serial.println("PageMotor >> Print Start");
        PageMotorMove(PRINT_START);
        delay(100);
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
      Serial.println("printing...");
      Serial.println("dataArray : ");
      for (int i = 0; i < lines; i++) {
        if(total_lines % 2 == 0){
          // 정방향 인쇄
          Serial.print("forward >> ");
          for (int j = 0; j < 64; j++) {
            Serial.print(dataArray[i][j]);
            Serial.print(" ");
            if(dataArray[i][j] == 1){ // 찍어야 하는 위치면
              // 인덱스 위치로 이동
              MainMotorMoveFromZeroPoint(dot_point[j]);
              delay(10);
              Solenoid_ON();
              delay(30);
              Solenoid_OFF();
              delay(10);
            }
          }
        }
        else{
          // 반대로 인쇄
          Serial.print("reverse >> ");
          for (int j = 63; j >= 0; j--) {
            Serial.print(dataArray[i][j]);
            Serial.print(" ");
            if(dataArray[i][j] == 1){ // 찍어야 하는 위치면
              // 인덱스 위치로 이동
              MainMotorMoveFromZeroPoint(dot_point[j]);
              delay(10);
              Solenoid_ON();
              delay(30);
              Solenoid_OFF();
              delay(10);
            }
          }
        }

        Serial.println();
        // 인쇄 정보 notify 송신
        total_lines++;
        LineNotify(total_lines);
        delay(400);
        // 페이지 모터 인쇄 용지 이동(줄간격)
        if(i != lines - 1){ // 마지막 라인일 경우에는 이동 하지 않음
          Serial.println("PageMotor >> Print Line");
          PageMotorMove(PRINT_LINE);
          delay(100);
        }
        // // 시연용
        // GoToZeroPoint();
        
      }
      // 페이지 모터 인쇄 용지 이동(칸간격)
      Serial.println("PageMotor >> Print Block");
      PageMotorMove(PRINT_BLOCK);
      delay(100);

      // 추가 데이터 필요 notify
      if(received_size < total_size){
        DataNotify();
      }
      
      if(total_lines % 78 == 0){  // 한페이지 인쇄 완료 시
        // 페이지 모터 인쇄 용지 제거
        Serial.println("PageMotor >> Print End : " + String(PRINT_END));
        PageMotorMove(PRINT_END);
        delay(100);
      }

    }
  }
  
  Serial.println("Complete Print");
  CompleteNotify(print_id);
  // 0으로 이동
  MainMotorMoveFromZeroPoint(0);
  delay(300);
  // 솔레노이드 초기화
  InitMainMotor();
  // 솔레노이드 OFF
  Solenoid_OFF();
  // 페이지 모터 인쇄 종료 시
  if(total_lines % 78 != 0){
    Serial.println("PageMotor >> Print End : " + String(PRINT_END_VALUE * ((78 - total_lines) % 78)));
    PageMotorMove(PRINT_END_VALUE * ((78 - total_lines) % 78));
  }

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
void CompleteNotify(int print_id){
  String noti = "Complete_Print|" + String(print_id) ;
  char notiArray[noti.length() + 1];
  noti.toCharArray(notiArray, sizeof(notiArray));
  blueSerial.write(notiArray);
}
void ZeroNotify(){
  String noti = "Init";
  noti += toZeroPorint;
  char notiArray[noti.length() + 1];
  noti.toCharArray(notiArray, sizeof(notiArray));
  blueSerial.write(notiArray);
}
void TestEndNotify(){
  String noti = "TestEnd";
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