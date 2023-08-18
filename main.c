#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

/*
ISR_ALIAS(TWI_vect, exit);

ISR(TWI_vect)
{
 asm volatile("nop");
 asm volatile("nop");
 asm volatile("nop");
 asm volatile("nop");
}
*/


void exit(int status){while(1);}


char usart_read(){
  while(!(UCSR0A & (1<<RXC0)));
  return UDR0;
}

void usart_write(unsigned char c){
  UDR0 = c;
  while(!(UCSR0A & (1<<UDRE0)));
}

int main(){

  //setup LED output
  DDRB |= (1<<5);
  PORTB &= ~(1<<5);

  //setup usart
  // UBRRn = (fosc / (16 * baud)) - 1
  //16MHz clock; 9600 baudrate
  //>>> (16000000.0 / (16*9600) ) - 1
  //103.16666666666667
  UBRR0H = 0;
  UBRR0L = 103;

  //enable transmitter and receiver
  UCSR0B |= (1<<RXEN0) | (1<<TXEN0);

  //disable all usart interrupts
  UCSR0B &= ~((1<<RXCIE0)|(1<<TXCIE0)|(1<<UDRIE0));

  //normal transmissio speed
  UCSR0A &= ~(1<<U2X0);

  //disable parity
  UCSR0C &= ~((1<<UPM01)|(1<<UPM00));

  //set 1 stop bit
  UCSR0C &= ~(1<<USBS0);

  //set 8 bit character size
  UCSR0C |= (1<<UCSZ00) | (1<<UCSZ01);

  //set pin tx (PD1) as output
  DDRD |= (1<<1);

  //set pin A0(PC0) input
  DDRC &= ~(1<<0);

  uint8_t t = 0; // pin value
  uint8_t c = 0; // previous pin value
  #define TIMS_SIZE 64
  uint8_t tims[TIMS_SIZE];
  int8_t k = 0; // tims buffer pointer
  int8_t b = 0; // bit counter
  int8_t print = 0; // flush 
  uint16_t tim = 0; // timer counter (TODO: use hardware timer)

  PORTB &= ~(1<<5);


  for(;;){

    // detect signal edge
    t = (PINC&1);
    if(t != c){
      c = t;

      // record pulse duration and reset timer
      int8_t x = tim>>8;
      tim = 0;

      // quantize time duration
      if(x < 4) x = 0;
      else if(x < 8) x = 1;
      else x = 2;

      // when signal goes lo means time invertal represets hi pulse period
      // push bit based on high pulse duration
      if(c==0 && x<2){
        tims[k] |= (x<<(7-b));
        b += 1;
      }

      // switch to next byte when all 8bits filled
      if(b>=8){
        b=0;
        k++;
        tims[k] = 0;
        if(k >= TIMS_SIZE) print = 1;
      }


      // pulse too long - reset buffer
      if(x==2){
        k = 0;
        b = 0;
        print = 0;
        tims[k] = 0;
      }

    }

    // timeout when no bits detected
    tim++;
    if(tim > 10240){
      tim = 0;
      print = 1;
    }


    if(print == 1){
      PORTB |= (1<<5);//DEBUG

      // number of bytes to send
      const unsigned char n = k+(b>0);

      if(n>0){
        usart_write(k*8+b); // send number of bits

        // send bytes and shift out unused bits
        const unsigned char B = 8-b;
        usart_write((tims[0]>>B) );
        for(unsigned char i=1; i<n; i++){
          usart_write((tims[i-1]<<b)  |  (tims[i]>>B) );
        }
      }

      k = 0;
      b = 0;
      print = 0;
      tims[k] = 0;

      PORTB &= ~(1<<5);//DEBUG
    }


  }

}

