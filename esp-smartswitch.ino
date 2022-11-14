#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

int led_status;

extern "C" {
#include <user_interface.h>
}

// WIFI网络基础信息设置
IPAddress apIP(192, 168, 6, 1);  
IPAddress gateway(192, 168, 6, 1);
IPAddress subnet(255, 255, 255, 0); 

MDNSResponder mdns;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void wificonfig_wifiOn() {
  wifi_fpm_do_wakeup();
  wifi_fpm_close();
  delay(100);
  }

#define FPM_SLEEP_MAX_TIME 0xFFFFFFF


void wificonfig_wifiOff() {
  wifi_station_disconnect();
  wifi_set_opmode(NULL_MODE);
  wifi_set_sleep_type(MODEM_SLEEP_T);
  wifi_fpm_open();
  wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
  delay(100);
  }

// root页内容(HTML)
String HTML = "<!DOCTYPE html><html><body><h1>Smart Switch\r\n</h1><h2>Control your switch on any devices</h2><p>Easy to control anything on your devices!</p><h3>Menu:</h3><a href=\"http://192.168.6.1/on\"><button>ON/OFF</button></a><a href=\"http://192.168.6.1/off\"><button>test</button></a><a href=\"http://192.168.6.1/update\"><button>update</button></a></body></html>";

String RediretRootHTML = "<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"0; URL='http://192.168.6.1/'\" /></head></html>";

// root页函数
void handleRoot() {
  digitalWrite(2, 1);
  httpServer.send(200, "text/html", HTML);
  digitalWrite(2, 0);
}

// 开关按下页函数
void handleOnButtom(){
  if(led_status == 0){
    digitalWrite(4, HIGH);
    led_status = 1;
    httpServer.send(200, "text/html", RediretRootHTML);
  }
  else{
    digitalWrite(4, LOW);
    led_status = 0;
    httpServer.send(200, "text/html", RediretRootHTML);
  }
}

// 开关关闭函数
void handleOffButtom(){
  digitalWrite(4, LOW);
  led_status = 0;
  httpServer.send(200, "text/html", RediretRootHTML);
}

// 初始化
void setup(void){
  led_status = 0;

  // 通过函数配置GPIO4用作继电器控制脚并使能，默认拉低（关闭）
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  
  // 监听三个不同的WebServer接口，实现触发对应函数
  httpServer.on("/", handleRoot);
  httpServer.on("/on", handleOnButtom);
  httpServer.on("/off", handleOffButtom);
  
  pinMode(LED_BUILTIN, OUTPUT);
  wificonfig_wifiOff();
  Serial.begin(115200);
  Serial.println(); 
  Serial.println("Booting Sketch...");
  wificonfig_wifiOn();
  WiFi.softAPConfig(apIP, gateway, subnet);
  boolean result = WiFi.softAP("Smart Switch", "66636888"); // "" => WiFi config
  Serial.println(result == true ? "AP setup OK" : "AP setup failed");

  IPAddress myIP = WiFi.softAPIP();  
  Serial.print("Access Point IP address: ");Serial.println(myIP);
  if (mdns.begin("switch", myIP)) {
    Serial.println("MDNS responder started");
    }
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  Serial.println("HTTPUpdateServer ready! Open http://switch.local/update");
  Serial.printf("or http://");Serial.print(myIP);Serial.println("/update in your browser");  
  }


void loop(void){
  httpServer.handleClient();
#if 0
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
#endif
  }
