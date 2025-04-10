//PART 3
//스케쥴링만 안함(아마도)
// case 번호 쪽 참고할 것. 맞춰가야함
//앱으로 조종하는건 toggle X, 리모컨 조종 필요 시 토글 o
//가스값 보내는거
//후드는 only 리모컨-O
//밸브 onoff 빼버려-O

#include <SoftwareSerial.h>   //소프트웨어 시리얼 라이브러리 추가(앱)
#include <U8glib.h>           //u8g 라이브러리 추가 ( OLED 화면 )
#include <Servo.h>            //서보모터 라이브러리 추가

Servo servo3;                  //부엌 창문 서보모터 객체 생성
Servo servo4;                  //가스 밸브 서보모터 객체 생성

#define TXD 10                 //TXD, 10번 핀으로 설정
#define RXD 11                 //RXD, 11번 핀으로 설정
#define ECHO3 6                 //ECHO, 6번
#define TRIG3 8                 //TRIG, 8번
#define LED_room2 2              //방2 LED 제어, 2번 핀
#define TOUCH2 4                  //터치 제어, 4번 핀
#define SERVO_WIN 3               //서보모터 각도 제어, 3번 핀으로 설정
#define SERVO_GAS 5               //서보모터 각도 제어, 5번 핀으로 설정

#define HOOD_MOTOR 7                //DC 모터 제어, 7번 핀
#define PIEZO_room2 9                   //피에조 제어 (방2)
#define GAS_OUT A0                //가스센서 아날로그 출력, A0핀



U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);      //SSD1306 128X64 I2C 규격 선택  ( OLED 화면 )

SoftwareSerial btSerial(TXD, RXD);                //소프트웨어 시리얼 mySerial 객체 선언(앱)


bool hoodState = false;                       //후드 상태 저장 변수
bool servo3State = false;                      //서보모터 상태 저장 변수
static bool carDetected = false;              //차량 감지 상태 저장


unsigned long displayStartTime = 0;           // 화면 표시 시작 시간
//int value = 0;                          //터치센서 스위칭 횟수



void setup() {
  Serial.begin(9600);

  btSerial.begin(9600);     //소프트웨어 시리얼 동기화(앱)

  servo3.attach(SERVO_WIN);    //부엌 창문 서보 모터 제어 위한 핀 설정
  servo4.attach(SERVO_GAS);    //가스밸브 서보 모터 제어 위한 핀 설정
  servo3.write(175);        //처음 창문 위치 설정- 수정 필요
  servo4.write(100);        //처음에 가스 배관과 일직선상에 위치

  pinMode(TRIG3, OUTPUT);
  pinMode(ECHO3, INPUT);
  pinMode(TOUCH2, INPUT);
  pinMode(HOOD_MOTOR, OUTPUT);

  pinMode(PIEZO_room2, OUTPUT);
  pinMode(LED_room2, OUTPUT);
  pinMode(GAS_OUT, INPUT);

  delay(200);
  servo3.detach();
  servo4.detach();

  u8g.firstPage();
  do {
    u8g.setFont(u8g_font_fub14);
    u8g.setPrintPos(20, 40);
    u8g.print(" ");
  } while (u8g.nextPage());     //화면 빈 화면으로 초기화(즉시 꺼짐)

}





void loop() {
//변수 설정
  int gasValue = analogRead(GAS_OUT);           //아날로그데이터 읽어 gasValue 저장(가스센서 신호)
  static bool lastTouchState = LOW;             //이전 터치 상태 저장
  bool touchState = digitalRead(TOUCH2);

//초음파 준비
  digitalWrite(TRIG3, HIGH);           //펄스파 발생
  delayMicroseconds(10);              //10us 지연
  digitalWrite(TRIG3 ,LOW);            //펄스파 종료

  long duration = pulseIn(ECHO3, HIGH);          //Echo 핀이 high된 이후 지연된 펄스의 시간: duration
  if(duration == 0) { return; }                 //시간 0이면 종료

  long distance = duration / 58;                //총 걸리는 시간 58로 나눠 cm 단위로 거리 측정.

  Serial.print("Distance : ");    //시리얼 모니터로 거리 데이터 출력
  Serial.print(distance);
  Serial.println("cm");
  delay(1000);        //거리 데이터 2초마다 출력


  //OLED -입차 신호
  if (distance <= 6 && !carDetected) { 
    carDetected = true;
    displayStartTime = millis();    //현재 시간 저장

    u8g.firstPage();    
    do {
        u8g.setFont(u8g_font_fub14);
        u8g.setPrintPos(20, 40);
        u8g.print("PARKING");
    } while(u8g.nextPage());
} 


  // 5초 후 화면 끄기
      if (carDetected && millis() - displayStartTime >= 5000) {
        carDetected = false;  // 다시 감지할 수 있도록 초기화

        u8g.firstPage();
        do {
            u8g.setFont(u8g_font_fub14);
            u8g.setPrintPos(40, 55);
            u8g.print(" ");  // 빈 화면 출력
        } while (u8g.nextPage());
    }





//주방
  if(gasValue >=200)      //가스 누출 상황일 시
  {
    //피에조 알람 필요- 다른 보드에 존재. 아님 말고

    servo4.attach(SERVO_GAS);                             //서보모터 출력
    servo4.write(175);       //서보모터와 가스배관과 90도 방향되도록 회전
    delay(500);               //서보모터가 위치까지 도달할 수 있도록 1초 대기
    servo4.detach();         //서보모터 출력 신호 정지
  }

  btSerial.print(gasValue,1);     //앱 화면으로 가스 누출량 출력
  btSerial.print(" ");


  //후드(수동)
  if (touchState == HIGH && lastTouchState == LOW) {
          //hoodState = !hoodState;  // 후드 상태 변경
          //digitalWrite(HOOD_MOTOR, hoodState ? HIGH : LOW);  // 후드 ON/OFF
        

        //서보 동작확인
        
        servo3.attach(SERVO_WIN);
        servo3State = !servo3State;
        servo3.write(servo3State ? 115 : 175);
          delay(500);
        servo3.detach();
        
    }


    lastTouchState = touchState;  // 이전 상태 업데이트
    delay(50);  // 버튼 떨림(디바운싱) 방지





  //앱으로 조종 시 - 방2 led, 후드, 가스 밸브 조종 - 하지마 보류
/*
   if(btSerial.available())        //앱에서 데이터 발생, 블루투스 모듈로 데이터 입력됬을 시(조종)
  {
    byte input = btSerial.read();   //데이터 읽어 input 변수에 저장.


    switch(input)                   //input 값 맞는 case 실행
    {
      //방2 led on/off
      case 31:
           digitalWrite(LED_room2, HIGH);  
           break;


      case 32:
           digitalWrite(LED_room2, LOW);
           break;


      //부엌 창문 열/닫
       case 33:
          servo3.attach(SERVO_WIN);                             //서보모터 출력
          servo3.write(60);       
          delay(1000);               //서보모터가 위치까지 도달할 수 있도록 1초 대기
          servo3.detach();         //서보모터 출력 신호 정지
          break;

      case 34:
          servo3.attach(SERVO_WIN);                             //서보모터 출력
          servo3.write(0);       
          delay(1000);               //서보모터가 위치까지 도달할 수 있도록 1초 대기
          servo3.detach();         //서보모터 출력 신호 정지
          break;

      //후드 on/off(리모컨)
      case 35:
          hoodState = !hoodState;  // 후드 ON/OFF 토글
          digitalWrite(HOOD_MOTOR, hoodState ? HIGH : LOW);  // 후드 ON / OFF
          break;

    }
  }
*/

}


}

