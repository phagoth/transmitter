/*
 * Transmitter.c
 *
 * Created: 15.02.2016 6:32:23
 *  Author: Сергей Сивак
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 8000000UL  // 8 MHz
#include <avr/delay.h>
int a=0, speed, i=0,SendByteTWI =0b11000101, StartTWI=0b11100101, StopTWI=0b11010101, Ed, Des, Sot, b, c, e; // Переменные общего назначения и для работы с TWI
int Decoding[10] = {189, 9, 117, 109, 201, 236, 252, 13, 253, 237};//Массив с дешифром для семисегментного индикатора
int PD5p, PD6p, PD0p=1, PD1p=1, speeder=16080;// Переменные для работы с энкодером
int Effect, Diod[10]={16,32,8,1,4,128,64,1,4,128},Schet;// Переменные для создания эффекта на неиспользуемом 4 цифровом индикаторе.
int Step, Dir, stop=1, Kn1, Kn2, stopp, ravno;//Переменные для работы с двигателем, кнопками и модулем приемника 

ISR (TIMER0_OVF_vect )
{
	if (stop==1)//замедление
	{  
	  if (speeder<32696)
    {
      speeder += 268;
    } 
    else 
    {
      Kn1=0; Kn2=0; speeder=32697; stopp=1; stop=0;
    }
	}
	
  if ((stopp!=1)&&(ravno>=speeder)&&(speeder < 32697))
  {
    if (PINB==(2|PINB))//Инверсия бита
    {
      PORTB=(~2)&PINB;
      ravno=0;
    }
    else 
    {
      PORTB=2|PINB;
      ravno=0;
    }
  }
  
  ravno++;
}

ISR (TWI_vect) //Обработчик прерывания модуля TWI
{	
  if (TWSR==0x08)
  {
    TWDR=0x70;
    TWCR=SendByteTWI; // Запускаем передачу адреса
  }
	if (TWSR==0x18)//Передаем пустой байт инструкций (субадреса)
  { 
	  TWDR=0b00000000;
	  TWCR=SendByteTWI; //Запускаем передачу байта данных
	}
	if ((TWSR==0x28)&&(i==0))//Передаем байт управления отображением
  {    
    TWDR=0b00110111;
    TWCR=SendByteTWI; //Запускаем передачу байта данных
    i++;
  }
  if ((TWSR==0x28)&&(i==1))//Передаем байт на первый индикатор
  {  
    TWDR=Ed;
    TWCR=SendByteTWI; //Запускаем передачу байта данных
    i++;
  }
  if ((TWSR==0x28)&&(i==2))
  {
	  TWDR=Des;
	  TWCR=SendByteTWI; //Запускаем передачу байта данных
	  i++;
  }
	if ((TWSR==0x28)&&(i==3))
  {
    TWDR=Sot;
    TWCR=SendByteTWI; //Запускаем передачу байта данных
    i++;
  }
  if ((TWSR==0x28)&&(i==4))//Эффект на неиспользуемом разряде
  {
		if ((Schet>=2*speed)&&(speed!=0))
		{
      TWDR=Diod[Effect];
			TWCR=SendByteTWI; //Запускаем передачу байта данных
			Schet=0;
			Effect++;
		}
		 	 
		i++;
		 
		Schet++;
		
    if (Effect==10)
		{
      Effect=0;
		}
  }
  if ((TWSR==0x28)&&(i==5))
  {
    TWCR=StopTWI; //Запускаем передачу  stop
    i=0;
  }
}
	


int main(void)
{
  // TWI initialization
  // Bit Rate: 100,000 kHz
  TWBR=0x20;
  // Two Wire Bus Slave Address: 0x5
  // General Call Recognition: On
  TWAR=0x0B;
  // Generate Acknowledge Pulse: On
  // TWI Interrupt: On
  TWCR=0x45;
  TWSR=0x00;

  // Timer/Counter 0 initialization
  // Clock source: System Clock
  // Clock value: 7,813 kHz
  TCCR0=0x01;
  TCNT0=0x00;

  // Timer(s)/Counter(s) Interrupt(s) initialization
  TIMSK=0x01;

  // Инициализация портов 0- установка бита порта на вход, 1- на выход.
  PORTB=0xff;
  DDRB=0xff;
  PORTD=0xff;	
  DDRD=0x00;
    // Global enable interrupts

  asm volatile ("sei");
  _delay_ms(100);//Общая задержка перед выполнением программы

  if((32|PIND)==PIND) {PD5p=1;} else {PD5p=0;}//Инициализация энкодера Если значения одинаковы значит на 5 ножке лог.1 иначе 0
  if((64|PIND)==PIND) {PD6p=1;} else {PD6p=0;} //(энкодер подключен к 5 и 6 ножке порта D)



  while(1)
  { 
    _delay_ms(4);
    // обработчик энкодера(энкодер подключен к 5 и 6 ножке порта D)
    // increment
    if (((PD6p==1)&&(PD5p==0)&&((32|PIND)!=PIND)&&((64|PIND)!=PIND))|((PD6p==0)&&(PD5p==0)&&((32|PIND)==PIND)&&((64|PIND)!=PIND))|((PD6p==0)&&(PD5p==1)&&((32|PIND)==PIND)&&((64|PIND)==PIND))|((PD6p==1)&&(PD5p==1)&&((32|PIND)!=PIND)&&((64|PIND)==PIND)))
    {
      if (speeder<32696)
      {
        speeder += 268;
      }
      else
      {
        speeder = 32697;
      }// для изменения положения нуля
    } 
    // decrement
    if (((PD6p==0)&&(PD5p==0)&&((32|PIND)!=PIND)&&((64|PIND)==PIND))|((PD6p==1)&&(PD5p==0)&&((32|PIND)==PIND)&&((64|PIND)==PIND))|((PD6p==1)&&(PD5p==1)&&((32|PIND)==PIND)&&((64|PIND)!=PIND))|((PD6p==0)&&(PD5p==1)&&((32|PIND)!=PIND)&&((64|PIND)!=PIND)))
    {
      if (speeder>32696)
      {
        speeder=32696;
      }
      if (speeder>804)
      {
        speeder -= 268;
      }
    }
    if((PIND|32)==PIND) {PD5p=1;} else {PD5p=0;}//Запомнить  состояние энкодера
    if((PIND|64)==PIND) {PD6p=1;} else {PD6p=0;}
    
    // Формирование отображаемого числа:
    if (speeder >=32697)
    {
      Sot=0; Des=0; Ed=Decoding[0]; stop=1;
    }// Перекинуть ноль с перед еденицы за 120. 
    else
    {
      speed=(speeder-536)/268;
      b=speed/100;	// Разложение  числа на составные (сотни, десятки, еденицы,)
      c=(speed-(b*100))/10;
      e=(speed-(b*100))-(c*10);
      Ed=Decoding[e];
      
      if (b==0)//Группа проверок для гашения нулей перед числом
      {
        Sot=0;
      } 
      else
      {
        Sot=Decoding[b];
      }
      
      if ((b==0)&&(c==0))
      {
        Des=0;
      } 
      else
      {
        Des=Decoding[c];
      }
      
      b=c=e=0;	
    }

  //Обработчик кнопок:
    if ((((PIND|2)==PIND)|((PIND|1)==PIND))&&(stop==0)&&((PD0p==0)|(PD1p==0))&&((Kn1==1)|(Kn2==1))) {stop=1; }	// Обработка сигнала стоп - по нажатию любой кнопки, если двигатель крутится
    if (((PIND|1)==PIND)&&(stop==1)&&(PD0p==0))	{PORTB|=4; stop= 0; stopp=0; Kn1=1; _delay_ms(16); }// Крутим, предположительно, в лево 
    if (((PIND|2)==PIND)&&(stop==1)&&(PD1p==0)) {PORTB&=~4; stop= 0; stopp=0; Kn2=1; _delay_ms(16); }// Крутим, предположительно, в право 
    if ((PIND|1)==PIND) {PD0p=1;} else {PD0p=0;}//Запомнить  состояние кнопок, крутим по условию отпускания кнопки
    if ((PIND|2)==PIND) {PD1p=1;} else {PD1p=0;}
    
    // PORTB= 0b01010100;
    //PORTB&=~0b01000100; Маска для сброса битов
    //PORTB|= 0b01000100; Маска для установки битов
    
    TWCR=StartTWI;// инициируем начало передачи данных 
  }
}