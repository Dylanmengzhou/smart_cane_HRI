#include <SoftwareSerial.h>
SoftwareSerial Bluetooth(2, 3);

void setup() {
  Bluetooth.begin(9600);
  Serial.begin(9600);
}

void loop() {
  if (Bluetooth.available()) {
    // 读取接收到的字符直到遇到换行符
    String received = "";
    while (Bluetooth.available()) {
      char c = (char)Bluetooth.read();
      received += c;
      delay(10); // 稍微延迟以等待数据
    }
    
    Serial.println("DATA RECEIVED:");
    Serial.println(received);
  }
}
