#include <IRremote.h>

int bit0 = 8;
int bit1 = 9;
int bit2 = 10;
int c = 11;
int p = 12;

IRsend irsend;

void setup()
{
  pinMode(bit0, INPUT_PULLUP);
  pinMode(bit1, INPUT_PULLUP);
  pinMode(bit2, INPUT_PULLUP);

  pinMode(c, INPUT_PULLUP);
  pinMode(p, INPUT_PULLUP);
}

void loop()
{

  char sendCode = digitalRead(c);
  char positioning = digitalRead(p);

  char val0 = digitalRead(bit0);
  char val1 = digitalRead(bit1);
  char val2 = digitalRead(bit2);

  int code = val0 + val1 * 10 + val2 * 100;

  if (sendCode == 1)
  {
    if (positioning == 1)
    {
      irsend.sendNEC(0xFF, 8);
      delay(168);
    }
    else
    {
      switch (code)
      {
      case 0:
        irsend.sendNEC(0x00, 8);
        delay(186);
        break;

      case 1:
        irsend.sendNEC(0x01, 8);
        delay(184);
        break;

      case 10:
        irsend.sendNEC(0x02, 8);
        delay(184);
        break;

      case 11:
        irsend.sendNEC(0x03, 8);
        delay(182);
        break;

      case 100:
        irsend.sendNEC(0x04, 8);
        delay(184);
        break;

      case 101:
        irsend.sendNEC(0x05, 8);
        delay(182);
        break;

      case 110:
        irsend.sendNEC(0x06, 8);
        delay(182);
        break;

      case 111:
        irsend.sendNEC(0x07, 8);
        delay(180);
        break;
      }
    }
  }
}
