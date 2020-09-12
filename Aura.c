#include "Aura.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "wiringPi.h"
#include "wiringPiI2C.h"
#include <ctype.h>

//все #DEFINE описаны в Aura.h

//пины и параметры датчика веса
int DOUT=23;			//пин с которого будем читать данные с датчика веса
int PD_SCK=24;			//пин тактирования которым будем управлять датчиком
int GAIN=64;			//режим усиления для датчика (принимает значение только 64 или 128)
int GainBits=3;			//количество бит для установки режима усиления
						
int OFFSET=50000;		//смещение веса. должно иметь такое значение, чтобы без нажатия на датчик 
						//значение функции getValue() было как можно ближе к нулю. 
						
int ReferenceUnit=1000;	//единица измерения (в милиграммах). значение на которое будет делиться 
						//читаемое значение после вычитания смещения при использовании функции getWeight()
						//чтобы получать результат измерений в граммах, нужно указать ReferenceUnit = 1000						
//распиновка по board		
int SoundPin=21;		//пин датчика звука
int LightPin=22;		//пин датчика света

//кнопки
int btn_left=18;	//кнопка-лево	(1)
int btn_right=15;	//кнопка-право	(2)
int btn_top=19;		//кнопка-верх	(3)
int btn_bottom=16;	//кнопка-низ	(4)

//светодиоды	
int led1=31;
int led2=33;
int led3=35;
int led4=32;

//шаговый двигатель 
int StepPins[4] = {40,38,36,37};

//Адреса АЦП. Определяются в init_GPIO
int I2C_ADC=0; 
int I2C_LCD=0; 

//переменные и функции прерываний
char Sound_detected=0;
char Light_detected=0;
char btn_left_detected=0;
char btn_right_detected=0;
char btn_top_detected=0;
char btn_bottom_detected=0;

//функции прерываний
void SoundSensorInterrupt 	(void){Sound_detected=1;		}	//обработчик прерывания датчика звука
void LightSensorInterrupt 	(void){Light_detected=1;		}	//обработчик прерывания датчика света
void btn_leftInterrupt 		(void){btn_left_detected=1;		}	//обработчик прерывания левой кнопки 
void btn_rightInterrupt 	(void){btn_right_detected=1;	}	//обработчик прерывания правой кнопки 
void btn_topInterrupt 		(void){btn_top_detected=1;		}	//обработчик прерывания верхней кнопки 
void btn_bottomInterrupt 	(void){btn_bottom_detected=1;	}	//обработчик прерывания нижней кнопки 

