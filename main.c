#include <avr/io.h>
#include <util/delay.h>
#include "LCDlib.h"
#include "rtc.h"
#include "I2C.h"
#include <stdio.h>
#include <avr/interrupt.h>

static FILE mystdout = FDEV_SETUP_STREAM(LCD_putchar, NULL, _FDEV_SETUP_WRITE);
static int LCD_putchar(char c, FILE *stream)
{
	if (c == '\n') lcd_set_xy(1,0);
	else lcd_write8(c);
	return 0;
}

#define CHECK_PIN(REG, PIN) (!(REG & (1 << PIN)))///< checks weather button is pushed
#define SwPort PORTD///<port with buttons
#define SwDDR DDRD///<DDR for buttons
#define SwPIN PIND///<PIN for buttons
#define Sw0 PD0///<button 1
#define Sw1 PD1///<button 2
#define Sw2 PD2///<button 3
#define Sw3 PD3///<button 4
#define RELPort PORTB///<port with relays
#define REL1 0///<relay 1
#define REL2 1///<relay 2
#define REL12 2///<both relays


volatile uint8_t h,m,s,date,month,year; /*!< variables containing time and date values displayed on LCD and used in calculations*/
/// enum assigning values to names of days
/*! enum assigning values to names of days (numbers correspond to values from RTC)*/
 typedef volatile enum Day
 {
     PN,
     WT,
     SR,
     CZW,
     PT,
     SOB,
     ND
 } Day;
 Day day = PN;

volatile uint8_t lv = 5;///<confidence level for button software debouncing
///structure containing button properties
typedef volatile struct Button{
    uint8_t co;///<counter
	uint8_t rF;///<ready flag
	uint8_t f;///<button pushed flag
} Button;
Button bt[4];
volatile uint8_t relState[] = {0, 0};///<variable containing state of both of relays

/* display */
volatile uint8_t dispCo = 0;
volatile uint8_t dispF = 0;
char line1[17];
char line2[17];
///function used for displaying text from line1 and line2 on LCD
void update_disp(void)
{

	lcd_clear();
	lcd_set_xy(0,0);
	printf("%s\n%s",line1, line2);
	dispF = 0;
}

/* rel functionality */
uint8_t relONf = 0;
uint8_t relONrf = 1;
///used to turn on and off relays
void relFunc(uint8_t rel)
{
	if(rel < REL12)
	{
		if(relState[rel]) RELPort |= 1 << rel;
		else RELPort &= ~(1 << rel);
	}
	else
	{
		if (relONf)
		{
			RELPort |= (1 << REL1 | 1 << REL2);
		}
		else
		{
			RELPort &= ~(1 << REL1 | 1 << REL2);
		}
	}
	
}
///contains possible tasks
typedef enum Task
{
	MANUAL,
	WEEK
} Task;
Task task= MANUAL;

///contains states of the state machine
typedef enum State
{
	IDLE,
	MENU,
	SET_DATE_N_TIME,
	WEEK_PROG,
	WEEK_PROG_DAY
} State;
State state = IDLE;


///determines weather button is pressed and sets appropriate flags
void swCheck(uint8_t no)
{
	if(CHECK_PIN(SwPIN, no))
	{
		if(bt[no].co > lv && bt[no].rF)
		{
			bt[no].f = 1;
			bt[no].rF = 0;
		}
		else bt[no].co++;
	}
	else
	{
		bt[no].co = 0;
		bt[no].rF = 1;
	}
}
///time interrupt routine
ISR(TIMER0_COMP_vect )
{
	TCNT0 = 0;
	swCheck(0);
	swCheck(1);
	swCheck(2);
	swCheck(3);
	if(dispCo > 50)
	{
		dispF = 1;
		dispCo = 0;
	}
	else dispCo++;
}

/* displaying cursor and changing pointed value */
///assigns indexes to names used later in displaying cursor
 typedef volatile enum CursorPos
 {
	 HOUR,
	 MINUTE,
	 SECOND,
	 DATE,
	 MONTH,
	 YEAR,
	 DAY
 }CursorPos;
