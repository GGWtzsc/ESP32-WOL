#include <WiFi.h>
#include <WebServer.h>
#include <WakeOnLan.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

const char* ssid = "aa";
const char* password = "aabbccdd";
const char* www_username = "aa";
const char* www_password = "1234";
const char* mqtt_server = "xxxxxxx.com";
#define PORT 18883
const char *MACAddress = "aa:bb:cc:dd:ee:ff";

#define LED 22
#define Button 18
WebServer server(80);

int a;
int button = 0;
int lastbutton = 0;

WiFiUDP UDP;
WakeOnLan WOL(UDP);

WiFiClient espClient;
PubSubClient client(espClient);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta charset="UTF-8" meta name="viewpoDrt" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1 align="center" style = "background-color:#C8E8ED;">ESP32 control</h1>
  <h3 align="center">ESP32 Control</h3>
  <P> <br/>
  <br/></p>
  <p align="center">indicator: %ANGLE% ;</p>
  <P><br/><p>
  <fontbase size=2>
  <p align="center"><font size="6">&nbsp;&nbsp;&nbsp;<a href="/0"><button style="width: 300px; height: 100px; background-color:#F1948A;"><font size=6>ON</font></button></a></font></p>
    <P> <br/>
  <br/><p>
  <p align="center"><font size="6">&nbsp;&nbsp;&nbsp;<a href="/1"><button style="width: 300px; height: 100px; background-color:#ABEBC6;"><font size=6>refresh</font></button></a></font></p>
</body>
</html>
)rawliteral";

void handleRoot() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication(); 
    return;
  }
  String s = index_html;
  s.replace("%ANGLE%", String(a));
  server.send(200, "text/html",s);
}

void handleZero() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication(); 
    return;
  }
  digitalWrite(LED,LOW);
  WOL.sendMagicPacket(MACAddress);
  a=!digitalRead(LED);
  server.sendHeader("Location", "/",true);
  server.send(302, "text/plain","");
  delay(100);
  digitalWrite(LED,HIGH);
}

void handleOne() {
  if (!server.authenticate(www_username, www_password)) {
    server.requestAuthentication(); 
    return;
  }
  a=!digitalRead(LED);
  server.sendHeader("Location", "/",true);
  server.send(302, "text/plain","");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  WOL.sendMagicPacket(MACAddress);
  digitalWrite(LED, LOW);
  delay(200);
  digitalWrite(LED, HIGH);
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("try...");
    if (client.connect("ESP_YCKJ_Client")) {
      Serial.print("OK：");
      Serial.println(mqtt_server);
      client.subscribe("PCswitch");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  digitalWrite(LED,HIGH);
  a = !digitalRead(LED);

  pinMode(Button, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, PORT);
  client.setCallback(callback);

  server.on("/", handleRoot);
  server.on("/0", handleZero);
  server.on("/1", handleOne);

  server.begin();
}

void loop() {
  server.handleClient();
  button = digitalRead(Button);
  if(button != lastbutton){
   if(button == LOW){
    digitalWrite(LED,LOW);
    WOL.sendMagicPacket(MACAddress);
    delay(100);
    digitalWrite(LED,HIGH);
    }
   lastbutton = button;
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