int set_interrupt(int target,int state){	//установка прерываний на кнопки и датчики
	/*	СПИСОК ВОЗМОЖНЫХ ЦЕЛЕЙ ДЛЯ УСТАНОВКИ ПРЕРЫВАНИЯ:
	 * 	0 - датчик звука
	 * 	1 -	датчик света
	 * 	2 - левая кнопка
	 * 	3 - правая кнопка
	 * 	4 -	верхняя кнопка
	 * 	5 -	нижняя кнопка
	 * 
	 * 	ВОЗМОЖНЫЕ СОСТОЯНИЯ:
	 * 	0 - регистрация по растущему фронту
	 * 	1 - регистрация по падающему фронту
	 * 	2 - регистрация по обоим фронтам
	*/
	switch (target){
		case 0:
			if (state==0) wiringPiISR (SoundPin,INT_EDGE_RISING, 	&SoundSensorInterrupt);
			if (state==1) wiringPiISR (SoundPin,INT_EDGE_FALLING, 	&SoundSensorInterrupt);
			if (state==2) wiringPiISR (SoundPin,INT_EDGE_BOTH, 		&SoundSensorInterrupt);
			return 1;
		break;
		
		case 1:
			if (state==0) wiringPiISR (LightPin,INT_EDGE_RISING, 	&LightSensorInterrupt);
			if (state==1) wiringPiISR (LightPin,INT_EDGE_FALLING, 	&LightSensorInterrupt);
			if (state==2) wiringPiISR (LightPin,INT_EDGE_BOTH, 		&LightSensorInterrupt);
			return 1;
		break;
		
		case 2:
			if (state==0) wiringPiISR (btn_left,INT_EDGE_RISING, 	&btn_leftInterrupt);
			if (state==1) wiringPiISR (btn_left,INT_EDGE_FALLING, 	&btn_leftInterrupt);
			if (state==2) wiringPiISR (btn_left,INT_EDGE_BOTH, 		&btn_leftInterrupt);
			return 1;
		break;
		
		case 3:
			if (state==0) wiringPiISR (btn_right,INT_EDGE_RISING, 	&btn_rightInterrupt);
			if (state==1) wiringPiISR (btn_right,INT_EDGE_FALLING, 	&btn_rightInterrupt);
			if (state==2) wiringPiISR (btn_right,INT_EDGE_BOTH, 	&btn_rightInterrupt);
			return 1;
		break;
		
		case 4:
			if (state==0) wiringPiISR (btn_top,INT_EDGE_RISING, 	&LightSensorInterrupt);
			if (state==1) wiringPiISR (btn_top,INT_EDGE_FALLING, 	&LightSensorInterrupt);
			if (state==2) wiringPiISR (btn_top,INT_EDGE_BOTH, 		&LightSensorInterrupt);
			return 1;
		break;
		
		case 5:
			if (state==0) wiringPiISR (btn_bottom,INT_EDGE_RISING, 	&btn_bottomInterrupt);
			if (state==1) wiringPiISR (btn_bottom,INT_EDGE_FALLING, &btn_bottomInterrupt);
			if (state==2) wiringPiISR (btn_bottom,INT_EDGE_BOTH, 	&btn_bottomInterrupt);
			return 1;
		break;		
	}
	return 0;
}


int get_interrupt(int target){
	/*	СПИСОК ВОЗМОЖНЫХ ЦЕЛЕЙ ДЛЯ ЧТЕНИЯ СОСТОЯНИЯ ПРЕРЫВАНИЯ:
	 * 	0 - датчик звука
	 * 	1 -	датчик света
	 * 	2 - левая кнопка
	 * 	3 - правая кнопка
	 * 	4 -	верхняя кнопка
	 * 	5 -	нижняя кнопка
	 * 
	 * 	ВОЗМОЖНЫЕ СОСТОЯНИЯ:
	 * 	0 - нет прерывания
	 * 	1 - есть прерывание
	*/
	switch (target){
		int state=0;
		case 0:							//датчик звука
			state=Sound_detected;		//запоминаем состояние прерывания
			Sound_detected=0;			//обнуляем состояние прерывания
			return state;				//возвращаем состояние
		break;
		
		case 1:							//датчик света
			state=Light_detected;		//запоминаем состояние прерывания
			Light_detected=0;			//обнуляем состояние прерывания
			return state;				//возвращаем состояние
		break;
		
		case 2:							//левая кнопка
			state=btn_left_detected;	//запоминаем состояние прерывания
			btn_left_detected=0;		//обнуляем состояние прерывания
			return state;				//возвращаем состояние
		break;
		
		case 3:							//правая кнопка
			state=btn_right_detected;	//запоминаем состояние прерывания
			btn_right_detected=0;		//обнуляем состояние прерывания
			return state;				//возвращаем состояние
		break;
		
		case 4:							//верхняя кнопка
			state=btn_top_detected;		//запоминаем состояние прерывания
			btn_top_detected=0;			//обнуляем состояние прерывания
			return state;				//возвращаем состояние
		break;
		
		case 5:							//нижняя кнопка
			state=btn_bottom_detected;	//запоминаем состояние прерывания
			btn_bottom_detected=0;		//обнуляем состояние прерывания
			return state;				//возвращаем состояние
		break;
	}
	return -1;
}

