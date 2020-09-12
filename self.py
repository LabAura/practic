#!/usr/bin/python
#coding=utf-8

import commands
import os
import sys
import glob
import smbus
import time
import AuraLib 
import RPi.GPIO as GPIO
from time import sleep

#функции для удобства работы	
def PrintOption(line):
	#\x7F - стрелка вправо, \x7E - стрелка вправо
	AuraLib.set_lcd_cleanup(2) # 000001 Clear display line2
	if line==1:	
		AuraLib.set_lcd_string("\x7F Sound sensor \x7E",2) 
	if line==2:
		AuraLib.set_lcd_string("\x7F Light sensor \x7E",2)
	if line==3:
		AuraLib.set_lcd_string("\x7F Heat sensor  \x7E",2)	
	if line==4:
		AuraLib.set_lcd_string("\x7F Diodes       \x7E",2)	
	if line==5:
		AuraLib.set_lcd_string("\x7F Servomotor   \x7E",2)	
	if line==6:
		AuraLib.set_lcd_string("\x7F Stepper motor\x7E",2)	
	if line==7:
		AuraLib.set_lcd_string("\x7F Joystick     \x7E",2)	
	if line==8:
		AuraLib.set_lcd_string("\x7F Weigher      \x7E",2)	
	sleep(0.3)
		
def RunOption(line):
	if line==1:		
		option_sound() 		
	if line==2:
		option_light()
	if line==3:
		option_temp()	
	if line==4:
		option_diodes()
	if line==5:
		option_servo()
	if line==6:
		option_stepper()
	if line==7:
		option_joystick()	
	if line==8:
		option_weigher()

#функции вариантов выбора:
			
def option_sound(): #сейчас срабатывает только на изменение звука на громкий
					#т.е. если звук свегда будет громким, функция этого не просечет
	AuraLib.set_lcd_cleanup(0)
	AuraLib.set_lcd_string("Sound sensor",1)
	
	while(AuraLib.get_interrupt(4) == False):				#пока не нажимаем на кнопку снова			
		AuraLib.set_lcd_cleanup(2)
		if AuraLib.get_sound()==True:							#если и датчик не работает и до этого прерываний не происходило			
			AuraLib.set_lcd_string("Sound",2)			
		else:													#если все таки срабатывает			
			AuraLib.set_lcd_string("No sound",2)
	AuraLib.set_lcd_cleanup(0)

def option_light():
	AuraLib.set_lcd_cleanup(0)	
	while(AuraLib.get_interrupt(4) == False):
		AuraLib.set_lcd_cleanup(0)
		AuraLib.set_lcd_string(("An:"+str(AuraLib.get_analog_light())),1)			
		if AuraLib.get_digital_light() == True:
			AuraLib.set_lcd_string("Dg: Light",2)
		else:
			AuraLib.set_lcd_string("Dg: No_Light",2)
		time.sleep(0.4)
	AuraLib.set_lcd_cleanup(0)
	
	 
def option_temp():
	AuraLib.set_lcd_cleanup(0)	
	AuraLib.set_lcd_string("Temp. sensor",1)
	
	while(AuraLib.get_interrupt(4) == False):
		AuraLib.set_lcd_cleanup(2)
		AuraLib.set_lcd_string(str(AuraLib.get_temp()),2)
		
def option_diodes():
	AuraLib.set_lcd_cleanup(0)	
	AuraLib.set_lcd_string("Diodes wave",1)
	
	while(AuraLib.get_interrupt(4) == False):
		sleep(0.3)
		AuraLib.set_diode(1,1)
		sleep(0.3)
		AuraLib.set_diode(2,1)
		sleep(0.3)
		AuraLib.set_diode(3,1)
		sleep(0.3)
		AuraLib.set_diode(4,1)
		sleep(0.3)
		AuraLib.set_diode(1,0)
		sleep(0.3)
		AuraLib.set_diode(2,0)
		sleep(0.3)
		AuraLib.set_diode(3,0)
		sleep(0.3)
		AuraLib.set_diode(4,0)
		sleep(0.3)
		
	AuraLib.set_diode(1,0)
	AuraLib.set_diode(2,0)
	AuraLib.set_diode(3,0)
	AuraLib.set_diode(4,0)
	AuraLib.set_lcd_cleanup(1)			
	
