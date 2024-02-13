#include <avr/interrupt.h>
#include <util/delay.h>

#include "USI_TWI_Master.h"

#define MESSAGEBUF_SIZE 4

#define TWI_CMD_MASTER_WRITE 0x10

int main(void)
{
  unsigned char messageBuf[MESSAGEBUF_SIZE];
  unsigned char TWI_targetSlaveAddress, temp, myCounter = 0;

  USI_TWI_Master_Initialise();
  TWI_targetSlaveAddress = 0x7f;

  for (;;)
  {

    messageBuf[0] = TWI_targetSlaveAddress << 1; // The first byte must always consit of General Call code or the TWI slave address.
    messageBuf[1] = TWI_CMD_MASTER_WRITE;             // The first byte is used for commands.
    messageBuf[2] = myCounter; // The second byte is used for the data.
    temp = USI_TWI_Start_Transceiver_With_Data(messageBuf, 3);

    myCounter += 1;
    _delay_ms(1000);
  }
}
