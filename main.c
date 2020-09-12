#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Aura.h"
#include "wiringPi.h"
#include "wiringPiI2C.h"

int main (void)
{				

	init_lcd();
	set_lcd_string_str("work",1);
	delay(2000);
	set_lcd_cleanup();
	
}

