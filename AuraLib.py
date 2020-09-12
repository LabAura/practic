#!/usr/bin/python
#coding=utf-8

import smbus
import os
import glob
import sys
import time
import RPi.GPIO as GPIO 
import traceback
import statistics

#настройки для работы с датчиком температуры
os.system('modprobe w1-gpio')
os.system('modprobe w1-therm')

#распиновка по board		
SoundPin=21		#пин датчика звука
LightPin=22		#пин датчика света

#кнопки
btn_left=18		#кнопка-лево	(1)
btn_right=15	#кнопка-право	(2)
btn_top=19		#кнопка-верх	(3)
btn_bottmom=16	#кнопка-низ		(4)

#светодиоды	
led1=31
led2=33
led3=35
led4=32	

#шаговый двигатель 
StepPins = [40,38,36,37]

#настройки дисплея
LCD_WIDTH = 16   	#длина строки дисплея	
LCD_CHR = 1 
LCD_CMD = 0 
LCD_LINE_1 = 0x80 	#адрес записи в первую строку
LCD_LINE_2 = 0xC0 	#адрес записи во вторую строку
LCD_BACKLIGHT  = 0x08  
#LCD_BACKLIGHT = 0x00  # Off	
ENABLE = 0b00000100 	
# Timing constants
E_PULSE = 0.0005
E_DELAY = 0.0005

#парсинг I2C адресов из таблицы 	
def I2C_Detect():	
	try:
		adress_table=os.popen("i2cdetect -y 1") 				#получаем таблицу с адресами 
		strings_list=adress_table.read().split('\n')			#разбиваем таблицу на строки		
		row_num=1								#счетчик строк
		Adresses=[]								#создаем массив для адресов
		for row in strings_list:				#проходимся по всему тексту
			#print(row)
			row=row[4:]							#убираем первые 4 символа из каждой строки. так или иначе, они бесполезны.
			if row_num>1: 						#в первой строке также нет полезной информации. пропускаем ее и работаем с остальными								
				cells=row.split()				#делим получившуюся строку на ячейки.
				for cell in cells:				#проверяем наличие чисел в ячейках
					if cell != "--":			#если текст в ячейке - не является "--", то там находится адрес
						Adresses.append(cell)	#добавляем адрес в массив
			row_num+=1	
		
		I2C_ADDR=0								#переменная для LCD адреса
		#в результате будет получено 2 адреса: 47 - АЦП, другой - LCD
		for cell in Adresses:			
			if cell != "48":
				I2C_ADDR = int(cell,16)			#перевод адреса из hex
				#print(cell)	
		return I2C_ADDR
	except:
		return False
		print("Cannot detect LDC I2C Address.")
		traceback.print_exc()
		#os.abort()
		
#открытие I2C интерфейсов
ADC_bus = smbus.SMBus(1) # Rev 2 Pi uses 1
LCD_bus = smbus.SMBus(1) # Rev 2 Pi uses 1

I2C_LCD=I2C_Detect()	#адрес LCD. на каждом стенде свой
I2C_ADC=0x48			#адрес АЦП. всегда один и тот же
		
#инициализация пинов
def init_GPIO():	
	try:	
		#GPIO.setwarnings(False)
		
		GPIO.setmode(GPIO.BOARD)
		
		#пины датчиков
		GPIO.setup(SoundPin, GPIO.IN)		
		GPIO.setup(LightPin, GPIO.IN)	
		
		#кнопки
		GPIO.setup(btn_top, 	GPIO.IN)		
		GPIO.setup(btn_bottmom, 	GPIO.IN)		
		GPIO.setup(btn_left, 	GPIO.IN)		
		GPIO.setup(btn_right, 	GPIO.IN)

		#светодиоды	
		GPIO.setup(led1, GPIO.OUT) #пин 19		
		GPIO.setup(led2, GPIO.OUT) #пин 18		
		GPIO.setup(led3, GPIO.OUT) #пин 16
		GPIO.setup(led4, GPIO.OUT) #пин 15	
	
		#шаговый двигатель
		for pin in StepPins:
			GPIO.setup(pin,GPIO.OUT)
			GPIO.output(pin, False)
		  
		return True
	except:
		print("GPIO initialisation failed")
		return False
		
