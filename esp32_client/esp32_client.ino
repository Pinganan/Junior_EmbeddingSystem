#include <SimpleDHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

char SSID[] = "iPhone";
char PASSWORD[] = "ml5418ml";
String Linetoken = "PNpDLhcG1L8AkV5uHyuRtXoQoQdBZlKStSpTvTTvV6l";
int send_flag = 0;

int pinRain = 13;
int pinDHT11 = 14;
SimpleDHT11 dht11(pinDHT11);

WiFiClient client;//網路連線物件
char line_host[] = "notify-api.line.me";//LINE Notify API網址

//server的IP
const char host[] = "172.20.10.12";
const int port = 8088;
void setup()
{
    Serial.begin(115200);
    Serial.print("Connecting to ");
    Serial.println(SSID);
    //連接至Wifi
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected with IP address: ");
    Serial.println(WiFi.localIP());
    pinMode(pinRain, INPUT);
}
void loop()
{
    delay(5000);
    Serial.println("=================================");
    char humidity_msg[10];
    Serial.print("connecting to ");
    Serial.println(host);
    WiFiClient client;//連線物件
    //嘗試讀取
    Serial.println("Sample DHT11...");
    byte temperature = 0;
    byte humidity = 0;
    int err = SimpleDHTErrSuccess;
    if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
      Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
      return;
    }
    //讀取成功，將溫濕度顯示在序列視窗
    Serial.print("Sample OK: ");
    Serial.print((int)temperature); Serial.print(" *C, "); 
    Serial.print((int)humidity); Serial.println(" H");
    if (!client.connect(host, port)) {
        Serial.println("server connection failed");
        return;
    }
    //傳送環境溫度給server
    sprintf(humidity_msg,"%d", (int)humidity);
    client.print(humidity_msg);
    client.stop();
    
    int rainvalue = 0;
    int line_flag = 0;
    String message;
    //讀取雨水
    // 0->rain true
    rainvalue = digitalRead(pinRain);
    Serial.print("Rain value:");
    Serial.println(rainvalue);
    if(rainvalue == 0)
    {
      line_flag = 1;
      message = "下雨啦!";
      Serial.println(message);
    }
    else
    {
      send_flag = 0;
    }
    Serial.println(line_flag);
    Serial.println(send_flag);
    WiFiClientSecure lineclient;//網路連線物件
    if(line_flag == 1 && send_flag == 0){
      Serial.println("LINE");
      if (lineclient.connect(line_host, 443)) {
        send_flag = 1;
        int LEN = message.length();
        //傳遞POST表頭
        String url = "/api/notify";
        lineclient.println("POST " + url + " HTTP/1.1");
        lineclient.print("Host: "); lineclient.println(line_host);
        //權杖
        lineclient.print("Authorization: Bearer "); lineclient.println(Linetoken);
        lineclient.println("Content-Type: application/x-www-form-urlencoded");
        lineclient.print("Content-Length: "); lineclient.println( String((LEN + 8)) );
        lineclient.println();      
        lineclient.print("message="); lineclient.println(message);
        lineclient.println();
        //等候回應
        delay(2000);
        String response = lineclient.readString();
        //顯示傳遞結果
        Serial.println(response);
        lineclient.stop(); //斷線
      }
      else {
        //傳送失敗
        Serial.println("line connected fail");
      }
    }
}
