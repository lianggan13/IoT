#!/usr/bin/python
#coding:utf-8
import sys
import os
from PyQt4.QtGui import *
from PyQt4.QtCore import * 
from PyQt4.QtWebKit import *
from PyQt4.QtNetwork import *
import threading
from socket import *
import time
import requests
import struct
import json
import urllib2
import cv2
import cv2.cv as cv
import Image,StringIO
import face

BUFFSIZE = 1024
global HOST 
global PORT
global temp_2
global humi
global pressure
global altitude
global distance
global currentTime
#视频图片信息 CSI 接收线程
class picCSIThread(QThread):
    savePicCSI = pyqtSignal(int)
    def __init__(self):
        super(picCSIThread,self).__init__()
    def run(self):
        global HOST
        global PORT
        FILEINFO_SIZE=struct.calcsize('l')
        evt3.wait()
        sockfd = socket(AF_INET,SOCK_STREAM)
        sockfd.connect((HOST,PORT))
        print 'picCSIThread connect..................'
        while True:
            filesize = sockfd.recv(FILEINFO_SIZE)
            filesize = struct.unpack('l',filesize)
            restsize = filesize[0]
            with open('./image/imgCSI.jpg','wb') as fp:
                while True:
                    if restsize > BUFFSIZE:
                        filedata = sockfd.recv(BUFFSIZE)
                    else:
                        filedata = sockfd.recv(restsize)
                    if not filedata: break
                    fp.write(filedata)
                    restsize = restsize - len(filedata)
                    if restsize == 0: break
            if face.face_detect():
                self.savePicCSI.emit(1)
            else:
                self.savePicCSI.emit(0)

        sockfd.close()
        print "recv pic finished..."

#视频图片信息 USB 接收线程
class picUSBThread(QThread):
    savePic = pyqtSignal()
    def __init__(self):
        super(picUSBThread,self).__init__()
    def run(self):
        global HOST
        global PORT
        evt2.wait()
        self.sockfd = socket(AF_INET,SOCK_STREAM)
        self.sockfd.connect((HOST,PORT))
        evt3.set()
        f = self.sockfd.makefile()
        print 'picUSBThread connect..................'
        while True:
            # 接收图片信息 显示在中央主Label上  lable.setPixmap()
            msg = f.readline()
            if not msg:
                continue
            #print len(msg)
            jpeg = msg.replace("\-n","\n")
            buf = StringIO.StringIO(jpeg[0:-1])
            buf.seek(0)
            pi = Image.open(buf)
            img = cv.CreateImageHeader((640,480),cv.IPL_DEPTH_8U,3)
            cv.SetData(img,pi.tobytes())
            buf.close()
            cv.SaveImage("./image/imgUSB.jpeg",img) #注:保存的图片格式必须是:jpeg 640x480
            self.savePic.emit()   #发送图片保存信号 
            
        self.sockfd.close()
		
	
#字符串信息 接收线程
class recvThread(QThread):
    warningSound = pyqtSignal(int)
    def __init__(self):
        super(recvThread,self).__init__()
    def run(self):
        global temp_2
        global humi
        global pressure
        global altitude
        global distance
        global HOST
        global PORT
        DATA_SIZE = struct.calcsize('iiiiiii')
        self.sockfd = socket(AF_INET,SOCK_STREAM)
        self.sockfd.connect((HOST,PORT))
        print 'recvThread connect..................'
	evt.set()
        while True:
            data = self.sockfd.recv(DATA_SIZE)
            temp_2,humi,pressure,altitude,distance,flame,fog = struct.unpack('iiiiiii',data)
            MainWindow.lcd[1].display(pressure)#压强
            MainWindow.lcd[2].display(altitude)#海拔
            #MainWindow.label[1].setText(str(distance))#距离
            if(temp_2 != 999): 
                MainWindow.lcd[0].display(temp_2-3)#温度
                MainWindow.lcd[3].display(humi)#湿度
               #MainWindow.label[3].setText(u"空气湿度:"+str(humi)+"%")
                print "dht11",temp_2,humi
       
            if flame == 1 and fog == 1:
                self.warningSound.emit(1)
            elif fog == 1:
                self.warningSound.emit(2)
            elif flame == 1:
                self.warningSound.emit(3)
            else:self.warningSound.emit(0)
                        
        self.sockfd.close()

