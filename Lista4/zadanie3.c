#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <inttypes.h>

typedef struct RgbColor{
    unsigned char r;
    unsigned char g;
    unsigned char b;
}RgbColor;

typedef struct HsvColor{
    unsigned char h;
    unsigned char s;
    unsigned char v;
} HsvColor;

static inline unsigned char one_third(unsigned char value){
    return (value >> 2) + (value >> 4) + (value >> 6) + (value >> 7);
} 

RgbColor HsvToRgb(HsvColor hsv)
{
    RgbColor rgb;
    unsigned char region, remainder, p, q, t;
    
    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }
    
    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6; 
    
    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;
    
    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }
    
    return rgb;
}

HsvColor RgbToHsv(RgbColor rgb)
{
    HsvColor hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);
    
    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * (rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}

static inline uint8_t square_f(int16_t value, int16_t MAX_CAP){
    int16_t exponent = (value >> 3);
    exponent *= exponent;
    // exponent /= 20;
    return (uint8_t)(exponent > MAX_CAP ? MAX_CAP : exponent);
}

void timer1_init()
{
  TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM10);
  TCCR1B = _BV(WGM12) | _BV(CS10);
  DDRB |= _BV(PB1);
}

void timer2_init(){
  TCCR1A |= _BV(COM1B1) | _BV(COM1B0);
  DDRB |= _BV(PB2);
}

void timer3_init(){
    // PRR &= ~_BV(PRTIM2);
    TCCR2A = _BV(COM2A1) | _BV(COM2A0) 
    | _BV(WGM21) | _BV(WGM20);
    TCCR2B = _BV(CS20);
    DDRB |= _BV(PB3);
}

static inline void breath(RgbColor rgb){
        int16_t r = (int16_t) rgb.r;
        int16_t g = (int16_t) rgb.g;
        int16_t b = (int16_t) rgb.b;
        for(int16_t i = 0; i <= 70; i++){
            OCR1A = square_f(2*i,g);
            OCR1B = square_f(2*i,r);
            OCR2A = square_f(2*i,b);
            _delay_ms(19);
        }
        for(int16_t i = 70; i > 0; i--){
            OCR1A = square_f(2*i,g);
            OCR1B = square_f(2*i,r);
            OCR2A = square_f(2*i,b);
            _delay_ms(19);
        }
}

#define BAUD 9600
#define UBRR_VALUE ((F_CPU)/16/(BAUD)-1) 

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

FILE uart_file;


int main(){
    fdev_setup_stream(&uart_file,uart_transmit,uart_receive,_FDEV_SETUP_RW);
    stdin = stdout = stderr = &uart_file;
    timer1_init();
    timer2_init();
    timer3_init();
    uart_init();
    HsvColor hsv;
    hsv.s = 255;
    hsv.v = 255;
    hsv.h = 255;
    while(1){
        printf("%"PRIu8"\r\n",hsv.h);
        breath(HsvToRgb(hsv));
        hsv.h = rand();
        //hsv.v = rand();        
    }
}