//************************ 	ФУНКЦИИ РАБОТЫ С МОДУЛЯМИ  ************************
int I2C_Detect(void){			//определение I2C адреса
	//с помощью терминала Linux создаем файл с таблицей адресов в текущей папке 
	system("i2cdetect -y 1 > $PWD/adress_table.txt");	 	
	FILE * fin = fopen("adress_table.txt", "r");

	int str_len=100;						//длина текстового буфера	
	char rez[4]={0};						//cоздаем массив для хранения итоговых значений
	int letter_count=0;						//счетчик для записи символа в свободные позиции итогового буфера
	char buff[str_len];						//создаем пустой текстовый буфер
	 
	fgets (buff, str_len, fin);						//пропускаем первую строку. она нам неинтересна
	while(fgets (buff, str_len, fin) != NULL){  	//читаем построчно весь файл			
		strcpy(buff,&buff[4]);						//убираем первые 4 символа. они тоже не нужны. 
		for (unsigned int count=0;count<strlen(buff);count++){	//проходимся по всем остальным символам строки									
			char a=buff[count];
			if (isalnum(a)){
				 rez[letter_count]=a;
				 letter_count++;	
			}
		}		
	}	
	//возврат I2C адреса
	char ret[2]={0}; 					//переменная с парой символов, которую будем возвращать из функции
	if ((rez[0]=='4')&&(rez[1]=='8')){	//если первая пара символов = 48, то возвращаем вторую
		ret[0]=rez[2];					
		ret[1]=rez[3];
	}else{								//если нет, то возвращаем первую.
		ret[0]=rez[0];					
		ret[1]=rez[1];
	}
	if ((ret[0]==0)&&(ret[1]==0)) return 0;			//в случае неудачи, возвращаем false
	else return strtol(ret,NULL,16);				//преобразует строковое hex-значение в числовое.
}
				
char* get_temp_raw(void){						//получение "сырого текста" из файла датчика температуры
	int str_len=100;
	//поиск датчика температуры в папке с устройствами шины 1-Wire
	//и помещение пути найденного файла в текстовый файл в папке с программой
	system("find /sys/bus/w1/devices/ -name 28* >$PWD/temp_way.txt"); 

	static char temp_raw_result[200]={0}; 		//создаем текстовый массив в который положим результат 	
	char buff[100]; 							//создаем текстовый буфер	
	FILE * way_file = fopen("temp_way.txt", "r");
	fgets (buff, str_len, way_file);			//читаем файл c путем до датчика в буфер. в конце будет символ переноса строки
	for (int count=0;count<strlen(buff);count++) if (buff[count]=='\n') buff[count]=0 ;	//изящное решение по удалению конца строки
	fclose(way_file);							//закрываем файл c путем до датчика
	strcat(buff,"/w1_slave");					//добавление пути до файла к пути до папки
	//смотрим, какой получился путь до файла. вдруг интересно...
	//for (int count=0;count<strlen(buff);count++) printf("%c",buff[count]);	
	
	FILE * sensor_file = fopen(buff, "r");
	while(fgets (buff, str_len, sensor_file) != NULL){ 			
		strcat(temp_raw_result,buff);
	}  
	//можно раскомментить и глянуть на получившийся результат.
	//printf("\n");
	//for (int count=0;count<strlen(temp_raw_result)-1;count++)	printf("%c",temp_raw_result[count]);
	return temp_raw_result;
}

double get_temp(void){						//получение только значения температуры от датчика
	char* mas=get_temp_raw();				//получаем строку для парсинга
	char str_temp[10]={0};					//создаем строку для хранения числа с датчика в текстовой форме

	int str_pos=0;							//указатели на позиции в строках.
	int char_pos=0;

	while (mas[char_pos]!='t') char_pos++;	//читаем строку до тех пор, пока не найдем символ "t"
	char_pos+=2;							//перед числом идут символы "t=". они нам не нужны.

	for (int count=char_pos;count<strlen(mas);count++){	//от рассчитанной позиции get_temp и до конца файла останется только температура.													
		str_temp[str_pos]=mas[count];					//вытаскиваем ее и преобразуем в число. 
		str_pos++;		
	}
	double temp=(atof(str_temp)/1000);
	return temp;
	}

