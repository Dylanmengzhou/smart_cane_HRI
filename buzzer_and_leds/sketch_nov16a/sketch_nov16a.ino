#include "pitches.h"
#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(2, 3);  // RX, TX

#define BUZZER_PIN 8
#define LED_PIN1 9
#define LED_PIN2 10
#define BUTTON_PIN 7
#define VIB_PIN 11
#define BT_STATE_PIN 4                            // 蓝牙模块状态引脚
int ledFrequency, buzzerFrequency, vibFrequency;  //接收数据
int melodyConnect[] = { NOTE_C4, NOTE_E4, NOTE_G4 };
int noteDurationsConnect[] = { 4, 4, 4 };
int melodyDisconnect[] = { NOTE_G4, NOTE_E4, NOTE_C4 };
int melodyBattery[] = { NOTE_D4, NOTE_C4, NOTE_C4 };
int noteDurationsDisconnect[] = { 4, 4, 4 };
int noteDurationBattery[] = { 4, 4, 4 };

  // 重新定义播放旋律的数组
  int melody[] = { NOTE_DS8 };
int noteDurations[] = { 8 };

unsigned long previousLEDUpdate = 0;
unsigned long previousMelodyUpdate = 0;
unsigned long previousVIBUpdate = 0;

int melodyIntervals[3] = { 500, 250, 125 };
int ledIntervals[3] = { 1000, 500, 250 };
int VIBIntervals[3] = { 500, 250, 125 };

int melodyIndex = 0;
bool toggleLED = false;
bool toggleVIB = false;
bool isPlaying = true;

int prevLedFrequency = -1;
int prevBuzzerFrequency = -1;
int prevVibFrequency = -1;
int power = 0;

unsigned long elseStartTime = 0;
bool isElseActive = false;


bool lastBTState = LOW;  // 上一次蓝牙状态

void setup() {
  Bluetooth.begin(9600);
  Serial.begin(9600);
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(VIB_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BT_STATE_PIN, INPUT);  // 设置蓝牙状态引脚为输入
}

void loop() {
  if (power == 0) {
    playTone(melodyBattery, noteDurationsDisconnect, 3);
    power = 1;
  }
  readButton();
  bool currentBTState = digitalRead(BT_STATE_PIN);

  // 检测蓝牙连接状态变化
  if (currentBTState != lastBTState) {
    if (currentBTState == HIGH) {
      playTone(melodyConnect, noteDurationsConnect, 3);  // 播放连接声音
    } else {
      playTone(melodyDisconnect, noteDurationsDisconnect, 3);  // 播放断开声音
      stopAll();                                               // 蓝牙断开时，停止所有活动
      isPlaying = false;                                       // 更新播放状态为停止
    }
    lastBTState = currentBTState;
  }

  if (Bluetooth.available()) {
    String received_string = "";
    // end line check
    while (Bluetooth.available()) {
      char c = Bluetooth.read();
      if (c == '#') {
        break;
      } else {
        received_string += c;
        delay(10);
      }
    }
    // 将接收到的字符串分割为三个数字
    parseReceivedData(received_string);
    Serial.println("DATA RECEIVED:");
    Serial.println(received_string);
    isPlaying = true; // 在接收到新数据时重置为播放状态
    isElseActive = false;
    elseStartTime = 0;
  } else {
    if (!isElseActive) {
        // else active, record the start time
        elseStartTime = millis();
        isElseActive = true;
    } else if (millis() - elseStartTime > 7000) { // Check if 7 seconds have passed
        Serial.println("zero");
        ledFrequency = 0;
        buzzerFrequency = 0;
        vibFrequency = 0;
        elseStartTime = millis();
    }
  }
  if (isPlaying && isValidFrequency(ledFrequency) && isValidFrequency(buzzerFrequency) && isValidFrequency(vibFrequency)) {
    if (prevLedFrequency != ledFrequency || prevBuzzerFrequency != buzzerFrequency || prevVibFrequency != vibFrequency) {

      stopAll();
    }
    if (ledFrequency > 0) {

      blinkLEDs();
    }
    if (buzzerFrequency > 0) {

      playMelody();
    }
    if (vibFrequency > 0) {

      vibrateMotor();
    }
    prevLedFrequency = ledFrequency;
    prevBuzzerFrequency = buzzerFrequency;
    prevVibFrequency = vibFrequency;
    delay(400);  // Minimum runtime
  } else {
    stopAll();
  }
}
bool isValidFrequency(int frequency) {
  return frequency >= 0 && frequency <= 8;
}

void parseReceivedData(String data) {
  int firstCommaIndex = data.indexOf(',');
  int secondCommaIndex = data.lastIndexOf(',');

  // 提取并转换数字
  ledFrequency = data.substring(0, firstCommaIndex).toInt();
  // Serial.println(ledFrequency);
  buzzerFrequency = data.substring(firstCommaIndex + 1, secondCommaIndex).toInt();
  // Serial.println(buzzerFrequency);
  vibFrequency = data.substring(secondCommaIndex + 1).toInt();
  // Serial.println(vibFrequency);
}

void blinkLEDs() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousLEDUpdate >= ledIntervals[ledFrequency - 1]) {
    previousLEDUpdate = currentMillis;
    digitalWrite(LED_PIN1, toggleLED ? HIGH : LOW);
    digitalWrite(LED_PIN2, toggleLED ? LOW : HIGH);
    toggleLED = !toggleLED;
  }
}

void playMelody() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMelodyUpdate >= melodyIntervals[buzzerFrequency - 1]) {
    previousMelodyUpdate = currentMillis;
    if (melodyIndex < 1) {
      int noteDuration = 1000 / noteDurations[melodyIndex];
      tone(BUZZER_PIN, melody[melodyIndex], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZER_PIN);
      melodyIndex++;
    } else {
      melodyIndex = 0;
    }
  }
}

void vibrateMotor() {
  unsigned long currentMillis = millis();
  if (vibFrequency == 3) {        // 最快频率
    digitalWrite(VIB_PIN, HIGH);  // 持续震动
  } else if (currentMillis - previousVIBUpdate >= VIBIntervals[vibFrequency - 1]) {
    previousVIBUpdate = currentMillis;
    digitalWrite(VIB_PIN, toggleVIB ? HIGH : LOW);
    toggleVIB = !toggleVIB;
  }
}

void readButton() {
  if (digitalRead(BUTTON_PIN) == LOW && isPlaying) {
    isPlaying = false;
    delay(200);  // 防抖动延时
  }
}

void stopAll() {
  digitalWrite(LED_PIN1, LOW);
  digitalWrite(LED_PIN2, LOW);
  digitalWrite(VIB_PIN, LOW);
  noTone(BUZZER_PIN);
  melodyIndex = 0;
  toggleLED = false;
  toggleVIB = false;
}

void playTone(int melody[], int durations[], int size) {
  for (int thisNote = 0; thisNote < size; thisNote++) {
    int noteDuration = 1000 / durations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}