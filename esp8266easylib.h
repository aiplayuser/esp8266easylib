#ifndef _ESP8266EASYLIB_H_
#define _ESP8266EASYLIB_H_

#include "Arduino.h"
#include "DNSServer.h" //自动弹窗
#include "ArduinoJson.h" //json解析
#include "PubSubClient.h" //mqtt服务
#include "FS.h" //闪存系统
#include "Ticker.h" //定时器

#include "ESP8266WiFi.h" //wifi服务
#include "ESP8266WebServer.h" //web服务
ESP8266WebServer webserver(80); //web对象 
#define espmodel "Esp8266" //芯片类型
#define leddark 1 //gpio2板载led高电平熄灭

File file; DNSServer dnsserver; WiFiClient wificlient; 
PubSubClient mqttclient(wificlient); DynamicJsonDocument doc(55);
String espid, ssid, pass, mqttserver, mqttmess, topic; //建立常用变量和对象

String httphead(){ return R"( <!DOCTYPE html><html>
    <head><meta charset=UTF-8><style>a{TEXT-DECORATION:none}</style>
    <meta name=viewport content=\"width=device-width,initial-scale=1.2\"></head>
    <body><p><form action=/again method=POST>
    <input type=submit value=重新启动RST>&nbsp&nbsp<a href=/>返回home</a></form><p>
    <form action=/scanwifi method=POST><input type=submit value=搜索附近WiFi></form><p> )"; }  //统一页面的顶部

void setbeep(int pin,int f,int n,int d){ int tn=n*d; while(tn--){ tone(pin,f,66); delay(999/n); } }  //自定义蜂鸣器 快setbeep(D5,3333,5,1) 慢setbeep(D5,2222,2,1) 定时器中不能使用

void sendmqtt(String topic,String message){ mqttclient.publish( topic.c_str(), message.c_str() ); } //向主题发布消息

String ssidlink(){ return "<a href=http://" + WiFi.localIP().toString() + " target=_blank>" + ssid + "</a>"; }
void callback(char* topic, byte* payload, unsigned int length) { 
    char payloadchar[length + 1]; memcpy(payloadchar, payload, length); 
    payloadchar[length] = '\0'; mqttmess=(String)payloadchar; 
    if(mqttmess=="search..."){ sendmqtt(topic,ssidlink()); mqttmess=""; } }  //mqtt回调函数，返回一个包含设备ip的链接。 

void connectmqtt(){ mqttclient.setServer(mqttserver.c_str(), 1883); 
    mqttclient.setKeepAlive(60); mqttclient.setCallback(callback); 
    mqttclient.connect(espid.c_str()); mqttclient.subscribe(topic.c_str()); }  //连接mqtt服务器 

void homepage(){ String html = httphead() + "ID: " + espid + "-"; 
    html+= String(ESP.getFlashChipSize()/1024/1024) + "MB<p>";
    html+= String(espmodel) + "-APIP: " + WiFi.softAPIP().toString() + "<p>";
    html+= WiFi.SSID() + ": " + WiFi.localIP().toString() + "<p>" ; //开发板信息

    html+= "<form action=/upload name=form1 method=POST enctype=multipart/form-data>\
            <input type=file name=data onchange=document.form1.submit()></form><p>" ; //上传文件
    
    FSInfo fsinfo; SPIFFS.info(fsinfo); 
    html+="FSInfo: "+String(fsinfo.usedBytes)+"/"+String(fsinfo.totalBytes)+"-Byte<p>"; //闪存信息
    Dir dir=SPIFFS.openDir("/"); while(dir.next()){ 
        html+="<form action=/delone method=post><a href="+dir.fileName()+">"+dir.fileName()+"</a>--"+dir.fileSize();
        html+="<input name=filepath style=width:0px value="+dir.fileName()+"><input type=submit value=删除Del></form><p>";} 

    html+= "<form action=/saveconfig method=post>\
        server:<input name=mqttserver style=width:144px value="+mqttserver+">-"+String(mqttclient.state())+"<p>\
        topic : <input name=topic style=width:144px value="+topic+"><p>\
        SS ID : <input name=ssid style=width:144px value="+ssid+"><p>\
        PASS : <input name=pass style=width:77px value="+pass+"><input type=submit value=保存save></form><p>" ;  //自定义开发板配置

    html+= "<form action=/delall method=POST><input type=submit value=格式化Format></form><p></body></html>"; 
    webserver.send(200,"text/html",html) ;  
    }

void cjson(){ doc.clear(); file.close(); } //关闭json对象
void rjson(String jsonfile){ file = SPIFFS.open(jsonfile,"r"); deserializeJson(doc,file); } //读取json对象
void wjson(String jsonfile){ file = SPIFFS.open(jsonfile,"w"); serializeJson(doc,file); cjson(); } //写入json对象

void again(String info = "R S T..."){ 
    webserver.send( 200,"text/html",httphead()+"<h2>"+ info +"<br>稍后返回home</h2></body></html>" ); delay(3333); ESP.restart(); }

String getChip() { uint8_t mac[6]; WiFi.macAddress(mac); 
    return String(mac[0]) + String(mac[1]) + String(mac[2]) + String(mac[3]) + String(mac[4]); } //拼接mac地址作为id

