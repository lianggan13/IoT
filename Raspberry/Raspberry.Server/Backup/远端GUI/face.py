#!/usr/bin/python
#coding:utf-8

import cv2
import cv2.cv as cv
    
#下载分类器文件，捕捉人脸特征和手机特征
face_cascade = cv2.CascadeClassifier( './data/lbpcascades/lbpcascade_frontalface.xml' )
phone_cascade = cv2.CascadeClassifier( './data/lbpcascades/iphone.xml' )

def face_detect():
    img = cv2.imread("./image/imgCSI.jpg")
    cv2.putText( img, "FPS: 4" , ( 10, 10 ), cv2.FONT_HERSHEY_SIMPLEX, 0.5, ( 0, 0, 255 ), 2 )
    cv2.putText( img, "NUM:",(170,10),cv2.FONT_HERSHEY_SIMPLEX,0.5,( 0, 0, 255), 2)
    cv2.imwrite("./image/imgCSI_nodetect.jpg",img,[int(cv2.IMWRITE_JPEG_QUALITY),100])
#转换为灰度图   
    gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
#直方图均衡处理
    gray = cv2.equalizeHist(gray)
 
#通过分类器得到rects
    faces = face_cascade.detectMultiScale(gray)
    
    if len(faces) == 0:
    	return 0
    #rects[:,2:] += rects[:,:2]
	#vis为img副本
	#vis = img.copy()
	#画矩形
    conut = 1
    cv2.putText( img, "FPS: 4" , ( 10, 10 ), cv2.FONT_HERSHEY_SIMPLEX, 0.5, ( 0, 0, 255 ), 2 )
    cv2.putText( img, "NUM:"+str(len(faces)),(170,10),cv2.FONT_HERSHEY_SIMPLEX,0.5,( 0, 0, 255), 2)
    for (x, y, w, h) in faces:
        cv2.rectangle(img, (x, y), (w+x, h+y), (0,255,0), 2)
        cv2.putText(img,"Face."+str(conut),(x,y),cv2.FONT_HERSHEY_SIMPLEX, 0.5, ( 0, 0, 255 ), 2)	
        conut += 1
    cv2.imwrite("./image/imgCSI_detect.jpg",img,[int(cv2.IMWRITE_JPEG_QUALITY),100]) 
    return 1

if __name__ == '__main__':
    face_detect()
