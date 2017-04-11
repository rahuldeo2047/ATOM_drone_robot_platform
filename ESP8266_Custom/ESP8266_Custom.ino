/*
    This sketch shows the WiFi event usage

*/

#include "data.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <ESP8266WebServer.h>

#include "SerialCommand.h"
#include <Bounce2.h>

SerialCommand SCmd(&Serial);
WiFiUDP udp;
#define LOCAL_PORT (10000)
int remote_port = 20000;
IPAddress remote_ip(192, 168, 1, 3);

ESP8266WebServer server(80);

const char* www_username = "admin";
const char* www_password = "esp8266";

const char* ssid     = "RICK";
const char* password = "88888888";

int system_mode = -1;

#define BUTTON_PIN 0
Bounce debouncer = Bounce();

void sendmsg(char* msg, int size)
{
  udp.beginPacket(remote_ip, (uint16_t) remote_port);
  udp.write(msg, size);
  udp.endPacket();
}

void WiFiEvent(WiFiEvent_t event) {
  //Serial.printf("[WiFi-event] event: %d\n", event);

  switch (event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
      Serial.println("ready");
      udp.begin(LOCAL_PORT);
      sendmsg("I AM ON", 8);
      Serial.println(system_mode);
      Serial.println("OK");

      server.on("/", []() {
        if (!server.authenticate(www_username, www_password))
          return server.requestAuthentication();
        server.send(200, "text/plain", "Login OK");
      });
      server.begin();

      Serial.print("Open http://");
      Serial.print(WiFi.localIP());
      Serial.println("/ in your browser to see it working");

      //udp.beginPacket(addr, port)
      //Serial.println("IP address: ");
      //Serial.println(WiFi.localIP());
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:
      Serial.println("WiFi lost connection");
      udp.stop();
      break;
  }
}

void setup() {
  Serial.begin(115200);
  //Serial.println("START DIRECTLY WITH TRANSPARENT MODE");
  //Serial.println("ISSUE with serial");
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  debouncer.attach(BUTTON_PIN);
  debouncer.interval(200);
  SCmd.addCommand("AT", check);
  SCmd.addCommand("AT+RST", resetESP8266);
  //SCmd.addCommand("AT+MODE", operationMode);
  SCmd.addCommand("AT+START", startWiFi);
  SCmd.addCommand("AT+CIFSR", printIP);
  //SCmd.addCommand("AT+SEND", sendData);
  SCmd.addCommand("AT+SRIPP", setRemoteIPPort);

  SCmd.addDefaultHandler(unrecognized);

  // WiFi.onEvent(WiFiEvent);

  startWiFi();

}

int button_state = 0;
void loop() {
  Data data;
  data.data.header.header1 = 0x02;
  data.data.header.header2 = 0xFF;
  data.data.header.header3 = 0xFE;

  //delay(100);
  //  SCmd.readSerial();
  server.handleClient();
  boolean changed = debouncer.update();
  //int value = debouncer.read();
  //
  if (changed) // keep it low for cmd mode
  {
    //Serial.println(debouncer.read());
    button_state = !debouncer.read();
  }

  if (0 == button_state)
  {
    data.data.header.header1 = 0x02;
    data.data.header.header2 = 0xFF;
    data.data.header.header3 = 0xFE;
    data.data.header.type1 = DATA_TYPE1_SERIAL;
    data.data.header.type2 = 0;

    int datasize = udp.available();//udp.parsePacket();
    //char buff[64] = {0};
    if (datasize)
    {
      data.data.header.len = datasize; // this datasize is not valid
      udp.read(data.uc_buffer, datasize > MAX_PAYLOAD_SIZE ? MAX_PAYLOAD_SIZE : datasize);
      //server.send(data.uc_buffer);
      //server.send(200, "text/plain", data.c_buffer);
      //EthernetClient client = server.available();
      //Serial.print("+IPD,");
      //Serial.print("0,");
      //Serial.print(datasize);
      //Serial.print(":");
      // Protocol
      Serial.write(data.uc_buffer, datasize);
    }

    datasize = Serial.available();
    if (datasize)
    {
      data.data.header.header1 = 0x02;
      data.data.header.header2 = 0xFF;
      data.data.header.header3 = 0xFE;
      data.data.header.chk = 0x00;
      data.data.header.type1 = DATA_TYPE1_SERIAL;
      data.data.header.type2 = DATA_TYPE2_SERIAL_0;
      data.data.header.res = 0x00;
      data.data.header.len = datasize ; //0xFFFF; // test
      //Serial.print("<");
      //Serial.print(datasize);
      //Serial.println(">");

      Serial.readBytes(data.data.c_payload, datasize > MAX_PAYLOAD_SIZE ? MAX_PAYLOAD_SIZE : datasize);
      //server.send(200, "text/plain", data.c_buffer);
      sendmsg(data.c_buffer, datasize+DATA_HEADER_LEN );
    }
  }
  else
  {
    SCmd.readSerial();
  }

}


// AT+SETREMOTEIPPORT IP1.IP2.IP3.IP4 PPORTT
void setRemoteIPPort()
{
  char *arg = NULL;
  arg = SCmd.next();

  if (arg != NULL)      // As long as it existed, take it
  {
    remote_ip.fromString(arg); // set IP
    Serial.print(remote_ip);
  }
  else {
    unrecognized();
    return;
  }

  Serial.print(" ");

  arg = SCmd.next();
  if (arg != NULL)      // As long as it existed, take it
  {
    remote_port = atoi(arg); // set PORT
    Serial.print(remote_port);
  }
  else {
    unrecognized();
    return;
  }
  Serial.println();
  Serial.println("OK");
}
//AT+SEND 2 HI
//AT+SEND HI
void sendData()
{
  char *arg = NULL;
  //  arg = SCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    Serial.print("OK");
    sendmsg(arg, strlen(arg)); // coculd be danngerous
    //Serial.println(arg);
  }
  else {
    unrecognized();
  }

}

void printIP()
{
  Serial.println(WiFi.localIP());
}

void startWiFi()
{
  // delete old config
 // WiFi.disconnect(true);

  delay(200);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(ssid, password);
  Serial.println("+STARTing...");
}

void operationMode()
{
  char *arg;
  //  arg = SCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL)      // As long as it existed, take it
  {
    Serial.print("+MODE");
    system_mode = atoi(arg); // check 0 or 1
    Serial.println(arg);
  }
  else {
    unrecognized();
  }
}

void resetESP8266()
{
  Serial.println("OK");
  delay(100);
  Serial.println("<Not_Implemented>");
}

void check()
{
  // wifi test
  Serial.println("OK");
}

void unrecognized()
{
  Serial.println("ERROR What?");
}