#创建一个总窗口类 class MainWindow ()
class MainWindow(QWidget):
    global distance
    global currentTime
    lcd = ['lcd0','lcd1','lcd2','lcd3']
    label = ['label_camera','label_distance','label_warning','label_humi']
    def __init__(self,parent = None):
    	QWidget.__init__(self,parent)
      #添加各种布局 各种控件
	self.yeeLinkTime = QTimer()
	self.yeeLinkTime.setInterval(60000) 

        self.labelIP    = QLabel(u"服务器IP:    ")
        self.labelPort  = QLabel(u"服务器Port:")
        self.lineEditIP = QLineEdit()
        self.lineEditPort = QLineEdit()
        self.lineEditIP.setText("192.168.191.4")
        self.lineEditPort.setText("6666")
        self.setWindowTitle(u"控制面板")
        
        
        self.connWeb = QPushButton(u"打开浏览器") 
        self.loginButton = QPushButton(u"登录")
        self.setButton   = QPushButton(u"重设")
        self.sockfd = 0
        MainWindow.lcd[0] = QLCDNumber(self)
        MainWindow.lcd[1] = QLCDNumber(self)
        MainWindow.lcd[2] = QLCDNumber(self)
        MainWindow.lcd[3] = QLCDNumber(self)
        self.label1 = QLabel(u"温度:")
        self.label2 = QLabel(u"压强:")
        self.label3 = QLabel(u"海拔:")
        self.label4 = QLabel(u"湿度:")
        self.label1_1 = QLabel(u"°C")
        self.label2_1 = QLabel("Pa")
        self.label3_1 = QLabel("m")
        self.label4_1 = QLabel("%")
       # MainWindow.label[2] = QLabel("------------")
       # MainWindow.label[2].resize(400,300)
        self.icoSafe = QIcon("./image/ico/safe.jpg")
        self.icoNoSafe = QIcon("./image/ico/nosafe.jpg")
        self.icoFlame = QIcon("./image/ico/flame.jpg")
        self.icoNoFlame = QIcon("./image/ico/noflame.jpg")
        self.icoFog = QIcon("./image/ico/fog.jpg")
        self.icoNoFog = QIcon("./image/ico/nofog.jpg")
        self.buttonSafe = QPushButton()
        self.buttonFlame = QPushButton()
        self.buttonFog  = QPushButton()
        self.buttonSafe.setIcon(self.icoSafe)
        self.buttonFlame.setIcon(self.icoNoFlame)
        self.buttonFog.setIcon(self.icoNoFog)
        self.buttonSafe.setIconSize(QSize(50,20))
        self.buttonFog.setIconSize(QSize(50,20))
        self.buttonFlame.setIconSize(QSize(50,20))
        #MainWindow.label[3] = QLabel()
        #MainWindow.label[3].resize(400,100)
       
        #加载按钮图片
        self.ico0 = QIcon("./image/ico/0.ico")
        self.ico2 = QIcon("./image/ico/2_1.jpg")
        self.ico4 = QIcon("./image/ico/4.ico")
        self.ico5 = QIcon("./image/ico/5_1.jpg")
        self.ico6 = QIcon("./image/ico/6.ico")
        self.ico7 = QIcon("./image/ico/7_1.jpg")
        self.ico11 = QIcon("./image/ico/11.ico")
        #CSI视频
        self.labelCSI = QLabel()
        self.buttonCSI = QPushButton()
        self.buttonCSI.setIconSize(QSize(50,20))
        self.buttonCSI.setIcon(self.ico7)
        #self.buttonCSI.setFlat(True)
        self.buttonPin = QPushButton()
        self.buttonPin.setIconSize(QSize(50,20))
        self.buttonPin.setIcon(self.ico2)
        #self.buttonPin.setFlat(True)
        self.isCSI =False
        self.imgCSI = QImage()
        self.imgCSI.load("./image/PiCamera.jpg")
        self.imgCSI = self.imgCSI.scaled(300,200,Qt.KeepAspectRatio)
        self.labelCSI.setPixmap(QPixmap.fromImage(self.imgCSI))
        #USB视频
        MainWindow.label[0]  = QLabel()
        MainWindow.label[0].setGeometry(10,10,800,600)
        self.saveVideoButton = QPushButton()
        self.stopButton = QPushButton()
        self.playButton = QPushButton()
        self.saveVideoButton.setIconSize(QSize(80,20))
        self.saveVideoButton.setIcon(self.ico0)
        #self.saveVideoButton.setFlat(True)
        self.stopButton.setIconSize(QSize(80,20))
        self.stopButton.setIcon(self.ico11)
        self.playButton.setIconSize(QSize(80,20))
        self.playButton.setIcon(self.ico6)
        #self.playButton.setFlat(True)
        
        self.isPicOnLabel=False
        self.img = QImage()
        self.scaledimg = QImage()
        self.img.load("./image/swpu.jpg")
        self.scaledimg = self.img.scaled(700,500,Qt.KeepAspectRatio); 
        MainWindow.label[0].setPixmap(QPixmap.fromImage(self.scaledimg))

       # MainWindow.label[1] = QLabel(u"距离",MainWindow.label[0])
       # MainWindow.label[1].setAlignment(Qt.AlignCenter)                #字体:上下左右居中
       # MainWindow.label[1].setFont(QFont("Roman times",13,QFont.Bold)) #字体:类型 大小 加粗
       # MainWindow.label[1].setStyleSheet("color:green")               #字体:颜色
       # self.clockLabel        = QLabel(MainWindow.label[0]) 
       # pe  = QPalette()
       # pe.setColor(QPalette.WindowText,Qt.red)#设置字体颜色  
       # self.clockLabel.setPalette(pe)
       # self.clockLabel.setGeometry(550,480,150,20)
        self.clockTimer = QTimer()
        self.clockTimer.start(1000)
        self.clockTimer.timeout.connect(self.showTime)
        
        #布局box
        hbox1 = QHBoxLayout()
        hbox1.addWidget(self.label1)
        hbox1.addWidget(MainWindow.lcd[0])
        hbox1.addWidget(self.label1_1)
        hbox2 = QHBoxLayout()
        hbox2.addWidget(self.label2)
        hbox2.addWidget(MainWindow.lcd[1])
        hbox2.addWidget(self.label2_1)
        hbox3 = QHBoxLayout()
        hbox3.addWidget(self.label3)
        hbox3.addWidget(MainWindow.lcd[2])
        hbox3.addWidget(self.label3_1)
        hbox4 = QHBoxLayout()
        hbox4.addWidget(self.label4)
        hbox4.addWidget(MainWindow.lcd[3])
        hbox4.addWidget(self.label4_1)

        vbox = QVBoxLayout()
        vbox.addLayout(hbox1)
        vbox.addLayout(hbox2)
        vbox.addLayout(hbox3)
        vbox.addLayout(hbox4)
        #vbox.addWidget(MainWindow.label[3])
        hboxWarning = QHBoxLayout()
        hboxWarning.addWidget(self.buttonSafe)
        hboxWarning.addWidget(self.buttonFog)
        hboxWarning.addWidget(self.buttonFlame)
        vbox.addLayout(hboxWarning)
        vbox.addWidget(self.labelCSI)
        hboxButtonCSI = QHBoxLayout()
        hboxButtonCSI.addWidget(self.buttonPin)
        hboxButtonCSI.addWidget(self.buttonCSI)
        hboxButtonCSI.addStretch()
        vbox.addLayout(hboxButtonCSI)
        vbox.addWidget(self.connWeb)
        #vbox.addStretch()  #添加纵向隔离器
        
        hboxIP   = QHBoxLayout()
        hboxIP.addWidget(self.labelIP)
        hboxIP.addWidget(self.lineEditIP)
        hboxPort = QHBoxLayout()
        hboxPort.addWidget(self.labelPort)
        hboxPort.addWidget(self.lineEditPort)
        vbox.addLayout(hboxIP)
        vbox.addLayout(hboxPort)
        hboxLink = QHBoxLayout()
        hboxLink.addStretch()
        hboxLink.addWidget(self.loginButton)
        hboxLink.addWidget(self.setButton)
        vbox.addLayout(hboxLink)

        vboxPic = QVBoxLayout()
        vboxPic.addWidget(MainWindow.label[0])
        hboxPicButton = QHBoxLayout()
        hboxPicButton.addWidget(self.saveVideoButton)
        hboxPicButton.addWidget(self.stopButton)
        hboxPicButton.addWidget(self.playButton)
        hboxPicButton.addStretch()
        vboxPic.addLayout(hboxPicButton)
        hbox3 = QHBoxLayout()
        hbox3.addLayout(vboxPic)
        hbox3.addLayout(vbox)
        
        self.setLayout(hbox3)
        self.resize(1000,700)

        MainWindow.lcd[0].setDigitCount(5)
        MainWindow.lcd[0].setMode(QLCDNumber.Dec)
        MainWindow.lcd[0].setSegmentStyle(QLCDNumber.Flat)
        MainWindow.lcd[0].setStyleSheet("border:1px solid green;color:red;background:silver")
        MainWindow.lcd[1].setDigitCount(5)
        MainWindow.lcd[1].setMode(QLCDNumber.Dec)
        MainWindow.lcd[1].setSegmentStyle(QLCDNumber.Flat)
        MainWindow.lcd[1].setStyleSheet("border:1px solid green;color:green;background:silver")
        MainWindow.lcd[2].setDigitCount(5)
        MainWindow.lcd[2].setMode(QLCDNumber.Dec)
        MainWindow.lcd[2].setSegmentStyle(QLCDNumber.Flat)
        MainWindow.lcd[2].setStyleSheet("border:1px solid green;color:green;background:silver")
        MainWindow.lcd[3].setDigitCount(5)
        MainWindow.lcd[3].setMode(QLCDNumber.Dec)
        MainWindow.lcd[3].setSegmentStyle(QLCDNumber.Flat)
        MainWindow.lcd[3].setStyleSheet("border:1px solid green;color:green;background:silver")

        self.recvthread = recvThread()          #字符串线程(接收传感器信息)
        self.picUSBthread  = picUSBThread()        #USB图像线程(接收图像信息)
        self.picCSIthread = picCSIThread()      #CSI图像线程

        self.loginButton.clicked.connect(self.connectToHost)
        self.setButton.clicked.connect(self.reSet)
        self.connWeb.clicked.connect(self.webShow)
        self.yeeLinkTime.timeout.connect(self.sendYeeLink)
        self.playButton.clicked.connect(self.playButtonUSBClicked)
        self.saveVideoButton.clicked.connect(self.saveVideoButtonClicked)
        self.buttonCSI.clicked.connect(self.playButtonCSIClicked)
        self.recvthread.warningSound.connect(self.playWarningSound)
        self.picUSBthread.savePic.connect(self.flushPicUSB)
        self.picCSIthread.savePicCSI.connect(self.flushPicCSI)
    def connectToHost(self):
        global HOST
        global PORT
        HOST = self.lineEditIP.text()
        PORT = self.lineEditPort.text()

        if HOST == "":
            QMessageBox.critical(self,u"错误",u"服务器地址错误")
            return
        if PORT == "":
            QMessageBox.critical(self,u"错误",u"服务器端口错误")
            return
        PORT = int(PORT)

        self.recvthread.start()
        evt.wait()          

        self.sockfd = socket(AF_INET,SOCK_STREAM) 
        self.sockfd.connect((HOST,PORT))        #主线程(发送指令)
        evt2.set()                          
    
        self.yeeLinkTime.start()   
    def reSet(self):
        self.lineEditIP.clear()
        self.lineEditPort.clear()
    def webShow(self):
        self.view = MyBrowser() #组合浏览器对象
        self.view.show()
    def sendYeeLink(self):
        global temp_2
        global humi
        global pressure
        global altitude
        if(temp_2 != 999):
            apiheaders  = {'U-ApiKey':'8b0df11c296b573b852b31a417a24e30','content-type':'application/json'}
            apiurlTemp  = ' http://api.yeelink.net/v1.0/device/356534/sensor/407095/datapoints'
            apiurlHum   =  'http://api.yeelink.net/v1.0/device/356534/sensor/404045/datapoints'
            apiurlAl    = 'http://api.yeelink.net/v1.0/device/356534/sensor/404046/datapoints'
            apiurlPre   = 'http://api.yeelink.net/v1.0/device/356534/sensor/404047/datapoints'
            payloadTemp = {'value':temp_2}
            payloadHum  = {'value':humi}
            payloadAL   = {'value':altitude}
            payloadPre  = {'value':pressure}
            requests.post(apiurlTemp,headers=apiheaders,data=json.dumps(payloadTemp))
            requests.post(apiurlHum,headers=apiheaders,data=json.dumps(payloadHum)) 
            requests.post(apiurlAl,headers = apiheaders,data = json.dumps(payloadAL))
            requests.post(apiurlPre,headers = apiheaders,data = json.dumps(payloadPre))
        #上传图片#
        if self.isPicOnLabel == True:
            url = ' http://api.yeelink.net/v1.0/device/356534/sensor/404085/photos'
            length = os.path.getsize('./image/imgUSB2.jpeg')
            image_data = open('./image/imgUSB2.jpeg', 'rb')
            request = urllib2.Request(url, data=image_data)
            request.add_header('U-ApiKey', '8b0df11c296b573b852b31a417a24e30')
            request.add_header('Content-Length', '%d' % length)
            res = urllib2.urlopen(request).read().strip()
    def keyPressEvent(self,event):
        if(event.isAutoRepeat()):
            return 
        if event.key() == Qt.Key_W:
            print "w"
            self.sockfd.send('w')    
        elif event.key() == Qt.Key_A:
            print "a"
            self.sockfd.send('a')
        elif event.key() == Qt.Key_S:
            print "s"
            self.sockfd.send('s')
        elif event.key() == Qt.Key_D:
            print "d"
            self.sockfd.send('d')
        elif event.key() == Qt.Key_I:
            print "i"
            self.sockfd.send('i')
        elif event.key() == Qt.Key_J:
            print "j"
            self.sockfd.send('j')
        elif event.key() == Qt.Key_K:
            print "k"
            self.sockfd.send('k')
        elif event.key() == Qt.Key_L:
            print "l"
            self.sockfd.send('l')
        elif event.key() == Qt.Key_F:
            print "f"
            self.sockfd.send('f')
        #键松开处理
    def keyReleaseEvent(self,event):
        if(event.isAutoRepeat()):
            return 
        if event.key() == Qt.Key_W:
            print "w0"
            self.sockfd.send('0')
        elif event.key() == Qt.Key_A:
            print "a0"
            self.sockfd.send('0')
        elif event.key() == Qt.Key_S:
            print "s0"
            self.sockfd.send('0')
        elif event.key() == Qt.Key_D:
            print "d0"
            self.sockfd.send('0')
    def playButtonCSIClicked(self):
        if(self.isCSI == False):
            self.isCSI = True
            self.buttonCSI.setIcon(self.ico5)
            self.picCSIthread.start()
        else:
            self.isCSI = False
            self.buttonCSI.setIcon(self.ico7)
    def playButtonUSBClicked(self):
        if(self.isPicOnLabel == False):
            self.isPicOnLabel = True
            self.playButton.setIcon(self.ico4)
            self.picUSBthread.start()
        else:
            self.isPicOnLabel = False
            self.playButton.setIcon(self.ico6)
    def saveVideoButtonClicked(self):
        a ='./video/'+ currentTime+'.jpeg'
        a.replace(' ','_')
        img = cv2.imread("./image/imgUSB2.jpeg")
        cv2.imwrite(a,img,[[int(cv2.IMWRITE_JPEG_QUALITY),100]])
        QMessageBox.information(self, u"提示",u"图像保存成功!")
    def flushPicCSI(self,val):
        if(self.isCSI == False):
            return 
        if val == 1:
            filename = "./image/imgCSI_detect.jpg"
        else:
            filename = "./image/imgCSI_nodetect.jpg"
        self.imgCSI.load(filename)
        self.imgCSI = self.imgCSI.scaled(300,200,Qt.KeepAspectRatio)
        self.labelCSI.setPixmap(QPixmap.fromImage(self.imgCSI))
    def flushPicUSB(self):
        if(self.isPicOnLabel == False):               #已经点击了"关闭视频"
            return
        img = cv2.imread("./image/imgUSB.jpeg")
        if distance < 350:
            cv2.putText( img, "Distance: "+str(distance) , ( 15, 15 ), cv2.FONT_HERSHEY_SIMPLEX, 0.5, ( 0, 0, 255 ), 2 )
        else:
            cv2.putText( img, "Distance: >350" , ( 15, 15 ), cv2.FONT_HERSHEY_SIMPLEX, 0.5, ( 0, 0, 255 ), 2 )
        cv2.putText( img, 'cm             Time:'+currentTime, (120, 15), cv2.FONT_HERSHEY_SIMPLEX, 0.5, ( 0, 0, 255 ), 2 )
        cv2.imwrite("./image/imgUSB2.jpeg",img,[int(cv2.IMWRITE_JPEG_QUALITY),100])

        if(not (self.img.load("./image/imgUSB2.jpeg"))):
            QMessageBox.information(self, u"错误",u"打开图像失败!")
            self.img = None
            return 

        self.scaledimg = self.img.scaled(700,500,Qt.KeepAspectRatio); 
        MainWindow.label[0].setPixmap(QPixmap.fromImage(self.scaledimg))
    def showTime(self):
        global currentTime
        now = int(time.time())
        timeArray = time.localtime(now)
        currentTime = time.strftime("%Y-%m-%d %H:%M:%S",timeArray)
       # self.clockLabel.setText(currentTime)
    def playWarningSound(self,val):
        if val == 0:
            self.buttonSafe.setIcon(self.icoSafe)
            self.buttonFog.setIcon(self.icoNoFog)
            self.buttonFlame.setIcon(self.icoNoFlame)
            return 
        os.system("mpg321 ./sound/beez.mp3 &")
        self.buttonSafe.setIcon(self.icoNoSafe)
        if val == 1:
            self.buttonFog.setIcon(self.icoFog)
            self.buttonFlame.setIcon(self.icoFlame)
        elif val == 2:
            self.buttonFog.setIcon(self.icoFog)
        else:
            self.buttonFlame.setIcon(self.icoFlame)

 #创建我的浏览器类