///structure for time and date display
 typedef struct TNDDisp
 {
	 volatile uint8_t dispCursorPos[7];
	 volatile CursorPos cPos;
 }TNDDisp;
 TNDDisp timeAndDateCursor =
 {
	 {
		 6,//hour position
		 9,//minute position
		 12,//second position
		 6,//date position
		 9,//month position
		 12,//year position
		 4//day position
	 },
	 HOUR
 };
 TNDDisp weekCursor = 
  {
	  {
		  5,//hour position
		  8,//minute position
		  6,//hour position
		  9,//minute position
		  0,
		  0,
		  0
	  },
	  HOUR
  };
  uint8_t weekDayCurs = 0;
///structure containing variables used in setting time and date functionality
 typedef struct TimeVar
 {
 volatile uint8_t h,m,s,date,month,year, day;
 }TimeVar;
 TimeVar tVar;
 uint8_t weekTime[4];//hours and minutes set for week mode 
 typedef enum WeekIt
 {
     H_ON,
     M_ON,
     H_OFF,
     M_OFF
 }WeekIt;
 WeekIt wI;
///used to store on which days relays should be switched on
 char weekDay[] = {' ', ' ', ' ', ' ', ' ', ' ', ' '};
 const char* const dayString[] = {"PN", "WT", "SR", "CZW", "PT", "SOB", "ND"};
///sets time values pointed by cursor on week display
 void changeWeekTimeVal()
 {
     switch(weekCursor.cPos)
     {
         case HOUR :
             if(weekTime[H_ON] == 23) weekTime[H_ON] = 0;
             else weekTime[H_ON]++;
             break;
         case MINUTE :
             if(weekTime[M_ON] == 59) weekTime[M_ON] = 0;
             else weekTime[M_ON]++;
             break;
         case SECOND ://hour off
             if(weekTime[H_OFF] == 23) weekTime[H_OFF] = 0;
             else weekTime[H_OFF]++;
             break;
         case DATE :
             if(weekTime[M_OFF] == 59) weekTime[M_OFF] = 0;
             else weekTime[M_OFF]++;
             break;
		default:
			break;
     }
 }
///sets  time and date poited by cursor on time and date display
void changeTNDVal()
 {
 switch(timeAndDateCursor.cPos)
 {
	case HOUR :
		if(tVar.h == 23) tVar.h = 0;
		else tVar.h++;
		break;
	case MINUTE :
		if(tVar.m == 59) tVar.m = 0;
		else tVar.m++;
		break;
	case SECOND :
		if(tVar.s == 59) tVar.s = 0;
		else tVar.s++;
		break;
	case DATE :
		if(tVar.date == 31) tVar.date = 1;
		else tVar.date++;
		break;
	case MONTH :
		if(tVar.month == 12) tVar.month = 1;
		else tVar.month++;
		break;
	case YEAR :
		if(tVar.year == 30) tVar.year = 0;
		else tVar.year++;
		break;
	case DAY :
		if(tVar.day == 6) tVar.day = 0;
		else tVar.day++;
		break;
 }
 }
///changes location of cursor on LCD adequately to state of the state machine
 void setCursor(){
     if(state == SET_DATE_N_TIME)
     {
		lcd_set_xy(0, timeAndDateCursor.dispCursorPos[timeAndDateCursor.cPos]);
     }
     else if(state == WEEK_PROG)
     {
		lcd_set_xy(0, weekCursor.dispCursorPos[weekCursor.cPos]);
     }
     else if(state == WEEK_PROG_DAY)
     {
		lcd_set_xy(0, weekDayCurs*2);
     }
 }
 

 // State machine function definition
///functionality of IDLE state
 void idleCode()
 {
     /* get data from RTC */
     rtc_get_time(&h, &m, &s);
     rtc_get_date(&day, &date, &month, &year);

     /* display */
     if(bt[0].f)
     {
         bt[0].f = 0;
         state = MENU;
     }
     else if(bt[1].f)
     {
         bt[1].f = 0;
         relState[REL1] = !relState[REL1];
         relFunc(REL1);
     }
     else if(bt[2].f)
     {
         bt[2].f = 0;
         relState[REL2] = !relState[REL2];
         relFunc(REL2);
     }
     else if(bt[3].f)
     {
         bt[3].f = 0;
         relState[REL1] = !relState[REL1];
         relFunc(REL1);
         relState[REL2] = !relState[REL2];
         relFunc(REL2);
     }
     else
     {
         if(!task) sprintf(line1, "%02d:%02d:%02d MANUAL",h,m,s );
         else sprintf(line1, "%02d:%02d:%02d WEEK",h,m,s );
         sprintf(line2, "%02d-%02d-%02d %s",date,month,year, dayString[day]);
         state = IDLE;
     }
 }