def set_interrupt(target,state):		#установка прерывания
	''' СПИСОК ВОЗМОЖНЫХ ЦЕЛЕЙ ДЛЯ УСТАНОВКИ ПРЕРЫВАНИЯ:
		0 - датчик звука
	  	1 -	датчик света
	  	2 - левая кнопка
	  	3 - правая кнопка
	  	4 -	верхняя кнопка
	  	5 -	нижняя кнопка
	  
	  	ВОЗМОЖНЫЕ СОСТОЯНИЯ:
	  	0 - регистрация по растущему фронту
	  	1 - регистрация по падающему фронту
	  	2 - регистрация по обоим фронтам
	'''
	#можно еще устранения дребезга навалить.
	#GPIO.add_event_detect(LightPin, GPIO.RISING ,bouncetime=200)
	if target==0:
		if state==0:
			GPIO.add_event_detect(SoundPin, GPIO.RISING,bouncetime=200)
			return 1
		if state==1:
			GPIO.add_event_detect(SoundPin, GPIO.FALLING,bouncetime=200)
			return 1
		if state==2:
			GPIO.add_event_detect(SoundPin, GPIO.BOTH,bouncetime=200)
			return 1
			
	if target==1:
		if state==0:
			GPIO.add_event_detect(LightPin, GPIO.RISING,bouncetime=200)
			return 1
		if state==1:
			GPIO.add_event_detect(LightPin, GPIO.FALLING,bouncetime=200)
			return 1
		if state==2:
			GPIO.add_event_detect(LightPin, GPIO.BOTH,bouncetime=200)
			return 1
			
	if target==2:
		if state==0:
			GPIO.add_event_detect(btn_left, GPIO.RISING,bouncetime=200)
			return 1
		if state==1:
			GPIO.add_event_detect(btn_left, GPIO.FALLING,bouncetime=200)
			return 1
		if state==2:
			GPIO.add_event_detect(btn_left, GPIO.BOTH,bouncetime=200)
			return 1
			
	if target==3:
		if state==0:
			GPIO.add_event_detect(btn_right, GPIO.RISING,bouncetime=200)
			return 1
		if state==1:
			GPIO.add_event_detect(btn_right, GPIO.FALLING,bouncetime=200)
			return 1
		if state==2:
			GPIO.add_event_detect(btn_right, GPIO.BOTH ,bouncetime=200)
			return 1
			
	if target==4:
		if state==0:
			GPIO.add_event_detect(btn_top, GPIO.RISING,bouncetime=200)
			return 1
		if state==1:
			GPIO.add_event_detect(btn_top, GPIO.FALLING,bouncetime=200)
			return 1
		if state==2:
			GPIO.add_event_detect(btn_top, GPIO.BOTH,bouncetime=200)
			return 1
			
	if target==5:
		if state==0:
			GPIO.add_event_detect(btn_bottmom, GPIO.RISING,bouncetime=200)
			return 1
		if state==1:
			GPIO.add_event_detect(btn_bottmom, GPIO.FALLING,bouncetime=200)
			return 1
		if state==2:
			GPIO.add_event_detect(btn_bottmom, GPIO.BOTH,bouncetime=200)
			return 1
	return 0	
	
def get_interrupt(target):				#чтение состояния прерывания
	if target==0:
		return GPIO.event_detected(SoundPin)
	if target==1:
		return GPIO.event_detected(LightPin)
	if target==2:
		return GPIO.event_detected(btn_left)
	if target==3:
		return GPIO.event_detected(btn_right)
	if target==4:
		return GPIO.event_detected(btn_top)
	if target==5:
		return GPIO.event_detected(btn_bottmom)
	return -1
		