class MyBrowser(QWidget):
    def __init__(self,parent = None):
        super(MyBrowser,self).__init__(parent)
        self.createlayout()
        self.createconnection()
    def search(self):
        #unicode()   QString(包含中文) ==>> python 所能识别的string 类型
        address =unicode(self.addressbar.text().toUtf8(),'utf-8','ignore')
        #address = "http://www.yeelink.net/login?returl=%2Fuser%2Fdevices"
        if address:
            if address.find("://")  == -1:
                address = 'http://'+address
            url = QUrl(address)
            self.view.load(url)
            self.pageList.append(url)
            self.i += 1
    def createlayout(self):
        self.setWindowTitle(u"数据处理Yeelink")
        self.label =  QLabel(u"输入:")
        self.addressbar = QLineEdit()
        self.addressbar.setText("http://www.yeelink.net/login?returl=%2Fuser%2Fdevices")
        self.gobutton = QPushButton("&go")
        self.forwardbutton = QPushButton(">>")
        self.backbutton = QPushButton("<<")
        self.i = -1
        self.pageList = []
        hbox = QHBoxLayout()
        hbox.addWidget(self.backbutton)
        hbox.addWidget(self.forwardbutton)
        hbox.addWidget(self.label)
        hbox.addWidget(self.addressbar)
        hbox.addWidget(self.gobutton)

        self.view = QWebView()
        #self.label_2 = QLabel(u"距离",self.view)
        vbox = QVBoxLayout()
        vbox.addLayout(hbox)
        vbox.addWidget(self.view)

        self.setLayout(vbox)
    def createconnection(self):
        # returnPressed() 按下回车 或 lineEdit 失去焦点时发射
        self.connect(self.addressbar,SIGNAL("returnPressed()"),self.search)
        self.connect(self.addressbar,SIGNAL("returnPressed()"),self.addressbar,SLOT("selectAll()"))
        self.connect(self.gobutton,SIGNAL("clicked()"),self.search)
        self.connect(self.gobutton,SIGNAL("clicked()"),self.addressbar,SLOT("selectAll()"))
        self.forwardbutton.clicked.connect(self.forwardPage)
        self.backbutton.clicked.connect(self.backPage)
        self.view.page().setLinkDelegationPolicy(QWebPage.DelegateAllLinks)
        self.view.page().linkClicked.connect(self.linkClicked)
    def forwardPage(self):
        if(self.i < len(self.pageList)-1 ): 
             self.i += 1
             self.view.load(self.pageList[self.i])
    def backPage(self):
        if(self.i > 0):
            self.i -= 1
            self.view.load(self.pageList[self.i])
    def linkClicked(self,url):
        self.view.load(url)
        self.pageList.append(url)
        self.i += 1

    
evt  = threading.Event()    #main线程的开启   
evt2 = threading.Event()    #USB图片线程的开启 
evt3 = threading.Event()    #CSI图片线程的开启

app = QApplication(sys.argv)
main = MainWindow()
main.show()
sys.exit(app.exec_())