///functionality of MENU state
void menuCode()
 {
     /* display */
     if(bt[0].f)
     {
         bt[0].f = 0;
         state = IDLE;
     }
     else if(bt[1].f)
     {
         bt[1].f = 0;
		 tVar.h = h;
		 tVar.m = m;
		 tVar.s = s;
		 tVar.date = date;
		 tVar.month = month;
		 tVar.year = year;
		 tVar.day = day;
         state = SET_DATE_N_TIME;
     }
     else if(bt[2].f)
     {
         bt[2].f = 0;
         state = WEEK_PROG;
     }
     else if(bt[3].f)
     {
         bt[3].f = 0;
		 task = MANUAL;
         state = IDLE;
     }
     else
     {
         sprintf(line1, "1.TIME/DATE");
         sprintf(line2, "2.WEEK 3.MANUAL");
         state = MENU;
     }
 }
///functionality of SET_DATE_N_TIME state
void setDateNTime()
 {
     /* display */
     if(bt[0].f)//accept
     {
         bt[0].f = 0;
         rtc_set_time(tVar.s, tVar.m, tVar.h, 1, 0);//set time
         rtc_set_date(tVar.day, tVar.date, tVar.month, tVar.year);//set date
         state = IDLE;
		 timeAndDateCursor.cPos = HOUR;
     }
     else if(bt[1].f)//which value
     {
         bt[1].f = 0;
         state = SET_DATE_N_TIME;
         if(timeAndDateCursor.cPos == DAY) timeAndDateCursor.cPos = HOUR;
         else timeAndDateCursor.cPos++;
     }
     else if(bt[2].f)//change value
     {
         bt[2].f = 0;
		 changeTNDVal();
         state = SET_DATE_N_TIME;

     }
     else if(bt[3].f)//cancel
     {
         bt[3].f = 0;
         state = MENU;
		 timeAndDateCursor.cPos = HOUR;
     }
     else
     {
         if(timeAndDateCursor.cPos < DATE)
		 {
			sprintf(line1, "TIME %02d:%02d:%02d", tVar.h, tVar.m, tVar.s);
			sprintf(line2, "DATE %02d-%02d-%02d", tVar.date, tVar.month, tVar.year);
		 }
		 else if(timeAndDateCursor.cPos < DAY)
		 {
			sprintf(line1, "DATE %02d-%02d-%02d", tVar.date, tVar.month, tVar.year);
			sprintf(line2, "DAY %s", dayString[tVar.day]);
		 }
		 else
		 {
			sprintf(line1, "DAY %s", dayString[tVar.day]);
			sprintf(line2, "TIME %02d:%02d:%02d", tVar.h, tVar.m, tVar.s);

		 }
		 //setCursor();
         state = SET_DATE_N_TIME;
     }
 }
///functionality of WEEK_PROG state
void weekProg()
 {
     /* display */
     if(bt[0].f)//accept
     {
         bt[0].f = 0;
         state = WEEK_PROG_DAY;
     }
     else if(bt[1].f)//which value
     {
         bt[1].f = 0;
		 if(weekCursor.cPos == DATE) weekCursor.cPos = HOUR;
		 else weekCursor.cPos++;
         state = WEEK_PROG;
     }
     else if(bt[2].f)//change value
     {
         bt[2].f = 0;
		 changeWeekTimeVal();
         state = WEEK_PROG;
     }
     else if(bt[3].f)//cancel
     {
         bt[3].f = 0;
         weekTime[H_ON] = 0;
         weekTime[M_ON] = 0;
         weekTime[H_OFF] = 0;
         weekTime[M_OFF] = 0;
         state = MENU;
     }
     else
     {
         if(weekCursor.cPos < SECOND)
         {
             sprintf(line1, "ON: %02d:%02d", weekTime[H_ON], weekTime[M_ON]);
             sprintf(line2, "OFF: %02d:%02d", weekTime[H_OFF], weekTime[M_OFF]);
         }
         else
         {
             sprintf(line2, "ON: %02d:%02d", weekTime[H_ON], weekTime[M_ON]);
             sprintf(line1, "OFF: %02d:%02d", weekTime[H_OFF], weekTime[M_OFF]);
         }

         state = WEEK_PROG;
     }
 }