#инициализация дисплея
def init_lcd():
	# Initialise display
	lcd_byte(0x33,LCD_CMD) # 110011 Initialise
	lcd_byte(0x32,LCD_CMD) # 110010 Initialise
	lcd_byte(0x06,LCD_CMD) # 000110 Cursor move direction
	lcd_byte(0x0C,LCD_CMD) # 001100 Display On,Cursor Off, Blink Off 
	lcd_byte(0x28,LCD_CMD) # 101000 Data length, number of lines, font size
	lcd_byte(0x01,LCD_CMD) # 000001 Clear display
	time.sleep(E_DELAY)
		
#функции для работы дисплея
def lcd_byte(bits, mode):
	bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT
	bits_low = mode | ((bits<<4) & 0xF0) | LCD_BACKLIGHT
  # High bits
	LCD_bus.write_byte(I2C_LCD, bits_high)
	lcd_toggle_enable(bits_high)
  # Low bits
	LCD_bus.write_byte(I2C_LCD, bits_low)
	lcd_toggle_enable(bits_low)

def lcd_toggle_enable(bits):
	time.sleep(E_DELAY)
	LCD_bus.write_byte(I2C_LCD, (bits | ENABLE))
	time.sleep(E_PULSE)
	LCD_bus.write_byte(I2C_LCD,(bits & ~ENABLE))
	time.sleep(E_DELAY)

#вывод строки
def set_lcd_string(message,line):
	if line == 1:
		line=0x80
	if line == 2:
		line=0xC0
	message = message.ljust(LCD_WIDTH," ")
	lcd_byte(line, LCD_CMD)

	for i in range(LCD_WIDTH):
		lcd_byte(ord(message[i]),LCD_CHR)

#очистка экрана		
def set_lcd_cleanup(str_num):
	if str_num==0:
		lcd_byte(0x01,LCD_CMD) # 000001 Clear display line1
		lcd_byte(0x02,LCD_CMD) # 000001 Clear display line2
	if str_num==1:
		lcd_byte(0x01,LCD_CMD) # 000001 Clear display line1
	if str_num==2:
		lcd_byte(0x02,LCD_CMD) # 000001 Clear display line1


#датчик звука			
def get_sound(): 					
	sound_factor=0
	if (GPIO.input(SoundPin) == False):			#если датчик звука не активен
		sound_factor+=1	
	if (GPIO.event_detected(SoundPin) == True):	#если прерывание не срабатывает 
		sound_factor+=1							
	if sound_factor<1:							#если и датчик не работает и до этого прерываний не происходило, то тогда сигнала и правда нет
		return False
	else:										#если все таки срабатывает
		return True		

#цифровой датчик света
def get_digital_light():	
	light_factor=0
	if (GPIO.input(LightPin) == False):			#эта дичь инверсирована и при отсутствии света дает 1		
		light_factor+=1	
	if (GPIO.event_detected(LightPin) == True):	#если датчик звука не активен
		light_factor+=1				
	if light_factor<1:							#если и датчик не работает и до этого прерываний не происходило, то тогда сигнала и правда нет
		return False
	else:										#если все таки срабатывает
		return True
	
def get_analog_channel(channel,queue=0):		#техническая функция. знать о ней не стоит.
	ADC_bus.write_byte_data(I2C_ADC,0x40 | ((channel) & 0x03), queue)
	ADC_bus.read_byte(I2C_ADC)		#читаем один раз вхолостую.
									#не знаю почему, но это делает чтение стабильным
	data = ADC_bus.read_byte(I2C_ADC)	
	return data		
	
#аналоговый датчик света
def get_analog_light():	
	return get_analog_channel(0)	
	
#джойстик-x
def get_joystick_X():	
	return get_analog_channel(2)		
	
#джойстик-y
def get_joystick_Y():
	return get_analog_channel(3)	
	
