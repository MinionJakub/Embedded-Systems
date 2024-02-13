#define __AVR_ATmega328P__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <util/delay.h>
#include <stdio.h>
#include "i2c.h"

#define BAUD 9600   //baudrate
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) //zgodnie ze wzorem
#define EASY_WAY 1

#if(!EASY_WAY)
#define Transmit_Buf 100
char tab1[Transmit_Buf];
volatile int8_t read1=-1,write1=0;

#define Recive_Buf 100
char tab2[Recive_Buf];
volatile int8_t read2=-1,write2=0;

// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
ISR(USART_UDRE_vect){
  if( (read1+1==Transmit_Buf&&write1==0) ||read1+1==write1){
    UCSR0B &= ~_BV(UDRIE0);
    return;
  }
  read1++;
  if(read1>=Transmit_Buf) read1-=Transmit_Buf;
  UDR0=tab1[read1];
}

int uart_transmit(char data, FILE *stream){
  while ((write1==read1)||(write1==Transmit_Buf&&read1<1));
  if (write1>=Transmit_Buf) write1-=Transmit_Buf;
  tab1[write1]=data;
  write1++;
  if ((read1==99&&write1==1)||read1+2 == write1) 
    UCSR0B |= _BV(UDRIE0);
  return 0;
}

// odczyt jednego znaku
ISR(USART_RX_vect){
  if(write2>=Recive_Buf) write2-=Recive_Buf;
  tab2[write2]=UDR0;
  write2++;
}

int uart_receive(FILE *stream){
  while (read2+1==write2||(read2==Recive_Buf-1&&write2==0));
  read2++;
  if(read2>=Recive_Buf) read2-=Recive_Buf;
  return tab2[read2];
}
#endif //!EASY_WAY

#if(EASY_WAY)
// inicjalizacja UART
void uart_init()
{
  // ustaw baudrate
  UBRR0 = UBRR_VALUE;
  // wyczyść rejestr UCSR0A
  UCSR0A = 0;
  // włącz odbiornik i nadajnik
  UCSR0B = _BV(RXEN0) | _BV(TXEN0);
  // ustaw format 8n1
  UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}

// transmisja jednego znaku
int uart_transmit(char data, FILE *stream)
{
  // czekaj aż transmiter gotowy
  while(!(UCSR0A & _BV(UDRE0)));
  UDR0 = data;
  return 0;
}

// odczyt jednego znaku
int uart_receive(FILE *stream)
{
  // czekaj aż znak dostępny
  while (!(UCSR0A & _BV(RXC0)));
  return UDR0;
}

#endif //EASY_WAY

FILE uart_file;

const char st1[] PROGMEM ="write\0";
const char st2[] PROGMEM ="read\0";

#define i2cCheck(code, msg)\
if((TWSR & 0xf8) != (code)){\
printf(msg " failed, status: %.2x\r\n",(TWSR & 0xf8));\
i2cReset();\
continue;\
}

const uint8_t eeprom_addr = 0xa0;
int main(){
  uart_init();
  fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
  stdin = stdout = stderr = &uart_file;
  #if(!EASY_WAY)
  sei();
  UCSR0B |= _BV(RXCIE0);
  #endif //!EASY_WAY
  i2cInit();
  uint16_t addr = 0;
  uint8_t data = 0;
  char inst[]={"\0\0\0\0\0\0\0\0\0"};
  while(1){
    scanf("%s",&inst);
    printf("%s: ",inst);
    if(!(strcmp_P(inst,&st1))){
      scanf("%"SCNx16"%"SCNx8,&addr,&data);
      printf("%.3x %"PRId8"\n\r",addr,data);
      i2cStart();
      i2cCheck(0x08,"I2C write start");
      i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
      i2cCheck(0x18,"I2C EEPROM write request");
      i2cSend(addr&0xff);
      i2cCheck(0x28,"I2C EEPROM set address");
      i2cSend(data);
      i2cCheck(0x28,"I2C EEPROM data transmitted");
      i2cStop();
      i2cCheck(0xf8,"I2C stop");
    }
    else if(!(strcmp_P(inst,&st2))){
      scanf("%"SCNx16,&addr);
      printf("%.3x\n\r",addr);
      i2cStart();
      i2cCheck(0x08,"I2C read start");
      i2cSend(eeprom_addr | ((addr & 0x100) >> 7));
      i2cCheck(0x18,"I2C EEPROM write request");
      i2cSend(addr&0xff);
      i2cCheck(0x28,"I2C EEPROM set address");
      i2cStart();
      i2cCheck(0x10,"I2C second start");
      i2cSend(eeprom_addr | 0x1 | ((addr & 0x100) >> 7));
      i2cCheck(0x40, "I2C EEPROM read request");
      data = i2cReadNoAck();
      i2cCheck(0x58,"I2C EEPROM read");
      i2cStop();
      printf("%.3x: %x\r\n",addr,data);
    }
  }
}