///functionality of WEEK_PROG_DAY state
void weekProgDay()
 {
     /* display */
     if(bt[0].f)//accept
     {
         task = WEEK;
         bt[0].f = 0;
         state = IDLE;
		 task = WEEK;
     }
     else if(bt[1].f)//which value
     {
         bt[1].f = 0;
		 if(weekDayCurs == 6) weekDayCurs = 0;
		 else weekDayCurs++;
         state = WEEK_PROG_DAY;
     }
     else if(bt[2].f)//change(increment) value
     {
         bt[2].f = 0;
		 if(weekDay[weekDayCurs] == ' ') weekDay[weekDayCurs] = 'x';
		 else weekDay[weekDayCurs] = ' ';
         state = WEEK_PROG_DAY;
     }
     else if(bt[3].f)//cancel
     {
         bt[3].f = 0;
         state = WEEK_PROG;
		 for (uint8_t it = 0; it<7; ++it)
		 {
		 weekDay[it] = ' ';
		 }
		 task = MANUAL;
     }
     else
     {
         sprintf(line1, "%c %c %c %c %c %c %c", weekDay[0], weekDay[1], weekDay[2], weekDay[3], weekDay[4], weekDay[5], weekDay[6]);
         sprintf(line2, "P W S C P S N");
         state = WEEK_PROG_DAY;
     }
 }
///sets flags for relays
void weekTask()
 {
 if(weekDay[day] == 'x')
 {
	if(h >= weekTime[H_ON] && m >= weekTime[M_ON] && relONrf)
	{
		relONf = 1;
		relONrf = 0;
	}
	if(h >= weekTime[H_OFF] && m >= weekTime[M_OFF] && !relONrf)
	{
		relONf = 0;
		relONrf = 1;
	}
 }
 }
/// main function
/*! contains all initializations and control core functionality of program*/
 int main(void)
{
    /* LCD init */
	DDRA = 0xFF;
	lcdinit();
	stdout = &mystdout;

	/* I2C init */
	I2C_init();

	/* RTC init */
	rtc_init();
	rtc_set_time(0,2,19,1,0);//set time
	rtc_set_date(2,6,6,17);//set date

	/* switches init */
	SwDDR &= ~(1 << Sw0 | 1 << Sw1 | 1 << Sw2 | 1 << Sw3);//set output for buttons
	SwPort |= 1 << Sw0 | 1 << Sw1 | 1 << Sw2 | 1 << Sw3;//set pull-up resistors

	/* Rel init */
	DDRB |= 0x0F;

	/* Timer INT init */
	OCR0 = 10;//compare value
	TCCR0 |= (1 << CS02) | (1 << CS00);//prescaler 1024
	TIMSK = (1 << OCIE0);//compare interrupt enable
	sei();//en global int

    while (1)
    {
		switch(state){
			case IDLE :
				idleCode();
				break;
			case MENU :
				menuCode();
				break;
			case SET_DATE_N_TIME :
				setDateNTime();
				break;
			case WEEK_PROG :
				weekProg();
				break;
			case WEEK_PROG_DAY :
				weekProgDay();
				break;
		}

		if(dispF)
        {
            update_disp();
            if(state == SET_DATE_N_TIME  || state == WEEK_PROG || state == WEEK_PROG_DAY)
			{
			 setCursor();
			 lcd_write_instr(0x0F);//set blinking 
			}
			else
			{
			 lcd_write_instr(0x0C);//unset blinking 
			}
        }
		if(task == WEEK) 
		{
			weekTask();
			relFunc(REL12);
		}
    }
}