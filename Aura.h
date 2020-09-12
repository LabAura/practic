#include <inttypes.h>
#include "wiringPi.h"

#ifndef _AURA_
#define _AURA_

//регистры каналов АЦП	
#define ch0 0x40	//свет
#define ch1 0x41	//температура ?
#define ch2 0x42	//ось X
#define ch3 0x43	//ось Y

//LCD константы
#define LCD_CHR  1 				// Режим отправки данных
#define LCD_CMD  0 				// Режим отправки комманд
#define LINE1  0x80 			// Адресс первой строки
#define LINE2  0xC0 			// Адресс второй строки
#define LCD_BACKLIGHT   0x08  	// Включение подсветки
#define ENABLE  0b00000100 		// Биты включения

//объявления переменных определяемых библиотекой.
//пины и параметры датчика веса
extern int DOUT;			//пин с которого будем читать данные с датчика веса

extern int PD_SCK;			//пин тактирования которым будем управлять датчиком
extern int GAIN;			//режим усиления для датчика (принимает значение только 64 или 128)
extern int GainBits;		//количество бит для установки режима усиления
						
extern int OFFSET;			//смещение веса. должно иметь такое значение, чтобы без нажатия на датчик 
							//значение функции getValue() было как можно ближе к нулю. 
						
extern int ReferenceUnit;	//единица измерения (в милиграммах). значение на которое будет делиться 
							//читаемое значение после вычитания смещения при использовании функции getWeight()
							//чтобы получать результат измерений в граммах, нужно указать ReferenceUnit = 1000						
//распиновка по board		
extern int SoundPin;		//пин датчика звука
extern int LightPin;		//пин датчика света

//кнопки
extern int btn_top;		//кнопка-верх
extern int btn_bottom;	//кнопка-низ
extern int btn_left;	//кнопка-лево
extern int btn_right;	//кнопка-право

//светодиоды	
extern int led1;
extern int led2;
extern int led3;
extern int led4;

//шаговый двигатель 
extern int StepPins[4];

//Адреса АЦП. Определяются в init_GPIO
extern int I2C_ADC; 
extern int I2C_LCD; 

//функции инициализации
char 	init_GPIO();									//инициализация всех пинов GPIO необходимых для работы стенда
void 	init_lcd(void);									//инициализация LCD дисплея 
int 	I2C_Detect(void);								//функция нахождения I2C-адреса LDC дисплея

//переменные и функции прерываний
extern char Sound_detected;
extern char Light_detected;
extern char btn_left_detected;
extern char btn_right_detected;
extern char btn_top_detected;
extern char btn_bottom_detected;

//функции прерываний
void 	SoundSensorInterrupt 	(void);
void 	LightSensorInterrupt 	(void);
void 	btn_leftInterrupt 		(void);
void 	btn_rightInterrupt 		(void);
void 	btn_topInterrupt 		(void);
void 	btn_bottomInterrupt 	(void);

int 	set_interrupt(int target,int state);			//установка прерывания на кнопку или датчик по определенному фронту.
int 	get_interrupt(int target);

//функции датчика температуры
char* 	get_temp_raw();									//чтение данных из файла датчика температуры
double 	get_temp();										//получение температуры с датчика

//функции LCD дисплея
void 	lcd_byte(int bits, int mode);					//техническая функция. не трожь
void 	lcd_toggle_enable(int bits);					//техническая функция. не трожь
void 	set_lcd_string_num(int i,int line);				//вывод числа на указанную строку LCD дисплея
void 	set_lcd_string_str(const char *s,int line);		//вывод строки на указанную строку LCD дисплея
void 	set_lcd_cleanup(int str); 							//Очистка экрана	


//функции модулей
char 	get_sound(void);									//Цифровой датчик звука
char 	get_digital_light(void);							//Цифровой датчик света
int 	get_analog_light(void);								//аналоговый датчик света
void 	set_diode(int diode,char state);					//установка выбранного светодиода в указанное состояние
int 	get_button(int btn);
void 	set_servo(char move[3]);							//установка сервопривода в указанное положение
char 	set_stepper(char Direction,char Unit,int Amount);	//поворот сервопривода в указанную сторону на указанный угол или кол-во шагов
int 	get_joystick_X(void);								//получение значения положения джойстика по оси X
int 	get_joystick_Y(void);								//получение значения положения джойстика по оси Y

//функции датчика веса. все функции для датчика веса сперриально начинаются с "hx_"
void 	hx_powerUp(void);								//включение датчика
void 	hx_powerDown(void);								//выключение датчика
void 	hx_reset(void);									//перезапуск датчика
char 	hx_isReady(void);								//ожидание готовности датчика
void 	hx_setGain(uint8_t gain);						//установка режима усиления датчика
void 	hx_tare(uint8_t);								//калибровка датчика перед началом измерений. автоматическое определение смещения
int32_t hx_read(void);									//чтение "сырого" значения с датчика
int32_t hx_readAverage(uint8_t);						//чтение ср. значения нескольких измерений датчика
void 	hx_setOffset(int32_t offset);					//установка значения смещения
int32_t hx_getOffset(void);								//получение значения смещения
int32_t hx_getRawValue(uint8_t);						//чтение "сырого" значения с датчика после вычета смещения
void 	hx_setReferenceUnit(float scale);				//установка единицы измерения
float 	hx_getReferenceUnit(void);						//получение единиц измерения
float 	hx_getUnits(uint8_t);							//получение значения с датчика в единицах измерения
#endif