//LCD функции
char init_GPIO(void){
	if (wiringPiSetupPhys()== -1) return 0;
	I2C_ADC=wiringPiI2CSetup(0x48);	//создание подключения к АЦП
	
	//цифровые датчики
	pinMode(SoundPin, INPUT);		
	pinMode(LightPin, INPUT);	
	
	//кнопки
	pinMode(btn_top, 	INPUT);		
	pinMode(btn_bottom, INPUT);		
	pinMode(btn_left,	INPUT);		
	pinMode(btn_right, 	INPUT);

	//светодиоды	
	pinMode(led1, OUTPUT); 	
	pinMode(led2, OUTPUT); 	
	pinMode(led3, OUTPUT); 
	pinMode(led4, OUTPUT); 	
	
	//установка пинов шагового двигателя
	pinMode(StepPins[0],OUTPUT);
	pinMode(StepPins[1],OUTPUT);
	pinMode(StepPins[2],OUTPUT);
	pinMode(StepPins[3],OUTPUT);
	
	//тензо
	pinMode(PD_SCK, OUTPUT);
	pinMode(DOUT, INPUT);
	return 1;
}

void lcd_toggle_enable(int bits)   {			//тоже техническая штука. не трогай ее.
	delayMicroseconds(500);
	wiringPiI2CReadReg8(I2C_LCD, (bits | ENABLE));
	delayMicroseconds(500);
	wiringPiI2CReadReg8(I2C_LCD, (bits & ~ENABLE));
	delayMicroseconds(500);
}

void lcd_byte(int bits, int mode)   {			//техническая штука. не трогай ее.

  int bits_high;
  int bits_low;
  // uses the two half byte writes to LCD
  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

  // High bits
  wiringPiI2CReadReg8(I2C_LCD, bits_high);
  lcd_toggle_enable(bits_high);

  // Low bits
  wiringPiI2CReadReg8(I2C_LCD, bits_low);
  lcd_toggle_enable(bits_low);
}

void init_lcd(void){
	//"глупое подключение"
	//I2C_LCD=wiringPiI2CSetup(0x3f);	//создание подключения к LCD-дисплею
	//"умное подключение
	I2C_LCD=wiringPiI2CSetup(I2C_Detect());	//создание подключения к LCD-дисплею
	// Initialise display
	lcd_byte(0x33, LCD_CMD); // Initialise
	lcd_byte(0x32, LCD_CMD); // Initialise
	lcd_byte(0x06, LCD_CMD); // Cursor move direction
	lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
	lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
	lcd_byte(0x01, LCD_CMD); // Clear display
	delayMicroseconds(500);
}

void set_lcd_string_str(const char *s,int line)   {	//функция строки на дисплее для символов 
	if (line==1) lcd_byte(LINE1, LCD_CMD);
	if (line==2) lcd_byte(LINE2, LCD_CMD);
	//lcd_byte(line, LCD_CMD);
	while ( *s ) lcd_byte(*(s++), LCD_CHR);
}

void set_lcd_string_num(int i,int line)   {			//функция строки на дисплее для чисел 
	
	if (line==1) lcd_byte(LINE1, LCD_CMD);
	if (line==2) lcd_byte(LINE2, LCD_CMD);
	
	//lcd_byte(line, LCD_CMD);
	char array1[20];
	sprintf(array1, "%d",  i);
	set_lcd_string_str(array1, line);
}

void set_lcd_cleanup(int str)   {					//очистка дисплея.
	if (str==0){
		lcd_byte(0x01, LCD_CMD);
		lcd_byte(0x02, LCD_CMD);
		}
	if (str==1) lcd_byte(0x01, LCD_CMD);
	if (str==2) lcd_byte(0x02, LCD_CMD);	
}