def get_button(num):
	'''
	btn_left=18		#кнопка-лево	(1)
	btn_right=15	#кнопка-право	(2)
	btn_top=19		#кнопка-верх	(3)
	btn_bottmom=16	#кнопка-низ		(4)
	'''
	if (num<0 or num>4):
		return -1
	else:
		if num==1:
			return GPIO.input(btn_left)
		if num==2:
			return GPIO.input(btn_right)
		if num==3:
			return GPIO.input(btn_top)
		if num==4:
			return GPIO.input(btn_bottmom)
		
def set_diode(num,state):
	if (num<0 or num>4):
		return False
	else:
		if num==1:
			if state==0:
				GPIO.output(led1, False)
			elif state==1:
				GPIO.output(led1, True)
			else:
				print("unexpected statement")
		if num==2:
			if state==0:
				GPIO.output(led2, False)
			elif state==1:
				GPIO.output(led2, True)
			else:
				print("unexpected statement")
		if num==3:
			if state==0:
				GPIO.output(led3, False)
			elif state==1:
				GPIO.output(led3, True)
			else:
				print("unexpected statement")
		if num==4:
			if state==0:
				GPIO.output(led4, False)
			elif state==1:
				GPIO.output(led4, True)
			else:
				print("unexpected statement")
		return True

#серводвигатель	
def set_servo(move):
	if((move>=50)or(move<=250)):
		os.system("echo 1="+str(move)+" > /dev/servoblaster")
	else:
		return False
		
#шаговый двигатель	
def set_stepper(Direction,Unit,Amount): 
	#Direction-направление (0 - по часовой стрелке, 1 - против часовой)
	#Unit-выбор вида отклонения (0 - углы, 1- шаги)
	#Amount-количество шагов или значение угла
	if (((Direction==0) or (Direction==1)) and ((Unit==0) or (Unit==1)) and (Amount>0)):
		StepCounter = 0
		WaitTime = 0.01
		
		Steps=Amount	#приравниваем локальную переменную к значению отклонения (шаг или угол)
		#здесь будет код для преобразования углов в шаги
		
		if (Unit==0):
			Steps_per_angle=2048.0/360.0 #целочисленное деление сжирает аж 0.68 шага.
			print("Stesp per angle="+str(Steps_per_angle))
			#количество шагов при задании отклонения по углу= кол-во шагов на каждый угол * угол отклонения 
			Steps=Steps*Steps_per_angle
			print("Stesp per sector="+str(Steps))
						
		Steps=int(Steps)
		#StepCount = 4
		if Direction==0:
			print("clockwerk. Steps="+str(Steps))
			while(StepCounter<Steps):	
				print(StepCounter)		
				GPIO.output(StepPins[(StepCounter-1)%4], False)
				GPIO.output(StepPins[StepCounter%4], True)
				GPIO.output(StepPins[(StepCounter+1)%4], False)						 
				StepCounter += 1
				time.sleep(WaitTime)
			
			GPIO.output(StepPins[0], False)
			GPIO.output(StepPins[1], False)
			GPIO.output(StepPins[2], False)
			GPIO.output(StepPins[3], False)
			return True			
				
		elif Direction==1:
			print("counter-clockwerk. Steps="+str(Steps))
			while(Steps>0):	
				print(Steps)		
				GPIO.output(StepPins[(Steps-1)%4], False)
				GPIO.output(StepPins[Steps%4], True)
				GPIO.output(StepPins[(Steps+1)%4], False)						 
				Steps -= 1
				time.sleep(WaitTime)
			
			GPIO.output(StepPins[0], False)
			GPIO.output(StepPins[1], False)
			GPIO.output(StepPins[2], False)
			GPIO.output(StepPins[3], False)
			return True
		else:
			print("Wrong direction")
			return False
		
	else:
		return False
		print("Wrong input data")
		
#set_stepper(0,1,2000)

#функции для работы термометра
def get_temp_raw():
	try:
		base_dir = '/sys/bus/w1/devices/w1_bus_master1/'		#директория для поиска адреса датчика
		device_folder = glob.glob(base_dir + '28-*')[0]			#получение директрории датчика
		device_file = device_folder + '/w1_slave'				#обращение к нужному файлу (?)
	except:
		print("Cannot find temp sensor file")
		return False
	f = open(device_file, 'r')
	lines = f.readlines()
	f.close()
	return lines	

