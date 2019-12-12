import os
import csv 
import time
import re
import serial
from time import sleep 
from datetime import datetime
from shutil import copyfile

#initializing variables
mph = 0
heart_rate = 0
set_speed = 0
temp=0
#Serial connection to max32 for temperature
#ser2 = serial.Serial('/dev/ttyUSB0',9600)
#Arduino uno serial connection for motor control
ser2 = serial.Serial('/dev/pts/3',115200)

#file maximum size limit of 20MB
size_limit = 20971520

#Create data log file
file = open("/home/pi/Desktop/final_code/data_log.csv", "a")
print "Log File Opened"
i=0
file_size = os.stat("/home/pi/Desktop/final_code/data_log.csv").st_size 
write_column = file.write("Time, Heart Rate, MPH, Set Speed, Temp\n") #Column of data we want to log.

if file_size ==0 :
     write_column
elif file_size != 0:
     write_column

c = 0
time_stamp = datetime.now()        
print "Writing Data To Log File.."
print "file size limit: 20 megabytes"
#HR=input("input max_HR: ")
#ser2.write(str(HR))
while True:
    file_size = os.stat("/home/pi/Desktop/final_code/data_log.csv").st_size 
#for logging to csv file 
    if file_size <= size_limit:
#*******************************************************
        #Temp_data=ser1.readline()
        motor_control = ser2.readline()
	#seperate data transmitted from motor control
        if(motor_control != None):
            data_contents = motor_control.split('\n')
            
            if (motor_control.find('mph') != -1):
                mph =  float(re.search(r'\d+', data_contents[0]).group())
                print mph
                c += 1
            if (motor_control.find('HR') != -1):
                heart_rate =  float(re.search(r'\d+', data_contents[0]).group())
                print heart_rate
                c += 1
            if (motor_control.find('set') != -1):
                set_speed =  float(re.search(r'\d+', data_contents[0]).group())
                print set_speed
                c += 1
            if (motor_control.find('temp') != -1):
                temp =  float(re.search(r'\d+', data_contents[0]).group())
                print temp
                c += 1
            if (c>=4):
                c = 0
                time.sleep(1)
            if (c==0):
                time_stamp = datetime.now()
                file.write(str(time_stamp)+","+str(heart_rate)+","+str(mph)+","+str(set_speed)+","+str(temp)+"\n")
                file.flush()
        
    elif file_size >= size_limit:
        #print "file size: " %file_size
        file.close()
        #print "Finished," "File Closed."
        copyfile("/home/pi/Desktop/final_code/data_log.csv", "/home/pi/Desktop/final_code/data_log1.csv")
        os.remove("/home/pi/Desktop/final_code/data_log.csv")
        #print "File Copied"
        file = open("/home/pi/Desktop/final_code/data_log.csv", "a")
        write_column = file.write("Time, Heart Rate, MPH, Set Speed, Temp\n")
        