char get_sound(void){								//цифровой датчик звука
	int SoundFactor=0;

	if (get_interrupt(0)==1) SoundFactor++;		//проверка флага срабатывания датчика звука по прерыванию
	Sound_detected=0;							//обнуление флага

	if (!digitalRead(SoundPin)) SoundFactor++;	
	//	простая проверка наличия сигнала с пина датчика звука на случай постоянного шума.
	//	(логический уровень остается высоким и прерывание неспособно его заметить т.к. оно
	//	работает только на изменение логического уровня) 	

	if (SoundFactor<1) return 0;				//если признаков срабатывания датчика не было, значит звука нет.
	else return 1;
}

char get_digital_light(void){						//цифровой датчик света
	int LightFactor=0;
	if (get_interrupt(1)==1) LightFactor++;		//проверка флага срабатывания датчика света по прерыванию
	Light_detected=0;							//обнуление флага

	if (!digitalRead(LightPin)) LightFactor++;	
	//	простая проверка наличия сигнала с пина датчика света на случай постоянного высокого уровня света.
	//	(логический уровень остается высоким и прерывание неспособно его заметить т.к. оно 
	//	работает только на изменение логического уровня) 	

	if (LightFactor<1) return 0;				//если признаков срабатывания датчика не было, значит света нет.
	else return 1;
}

int get_analog_light(void){							//аналоговый датчик света
	return (wiringPiI2CReadReg8(I2C_ADC,ch0));
}

int get_joystick_X(void){							//ось X джойстика
	
	return (wiringPiI2CReadReg8(I2C_ADC,ch2));
}

int get_joystick_Y(void){							//ось Y джойстика
	return (wiringPiI2CReadReg8(I2C_ADC,ch3));
}

int get_button(int btn){			//проверка состояния кнопки
	//кнопки:
	//1)btn_left	кнопка-лево
	//2)btn_right	кнопка-право
	//3)btn_top		кнопка-верх
	//4)btn_bottom	кнопка-низ
	if ((btn<1)||(btn>4)) return -1;
	else{
		switch (btn){
		case 1:
			return digitalRead(btn_left);			
		break;
		
		case 2:
			return digitalRead(btn_right);
		break;
		
		case 3:
			return digitalRead(btn_top);
		break;
		
		case 4:
			return digitalRead(btn_bottom);	
		break;								
		}
	}
	return -1;	//эта штука нужна на всякий случай и для успокоения компилятора
}

void set_diode(int diode,char state){			//установка состояния диода
	if ((diode<1)||(diode>4)){
		//cout<<"Diode "<<diode<<" is not exist.";
		return;
	}
	else{
		switch (diode){
		case 1:
			if (state==0) digitalWrite(led1,0);
			else digitalWrite(led1,1);
		break;
		
		case 2:
			if (state==0) digitalWrite(led2,0);
			else digitalWrite(led2,1);
		break;
		
		case 3:
			if (state==0) digitalWrite(led3,0);
			else digitalWrite(led3,1);
		break;
		
		case 4:
			if (state==0) digitalWrite(led4,0);
			else digitalWrite(led4,1);	
		break;						
		}
	}
}

void set_servo(char move[4]){	
	char command[40];
	command[0]=0;
	//"люблю с++ за Char строки. и за максимально удобное объединение строк"
	strcat(command,"echo P1-12=");	
	strcat(command,move);
	strcat(command," > /dev/servoblaster");	
	
	//cout<<command<<endl;
	system(command);
	//system("echo P1-12=",move," > /dev/servoblaster");
	delay(200);
}