def get_temp():
    lines = get_temp_raw()
    while lines[0].strip()[-3:] != 'YES':
        time.sleep(0.2)
        lines = get_temp_raw()
    equals_pos = lines[1].find('t=')
    if equals_pos != -1:
        temp_string = lines[1][equals_pos+2:]
        temp_c = float(temp_string) / 1000.0
        return temp_c


#***********************************************************************
#работа с датчиком веса       
class HX711:
	#dout = dataPin, pd_sck=clockPin
    def __init__(self, dout=23, pd_sck=24, gain=64, bitsToRead=24):
        self.PD_SCK = pd_sck
        self.DOUT = dout

        GPIO.setwarnings(False)
        GPIO.setmode(GPIO.BOARD)
        GPIO.setup(self.PD_SCK, GPIO.OUT)
        GPIO.setup(self.DOUT, GPIO.IN)

        # The value returned by the hx711 that corresponds to your
        # reference unit AFTER dividing by the SCALE.
        self.REFERENCE_UNIT = 1

        self.GAIN = 0
        self.OFFSET = 1
        self.lastVal = 0
        self.bitsToRead = bitsToRead
        self.twosComplementThreshold = 1 << (bitsToRead-1)
        self.twosComplementOffset = -(1 << (bitsToRead))
        self.setGain(gain)
        self.read()

    def isReady(self):
        return GPIO.input(self.DOUT) == 0

    def setGain(self, gain):
        if gain is 128:
            self.GAIN = 1
        elif gain is 64:
            self.GAIN = 3
        elif gain is 32:
            self.GAIN = 2

        GPIO.output(self.PD_SCK, False)
        self.read()

    def waitForReady(self):
        while not self.isReady():
            pass

    def correctTwosComplement(self, unsignedValue):
        if unsignedValue >= self.twosComplementThreshold:
            return unsignedValue + self.twosComplementOffset
        else:
            return unsignedValue

    def read(self):
        self.waitForReady()

        unsignedValue = 0
        for i in range(0, self.bitsToRead):
            GPIO.output(self.PD_SCK, True)
            bitValue = GPIO.input(self.DOUT)
            #print(bitValue);
            
            GPIO.output(self.PD_SCK, False)
            unsignedValue = unsignedValue << 1
            unsignedValue = unsignedValue | bitValue

        # set channel and gain factor for next reading        
        #print(bin(unsignedValue),'=',unsignedValue)
        for i in range(self.GAIN):
            GPIO.output(self.PD_SCK, True)
            GPIO.output(self.PD_SCK, False)
		
        return self.correctTwosComplement(unsignedValue)

    def getValue(self):
        return self.read() - self.OFFSET

    def getWeight(self):
        value = self.getValue()
        value /= self.REFERENCE_UNIT
        return value

    def tare(self, times=10):
		read_sum=0
		i=0
		self.read()
		while i < times:
			read_sum+=self.read()               
			i+=1
		self.setOffset(read_sum/times)

    def setOffset(self, offset):
        self.OFFSET = offset

    def setReferenceUnit(self, reference_unit):
        self.REFERENCE_UNIT = reference_unit

    # HX711 datasheet states that setting the PDA_CLOCK pin on high
    # for a more than 60 microseconds would power off the chip.
    # I used 100 microseconds, just in case.
    # I've found it is good practice to reset the hx711 if it wasn't used
    # for more than a few seconds.
    def powerDown(self):
        GPIO.output(self.PD_SCK, False)
        GPIO.output(self.PD_SCK, True)
        time.sleep(0.0001)

    def powerUp(self):
        GPIO.output(self.PD_SCK, False)
        time.sleep(0.0001)

    def reset(self):
        self.powerDown()
        self.powerUp()

