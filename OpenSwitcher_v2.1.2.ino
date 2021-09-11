//Amiga OpenSwitcher firmware. (c)2021 Matthew Bostock.
// v2.1.2

//Operating Guide

//>>>>>>START<<<<<<
//Upon power on, unconnected (ie plugged into PC), red light will be flashing. This means successfully flashed... Please plug into OpenSwitcher.
//------------------------------
//This will ONLY HAPPEN ONCE on FIRST INSTALL INTO OpenSwitcher! Upon Amiga power-on, white LED should flash 3 times. Identify relay, program virtual Eeprom with default values. When white LED flash 3 times This indicates setup is correct.
//------------------------------
//Upon Green LED, drive ID successful, booting in a stock unswapped condition
//Upon Blue LED, drive ID successful, booting in swapped condition.
//LED off after 8 seconds approx = OpenSwitcher is asleep.
//
//Upon Amiga RESET, pink LED, reboot successful, go back to start.
//If Amiga RESET is HELD for approx 3 seconds, White LED will blink 3 times. If in a swapped condition, will become unswapped. If unswapped, will be swapped. Stored to Eeprom.
//That is all

#include <Adafruit_DotStar.h>
#include <FlashStorage.h>
#define cia_sel1 4
#define cia_sel0 3
#define switch_request 2
#define relaypin2 1
#define relaypin1 0

#define RESET_SWAP_TIMEOUT 2000
#define START_MODE 1

FlashStorage(sys_prep, bool);
FlashStorage(swapdrives_active, bool);
FlashStorage(relay_latching, bool);

Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
unsigned long ticktimer1;
unsigned long ticktimer2;
static bool swapactive;
static bool relaylatching;


void led_show(const int red, const int green, const int blue)
{
  strip.setPixelColor(0, red, green, blue);
  strip.show();
}

void led_flash(const int red, const int green, const int blue, const int count, uint32_t pause1, uint32_t pause2)
{
  if (pause2 == 0) pause2 = pause1;
  for (int i = 0; i < count; ++i)
  { strip.setPixelColor(0, red, green, blue);
    strip.show();
    delayMicroseconds(pause1);
    strip.setPixelColor(0, 0, 0, 0);
    strip.show();
    delayMicroseconds(pause2);
  }

}

int read_eeprom_drive()
{
  int val = 0;
  val = swapdrives_active.read();
  //val = test_mode_swap;
  return val;
}

int read_eeprom_relay()
{
  int val = 0;
  val = relay_latching.read();
  //val = test_mode_swap;
  return val;
}


int write_eeprom(const int value)
{

  swapdrives_active.write(value);
}

int setup_system()
{
  if (sys_prep.read() == 1) return 0;
  else
  {
    led_show(0, 0, 0);
    pinMode(relaypin2, INPUT_PULLUP);
    if (digitalRead(relaypin2) == LOW) relay_latching.write(1);
    else relay_latching.write(0);

    swapdrives_active.write(START_MODE);
    sys_prep.write(1);
    delayMicroseconds(500000);
    led_flash(64, 64, 64, 3, 50000, 300000);


  }
  if (swapdrives_active.read() != START_MODE) return 1;
  return 0;
}

void override_system()
{
  digitalWrite(relaypin1, HIGH);
  while (true)
  {
    led_flash(8, 8, 8, 1, 500000, 0);
  }
}

void standby()
{
  //led_show(8,0,0);
  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

  GCLK->CLKCTRL.reg = uint16_t(
                        GCLK_CLKCTRL_CLKEN |
                        GCLK_CLKCTRL_GEN_GCLK2 |
                        GCLK_CLKCTRL_ID( GCLK_CLKCTRL_ID_EIC_Val )
                      );
  while (GCLK->STATUS.bit.SYNCBUSY) {}

  __DSB();
  __WFI();
}

void reset()
{
  SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk);
}

int startup_request_pause()
{
  static int halt_request = 0;
  pinMode(cia_sel0, INPUT_PULLUP);
  if (digitalRead(cia_sel0) == HIGH)
  {
    led_flash (64, 64, 0, 1, 1000000, 0);
    halt_request = 1;
  }

  return halt_request;
}

void setup()
{
  led_show(0, 0, 0);
  static int startup_pause = 0;
  startup_pause = startup_request_pause();
  if (startup_pause)
  {
    while (true) led_flash(64, 0, 0, 1, 50000, 950000);
  }
  pinMode(relaypin1, OUTPUT);

  if (read_eeprom_relay() == 1)
  {
    pinMode(relaypin2, OUTPUT);
    digitalWrite(relaypin2, LOW);
  }
  else
  {
    pinMode(relaypin2, INPUT_PULLUP);
  }
  digitalWrite(relaypin1, LOW);
  pinMode(switch_request, INPUT_PULLUP);
  pinMode(cia_sel0, INPUT_PULLUP);
  pinMode(cia_sel1, INPUT_PULLUP);
  setup_system();
}

void loop() {
  swapactive = read_eeprom_drive();
  relaylatching = read_eeprom_relay();

  if (digitalRead(switch_request) == HIGH)
  {
    ticktimer1 = millis();
    while (digitalRead(switch_request) == HIGH)
    {
      ticktimer2 = millis();
      if (ticktimer2 >= (ticktimer1 + RESET_SWAP_TIMEOUT))
      {
        led_show(0, 0, 0);
        if (swapactive)
        {

          //led_show(0, 64, 0);
          if (swapdrives_active.read() != 0) swapdrives_active.write(0);

        } else
        {
          //led_show(0, 0, 64);
          if (swapdrives_active.read() != 1) swapdrives_active.write(1);

        }
        led_flash (64, 64, 64, 3, 100000, 333333);

        reset();
      }
    }

  }

  relaylatching = read_eeprom_relay();

  while (digitalRead(cia_sel0) == LOW);
  delayMicroseconds(690);

  if (swapactive)
  {

    if (relaylatching == 1)
    {
      digitalWrite(relaypin1, HIGH); delayMicroseconds (500000); digitalWrite(relaypin1, LOW);
    }
    else
    {
      digitalWrite(relaypin1, HIGH);
    }
  }
  else
  {

    {
      if (relaylatching == 1)
      {
        digitalWrite(relaypin2, HIGH); delayMicroseconds (500000); digitalWrite(relaypin2, LOW);
      }
      else
      {
        digitalWrite(relaypin1, LOW);
      }
    }
  }
  attachInterrupt(switch_request, reset, RISING);
  delayMicroseconds(500000);
  if (swapactive) led_show(0, 0, 64);
  else led_show(0, 64, 0);

  while (ticktimer2 <= (ticktimer1 + 8000))
  {
    ticktimer2 = millis();
  }
  led_show(0, 0, 0);
  standby();
}
