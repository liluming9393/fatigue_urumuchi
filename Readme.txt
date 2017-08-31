疲劳检测、网络拨号检测、gps解析发送、数据接收四个线程
电信3g网卡拨号

no4.cpp中opencv文件路径需修改
报警信息、gps信息、注册信息发送IP、端口在types.h中定义
图片发送为短连接IP、端口在types.h中定义
设备ID在types.h中#define Device_ID 1
gps串口在types.h中定义

gps.cpp GPS数据解析,GPS串口初始化，GPS中断接收
network.cpp ppp连接、状态监测、socket建立、数据接收、接收数据解析
process.cpp 报警处理
upload.cpp 协议封装发送
no4.cpp 疲劳检测


增加GPS串口接收，使用中断方式进行接受
增加并测试GPS协议解析解析
对GPS_analysis函数中temp[0]为NULL的问题进行改正

使用带参数检测程序，加入报警声音
加入电信、联通pppd拨号脚本

修改GPS串口程序
增加读写互斥量
修改ppp初始化位置

修改socket中connect_tcp函数，改为非阻塞方式连接，消除由于阻塞导致GPS线程未执行程序崩溃问题
修改socket发送、接收timeout为1秒
修改图片发送函数中socket connet阻塞问题
增加DBG调试信息打印函数，type.h中define __DEBUG
修改gps速度解析，atoi改为atof
增加GPS时间间隔发送功能
增加GPS发送时间间隔上报功能
types.h增加软件版本号
设备状态信息帧中增加软件版本号字段
断线重连判断时间定义到types.h中

增加摄像头select timeout错误处理，关闭重新开启摄像头

增加signal(SIGPIPE, SIG_IGN);
增加recv和send后返回-1情况下的错误显示

增加检测报警开启关闭功能
修改no4.cpp，取消学习过程
ppp拨号前先将拨号进程kill
增加main函数启动信息设备ID
main函数及no4.cpp增加3次报警间隔采样数量的参数

增加m8t初始化、rtkrcv启动、rtk解算接收
调整DBG中\n改为\r\n

M8T单独使用程序初始化并保存配置，在疲劳检测程序中不进行初始化配置

网络线程中重连时增加释放sockfd1，防止不停生成套接字导致bad file description，shutdown加上close
增加close（sockfd2）

gps线程增加三种模式，在types.h中使用define切换

upload图片上传ioctl中sockfd2书写错误

增加版本更新功能

修改设备ID从ID.txt文档中获取
修改RTK方案，首先启动单点rtkrcv，获取定位后使用该时间修正系统时间，kill单点rtkrcv进程，开启差分rtkrcv进程

将RTK改为单独线程
RTKRCV输入数据改为tcpsvr方式，端口1240
增加gps.cpp中串口接收后通过1240端口转发程序
增加rtkrcv无结果输出时采用单点结果发送程序
修改rtkrcv配置程序
修改gps上传协议，增加定位状态字段
修改gps线程中解析RMC结果程序，将原始数据部分去除后解析

增加版本更新文件下载后处理

修改rtk时间设置，秒数+8*3600

修改差分与单点切换逻辑，2次解析rtk结果失败后切换为单点结果上传，解析出差分结果立即切换为差分结果上传。

增加log记录功能
增加看门狗功能

增加视频保存功能，超出一定大小后开启新文件保存
去除capture中120毫秒延时
修改版本更新下载文件格式及处理指令
1、下载压缩包文件sleep_upgrade.tar.gz
2、新建文件夹sleep_upgrade用于解压缩 mkdir ./sleep_upgrade
3、解压缩 tar -zxvf sleep_upgrade.tar.gz -C ./sleep_upgrade
4、将自启动文件移动到/etc下 mv -f ./sleep_upgrade/rc.local /etc/
5、将剩余文件移动到运行目录下 mv -f ./sleep_upgrade/* .
6、删除解压缩文件夹 rm -rf ./sleep_upgrade
7、删除压缩包 rm -f sleep_upgrade.tar.gz
版本更新压缩包文件内容
rc.local
sleep_detect
rtkrcv
rtkrcv_rtk.conf
rtkrcv_single.conf
rtkshut.sh
rtkstart.sh
将所有文件放入文件夹 执行tar -zcvf ./sleep_upgrade.tar.gz *

增加连续30次connect tcp失败重启系统

增加可用磁盘空间检测，可用空间小于1GB时不保存视频，剩余用空保留用于log保存
修改log、视频保存路径为types.h中save_path设置，挂载sd卡后填写挂载路径
去除疲劳检测报警声音

修改GPS上报信令，增加疲劳检测和视频保存状态
增加疲劳检测和视频保存切换功能
使用文本保存疲劳检测和视频保存切换状态，保证下次启动时依旧运行关机时的模式，文件名detect_save_flag.txt
增加挂载检测

增加禁停报警信息接收及声音报警
增加记录报警声音开关文件aplay_flag.txt
修改GPS上报信令，增加报警声音开关状态
