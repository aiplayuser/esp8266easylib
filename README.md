# esp8266easylib

Mgmt via Web: WiFi, MQTT, Update, Upload, Files. only 3 lines of code, 8266 is so easy to use.

#include "esp8266easylib.h"

void setup(){ setup1(); }

void loop(){ loop1(); }

esp8266易用库, 集成了常用的库和函数, 简化到只需输入三行代码, 就可以通过web页面实现WiFi连接, 系统更新, 文件上传管理, mqtt服务, 解决了使用8266的大部分基础工作, 大幅降低使用难度. 万事开头难, 迈出第一步就成功了一半, 该库就是为了解决使用8266开头难的问题, 让你快速迈出第一步, 不管做什么项目, 基本上都离不开联网, 上传程序, 文件管理, mqtt服务, 每次都要把这些功能重写一遍, 不如打包成一个库, 方便下次直接调用, 从而可以专注于自己的项目, 而不是想着怎么联网. 

克隆该项目到你的arduino项目文件夹Documents\Arduino\libraries就可以使用了. 

上传程序首次启动会以开发板id建立一个wifi信号, 手机连接该信号会自动弹出配置页面, 选择搜索附近WIFI, 输入密码连接, 连接成功后会自动返回首页, mqtt服务器后面数字为0表示连接成功, 重新修改ssid和pass以确保安全性, 设置一个足够复杂的mqtt主题, 防止和别人的主题相互干扰, 点save会保存配置信息到开发板上, 点删除或者格式化会删除配置信息. mqtt服务器网络故障时，会在loop函数里循环造成卡顿，目前还没有优化的方案，请确保服务器正常。

![alt text](https://github.com/aiplayuser/esp8266easylib/blob/main/esp8266easylib.PNG)

首次上传程序需要使用usb数据线通过com口上传, 以后只需要导出已编译的二进制文件, 直接拖拽到选择文件的地方, 开发板会自动上传文件更新系统. 

mqtt.html是一个简单的mqtt消息收发页面, 相当于一个web客户端, 把mqtt主题复制到这里, 点发送出现id号说明服务器连接成功, 再次点发送, 会搜索该主题下的所有设备, 把mqtt.html上传至手机或者电脑, 用浏览器打开,收藏或者添加到桌面, 方便下次使用. 

![alt text](https://github.com/aiplayuser/esp8266easylib/blob/main/mqtt.PNG)

GPIO(0)是板载的flash按钮, 这里用作复位, 按下清除ssid和pass, GPIO(2)是板载led, 这两个端口不要使用. 

代码很简单, 请仔细查看, 避免重复构建, 常用的库已经导入, 常用对象已经建立, web服务已经启动, 直接使用即可. 

该代码仅仅为了学习实验时让初学者快速上手，优化规范问题请自行解决。