def option_stepper():
	AuraLib.set_lcd_cleanup(0)
	AuraLib.set_lcd_string("Stepper motor",1)
	AuraLib.set_lcd_string("Top BTN to leave",2)
	while(AuraLib.get_interrupt(4) == False):
		AuraLib.set_stepper(1,0,5)

def option_joystick():
	while(AuraLib.get_interrupt(4) == False):
		AuraLib.set_lcd_cleanup(0)
		AuraLib.set_lcd_string(("X-asis"+str(AuraLib.get_joystick_X())),1)
		AuraLib.set_lcd_string(("Y-asis"+str(AuraLib.get_joystick_Y())),2)
		time.sleep(0.3)
	
def option_weigher():
	AuraLib.set_lcd_cleanup(0)	
	AuraLib.set_lcd_string("Weigher test",1)	
	AuraLib.set_lcd_string("Wait a sec",2)
	#датчик веса
	hx = AuraLib.HX711()
	hx.setGain(64)
	hx.tare()
	hx.setReferenceUnit(1000)
	hx.reset()
	
	while(GPIO.event_detected(AuraLib.btn_top) == False):
		AuraLib.set_lcd_cleanup(2)
		AuraLib.set_lcd_string("Weight="+str(hx.getWeight())+" gr",2)						
	AuraLib.set_lcd_cleanup(0)

def option_servo():
	AuraLib.set_lcd_cleanup(0)
	AuraLib.set_lcd_string("Servomotor",1)
	AuraLib.set_lcd_string("Top BTN to leave",2)
	while(AuraLib.get_interrupt(4) == False):
		AuraLib.set_servo(50)
		time.sleep(2)
		AuraLib.set_servo(250)
		time.sleep(2)

def Hello():				#функция приветствия
	AuraLib.set_lcd_cleanup(0)	
	AuraLib.set_lcd_string("Hi, i'm Aura",1) 
	sleep(1)
	


def GoodBye():				#завершение работы программы
	AuraLib.set_lcd_cleanup(0)
	AuraLib.set_lcd_string("GoodBye",1)
	sleep(2) 
	AuraLib.set_lcd_cleanup(0)
	sleep(1) 
	GPIO.cleanup()
	os.abort()
	
def line_dec(line):
	line-=1
	if line<1:					#если строка была первой, то переходим к концу списка
		line=8
	return line
	
def line_inc(line):
	line+=1
	if line>8:					#если ст0рока была последней, то переходим к началу списка
		line=1
	return line

#номер строки опций с которой начинается выбор модулей
line=1;

#основная функция:	
try:	
	AuraLib.init_GPIO()
	AuraLib.init_lcd()
	if ((AuraLib.get_button(1)==False) and (AuraLib.get_button(2)==False)):
		Hello()	
		AuraLib.set_interrupt(0,0)
		AuraLib.set_interrupt(1,0)
		AuraLib.set_interrupt(2,1)
		AuraLib.set_interrupt(3,1)
		AuraLib.set_interrupt(4,1)
		AuraLib.set_interrupt(5,1)
		while True:			#крутимся в цикле	
			AuraLib.set_lcd_string("  Choose module",1)		
			if (AuraLib.get_interrupt(2) == True):								#если нажата кнопка влево
				line=line_dec(line)													#переходим на предыдущую строку
				#print(line)					
															
			if (AuraLib.get_interrupt(3) == True):							#если нажата кнопка вправо
				line=line_inc(line)													#переходим на следующую строку
				#print(line)			
			PrintOption(line)														#выводим строку на LCD		
			#тут нужно реально быстро нажать верхнюю и нижнюю кнопку одновременно
			top_pressed=AuraLib.get_interrupt(4)	
			bottmom_pressed=AuraLib.get_interrupt(5)
			#если нажаты верхняя и нижняя кнопки, то выходим
			if (top_pressed == True) and (bottmom_pressed == True):
				print("Closing program")
				GoodBye()
			elif top_pressed == True:
				RunOption(line)

except KeyboardInterrupt:
	AuraLib.set_lcd_cleanup(0)
	#GPIO.cleanup()
	
