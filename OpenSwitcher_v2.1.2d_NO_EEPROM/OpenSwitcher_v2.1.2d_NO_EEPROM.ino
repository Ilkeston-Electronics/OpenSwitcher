#include <Adafruit_DotStar.h>
#define cia_sel1 4
#define cia_sel0 3
#define switch_request 2
#define relaypin2 1
#define relaypin1 0

#define START_MODE 0
#define USER_TIMER_SETUP 3
#define SLEEP_TIMEOUT 8

Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
unsigned long ticktimer1;
unsigned long ticktimer2;
static bool swapactive;
static bool relaylatching;
static int usertimer;

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

bool swap_drives()
{
  if (!swapactive) swapactive = true; else swapactive = false;
  return swapactive;
}

bool set_relay()
{
  static bool coil1_active = false;
  static bool coil2_active = false;
  if (swapactive)
    {

      if (relaylatching)
      {
        if (!coil1_active)
        {
          digitalWrite(relaypin1, HIGH); delayMicroseconds (500000); digitalWrite(relaypin1, LOW);
          coil1_active = true;
          coil2_active = false;
        }
      }
      else
      {
        digitalWrite(relaypin1, HIGH);
        coil1_active = true;
        coil2_active = false;
      }
      
    }
    else
    {
      if (relaylatching)
      {
        if (!coil2_active)
        {
          digitalWrite(relaypin2, HIGH); delayMicroseconds (500000); digitalWrite(relaypin2, LOW);
          coil2_active = true;
          coil1_active = false;
        }
      }
      else
      {
        digitalWrite(relaypin1, LOW);
        coil1_active = false;
        coil2_active = false;
      }
    }
    return coil1_active;
}

void first_boot_lock()
{
  pinMode(cia_sel0, INPUT_PULLUP);
  if (digitalRead(cia_sel0) == HIGH)
  {
    while (digitalRead(cia_sel0) == HIGH) led_flash(2, 0, 0, 1, 50000, 950000);
  }
}

int check_for_reset()
{
  ticktimer1 = millis();
  while (digitalRead(switch_request) == HIGH)
  {
    ticktimer2 = millis();

    if ((ticktimer2 - ticktimer1) == (usertimer * 1000) && (digitalRead(switch_request) == HIGH))
    {
        led_flash (64, 64, 64, 3, 50000,316000);
        if (digitalRead(switch_request) == LOW) return 1;
    }
    
  }
  //if(res) ticktimer1 = millis();
  return 0;
}

void setup() {
 
  //lock system if not plugged in to Amiga. Link keyboard reset and 5v to bypass. Set reset sense pin.

  first_boot_lock();
pinMode(relaypin2, INPUT_PULLUP);
    if (digitalRead(relaypin2) != HIGH) relaylatching = true;
    else relaylatching = false;
  //set other pins

  pinMode(switch_request, INPUT_PULLUP);
  pinMode(cia_sel1, INPUT_PULLUP);
  pinMode(relaypin1, OUTPUT);

  //Reset relay into default position if latching
  if (relaylatching)
  {
    pinMode(relaypin2, OUTPUT);
    digitalWrite(relaypin2, HIGH); delayMicroseconds (100000); digitalWrite(relaypin2, LOW);
  }
  else pinMode(relaypin2, INPUT_PULLUP);
  swapactive = START_MODE;
  usertimer = (USER_TIMER_SETUP - 1);
  led_show(0, 0, 0);
}

void loop() {
  //ticktimer1 = millis();
 
  
  if (relaylatching)
  {
    digitalWrite(relaypin2, HIGH); delayMicroseconds (100000); digitalWrite(relaypin2, LOW);
  }
  
  while (digitalRead(cia_sel0) == LOW);
  delayMicroseconds(690);
  set_relay();
  if (swapactive) led_show(0, 0, 8); 
  else led_show(0, 8, 0);
  
  while (digitalRead(switch_request) == LOW);
  led_show(0, 0, 0);
    if (check_for_reset() == 1) swap_drives();
  }