String gettype(String path){ if( path.endsWith(".htm") || path.endsWith(".html") ) return "text/html"; 
    else if(path.endsWith(".json")) return "text/json"; return "application/octet-stream"; } //获取文件类型

void setup1(){ Serial.begin(9600); Serial.println("\nbegin\n"); 
    pinMode(0,INPUT_PULLUP); pinMode(2,OUTPUT); SPIFFS.begin(); espid=getChip(); //初始化系统

    rjson("/config.json"); ssid=doc["ssid"].as<String>(); pass=doc["pass"].as<String>(); 
    topic=doc["topic"].as<String>(); mqttserver=doc["mqttserver"].as<String>(); cjson(); 
    if(ssid=="null")ssid=espid; if(pass=="null")pass=""; if(topic=="null")topic="administrator"; 
    if(mqttserver=="null")mqttserver="test.mosquitto.org";  //读取config

    WiFi.mode(WIFI_AP_STA); WiFi.softAP(ssid,pass); WiFi.begin(); 
    webserver.begin(); dnsserver.start(53,"*",IPAddress(192,168,4,1)); //开始所有服务

    webserver.onNotFound( [](){ String path=webserver.uri(); if(!SPIFFS.exists(path)){ homepage(); }
        else{ file=SPIFFS.open(path,"r"); webserver.streamFile(file,gettype(path)); file.close(); } } ); //处理所有请求  

    webserver.on("/",HTTP_GET,homepage);//进入首页
    
    webserver.on("/upload", HTTP_POST, [](){ webserver.send(200); }, [](){ 
        HTTPUpload& upload = webserver.upload(); bool isbin = upload.filename.endsWith(".bin"); 
        if ( upload.status == UPLOAD_FILE_START ) { 
            isbin?Update.begin((ESP.getFreeSketchSpace()-0x1000)&0xFFFFF000,U_FLASH):file=SPIFFS.open("/"+upload.filename,"w"); } 
        else if ( upload.status == UPLOAD_FILE_WRITE ) { 
            isbin ? Update.write(upload.buf, upload.currentSize) : file.write(upload.buf,upload.currentSize); } 
        else if ( upload.status == UPLOAD_FILE_END ) { 
            if( isbin && Update.end(true) ){ again("Updata..."); }else{ file.close(); homepage(); } }            
        } );  //上传文件更新系统
        
    webserver.on("/again",[](){again();}); //重新启动系统
    webserver.on("/saveconfig",[](){ 
        ssid=webserver.arg("ssid"); pass=webserver.arg("pass"); 
        topic=webserver.arg("topic"); mqttserver=webserver.arg("mqttserver"); 
        rjson("/config.json"); doc["ssid"]=ssid; doc["pass"]=pass; 
        doc["topic"]=topic; doc["mqttserver"]=mqttserver; 
        wjson("/config.json"); WiFi.softAP(ssid,pass); homepage(); } ); //保存config

    webserver.on("/joinwifi",[](){ WiFi.begin(webserver.arg("ssid"),webserver.arg("pass")); 
        WiFi.persistent(1); homepage(); } ); //连接wifi
    webserver.on("/scanwifi",[](){ String html=httphead(); int n=WiFi.scanNetworks(); 
        for(int i=0;i>-100;i--){ for(int j=0;j<n;j++){ if(WiFi.RSSI(j)==i){ 
            html+="<form action=/joinwifi method=post>";
            html+="<input name=ssid size=24 value="+WiFi.SSID(j)+">"+WiFi.RSSI(j)+"<br>";
            html+="<input name=pass size=15><input type=submit value=连接wifi></form><p>"; } } }
            html+="</body></html>"; webserver.send(200,"text/html",html); } ); //扫描wifi
                           
    webserver.on("/deloneok",[](){ SPIFFS.remove(webserver.arg("filepath")); homepage(); } ); //确认删除单个文件
    webserver.on("/delone",[](){ String html=httphead();
        html+="<form action=/deloneok method=post><input name=filepath size=24 value="+webserver.arg("filepath");
        html+="><p><input type=submit value=删除Del></form></body></html>";
        webserver.send(200,"text/html",html); } ); //删除单个文件

    webserver.on("/delallok",[](){ SPIFFS.format(); homepage(); } ); //确认删除所有文件
    webserver.on("/delall",[](){ String html=httphead();
        html+="<form action=/delallok method=post><input type=submit value=格式化Format></form></body></html>"; 
        webserver.send(200,"text/html",html); } ); //删除所有文件
    }
void listenserver(){ if(!digitalRead(0)){ 
                        rjson("/config.json"); doc["pass"]=""; doc["ssid"]=espid; 
                        wjson("/config.json"); again(); } //监听板载按钮重置ssid和pass
    digitalWrite(2,!leddark); webserver.handleClient(); 
    digitalWrite(2,leddark); dnsserver.processNextRequest(); } //监听服务    

void loop1(){ mqttclient.loop(); listenserver(); //监听服务
    if(!mqttclient.connected()){ connectmqtt(); for(int i=0; i<33; i++)listenserver(); } }  

#endif


    