char set_stepper(char Direction,char Unit,int Amount){
	if (Amount>0){	//проверка на правильность введенных данных
		int stepcounter=0;		//счетчик шагов
		int WaitTime = 5;		//время паузы перед каждым шагом в мс
		int steps=Amount;		//приравниваем количество необходимых шагов к локальной переменной	
		
		if (Unit==0){		//если мы передавали значение в функцию значение отклонения в градусах, то множим количество
								//Amount на значение равное количеству шагов для отклонения на 1 градус		
			float steps_per_angle=2048.0/360.0; //количество шагов на каждый градус. 
											//значение 2048.0 в принципе, можно поменять на другое
											//если количество шагов шаговика для совершения полного оборота от этого
			steps*=steps_per_angle;								
		}
		if (Direction==0){	//движение по часовой стрелке				
			while(stepcounter<steps){
				digitalWrite(StepPins[(stepcounter-1)%4],0);
				digitalWrite(StepPins[(stepcounter)%4],1);
				digitalWrite(StepPins[(stepcounter+1)%4],0);
				stepcounter++;
				delay(WaitTime);
			}			
		}else{					//движение против часовой стрелки
			while(steps>0){
				digitalWrite(StepPins[(steps-1)%4],0);
				digitalWrite(StepPins[(steps)%4],1);
				digitalWrite(StepPins[(steps+1)%4],0);
				steps--;
				delay(WaitTime);
			}	
		}
	return 1;	
	}else return 0;	//Direction и Unit так или иначе будут правильными, так что надо чекнуть только Amount	
}



//************************ 	ФУНКЦИИ ДАТЧИКА ВЕСА  ************************
void hx_powerUp(void) {
	digitalWrite(PD_SCK, 0);
	delayMicroseconds(100);
}

void hx_powerDown(void) {
	digitalWrite(PD_SCK, 0);
	digitalWrite(PD_SCK, 1);
	delayMicroseconds(100);
}

void hx_reset(void){
	hx_powerDown();
	hx_powerUp();
}

char hx_isReady(void){
	return digitalRead(DOUT) == 0;
}

void hx_setGain(uint8_t gain){
	switch(gain){
		case 128:
			GainBits = 1;
		break;
		case 64:
			GainBits = 3;
		break;
		case 32:
			GainBits = 2;
		break;
		default:

		break;
	}
	digitalWrite(PD_SCK, 0);
	hx_read();
}

void hx_tare(uint8_t times) {
	uint64_t sum = hx_readAverage(times);
	hx_setOffset(sum);
}

int32_t hx_read(void) {
	// wait for the chip to become ready
	while (!hx_isReady());

	int32_t data = 0;
	// pulse the clock pin 24 times to read the data
	for(uint8_t i = 24; i--;){
		delayMicroseconds(1);
		digitalWrite(PD_SCK, 1);
		delayMicroseconds(1);		
		data |= (digitalRead(DOUT) << i);		
		digitalWrite(PD_SCK, 0);
		delayMicroseconds(1);
	}
	// set the channel and the gain factor for the next reading using the clock pin
	for (int i = 0; i < GainBits; i++) {
		digitalWrite(PD_SCK, 1);
		delayMicroseconds(1);
		digitalWrite(PD_SCK, 0);
		delayMicroseconds(1);
	}

	if(data & 0x800000){
		data |= (long) ~0xffffff;
	}
	return data;
}

int32_t hx_readAverage(uint8_t times) {
	int64_t sum = 0;
	for (uint8_t i = 0; i < times; i++) {
		sum += hx_read();
	}
	return sum / times;
}

void hx_setOffset(int32_t offset) {
	OFFSET = offset;
}

int32_t hx_getOffset(void){
	return OFFSET;
}

int32_t hx_getRawValue(uint8_t times) {
	return hx_readAverage(times) - OFFSET;
}

void hx_setReferenceUnit(float scale) {
	ReferenceUnit = scale;
}

float hx_getReferenceUnit(void){
	return ReferenceUnit;
}

float hx_getUnits(uint8_t times) {
	return hx_getRawValue(times) / ReferenceUnit;
}
