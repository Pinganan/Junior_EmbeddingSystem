#include <SimpleDHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

//ssid, password, token--------------------------------------------
char SSID[] = "iPhone";
char PASSWORD[] = "....."; #
WiFiServer server(8088);
String Linetoken = "..................................."; #token
int pinDHT11 = 14;
SimpleDHT11 dht11(pinDHT11);
int send_flag = 1;
//---------------------------------------------------------
WiFiClientSecure client;//網路連線物件
char host[] = "notify-api.line.me";//LINE Notify API網址

void setup() {
  Serial.begin(115200);
  //連線到指定的WiFi SSID
  Serial.print("Connecting Wifi: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  //連線成功，顯示取得的IP
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
  server.begin();
  
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}
void loop() {
  delay(1000);
  Serial.println(".");
  //檢查Wifi連接狀態，若無連線則重連
  if(WiFi.status()!= WL_CONNECTED){
    Serial.println("reconnect...");
    WiFi.reconnect();
  }
  //等待client連線
  WiFiClient client2 = server.available(); 
  uint8_t data[30]; 
  int environment_hum = 0;
  if (client2) {
    Serial.println("=================================");
    Serial.println("new client");         
    /* check client is connected */           
    while (client2.connected()) {          
        if (client2.available()) {
            int len = client2.read(data, 30);
            if(len < 30){
                data[len] = '\0';  
            }else {
                data[30] = '\0';
            }    
            Serial.print("client sent: ");
            environment_hum += ((data[0]-48)*10);
            environment_hum += (data[1]-48);        
            Serial.println(environment_hum);
          //----------------------------------------------------------
          String message;
          //嘗試讀取
          Serial.println("Sample DHT11...");
          byte temperature = 0;
          byte humidity = 0;
          int err = SimpleDHTErrSuccess;
          int line_flag = 0;
          if ((err = dht11.read(&temperature, &humidity, NULL)) != SimpleDHTErrSuccess) {
            Serial.print("Read DHT11 failed, err="); Serial.println(err);delay(1000);
            return;
          }
          //讀取成功，將溫濕度顯示在序列視窗
          Serial.print("Sample OK: ");
          Serial.print((int)temperature); Serial.print(" *C, "); 
          Serial.print((int)humidity); Serial.println(" H");
          //設定觸發LINE訊息條件，組成Line訊息內容
          // cloth dry
          if ((float)humidity <= environment_hum * 0.987 + 2.2) {
            line_flag = 1;
            message = "該收衣服囉!";
            Serial.println(message);
          }
          else
          {
            send_flag = 0;
          }
        
          if(line_flag == 1 && send_flag == 0){
            if (client.connect(host, 443)) {
              send_flag = 1;
              int LEN = message.length();
              //傳遞POST表頭
              String url = "/api/notify";
              client.println("POST " + url + " HTTP/1.1");
              client.print("Host: "); client.println(host);
              //權杖
              client.print("Authorization: Bearer "); client.println(Linetoken);
              client.println("Content-Type: application/x-www-form-urlencoded");
              client.print("Content-Length: "); client.println( String((LEN + 8)) );
              client.println();      
              client.print("message="); client.println(message);
              client.println();
              //等候回應
              delay(2000);
              String response = client.readString();
              //顯示傳遞結果
              Serial.println(response);
              client.stop(); //斷線
            }
            else {
              //傳送失敗
              Serial.println("connected fail");
            }
          }
        }
    } 
  }
}
