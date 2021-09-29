//OpenSwitcher firmware v2.1.2e
//By M. Bostock
//29/09/21

#include <Adafruit_DotStar.h>
#define cia_sel1 4
#define cia_sel0 3
#define switch_request 2
#define relaypin2 1
#define relaypin1 0

#define START_MODE 0 //Boot up in swapped state? 0 = No, 1 = Yes
#define USER_TIMER_SETUP 3 //How long to hold CTRL A A before something happens
#define SLEEP_TIMEOUT 8 //Overall sleep timeout. Unused on NO_EEPROM version.
#define LED_BRIGHT 8 //All LED apart from red warning... LED brightness. 0 - 255 (0x00 - 0xFF)

Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
unsigned long ticktimer1;//global?
unsigned long ticktimer2;//global?
static bool swapactive; //global
static bool latchingrelay; //global
static bool relaychecked; //global?
static int usertimer; //global?

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

bool set_relay(const bool swap, const bool relaytype)
{
  if (relaytype)
  {
    if (swap)
    {
      digitalWrite(relaypin1, HIGH); delayMicroseconds (500000); digitalWrite(relaypin1, LOW); return 1;
    }
    else
    {
      digitalWrite(relaypin2, HIGH); delayMicroseconds (500000); digitalWrite(relaypin2, LOW); return 0;
    }
  }
  else
  {
    if (swap)
    {
      digitalWrite(relaypin1, HIGH);
      return 1;
    }
    else
    {
      digitalWrite(relaypin1, LOW);
      return 0;
    }

  }
}

bool reset_relay(const bool relaytype)
{
  if (relaytype)
  {
      digitalWrite(relaypin2, HIGH); delayMicroseconds (500000); digitalWrite(relaypin2, LOW); return 0;
  }
  else

    {
      digitalWrite(relaypin1, LOW);
      return 0;
    }

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
      led_flash (LED_BRIGHT, LED_BRIGHT, LED_BRIGHT, 3, 50000, 316000);
      if (digitalRead(switch_request) == LOW) return 1;
    }

  }
  //if(res) ticktimer1 = millis();
  return 0;
}

bool relaylatching()
{
  bool res = false;
  pinMode(relaypin2, INPUT_PULLUP);
  if (digitalRead(relaypin2) == LOW)
  {
    res = true;
    //led_show(0, 8, 8);
    pinMode(relaypin2, OUTPUT);
  }
  return res;
}

void setup() {

  //lock system if not plugged in to Amiga. Link keyboard reset and 5v to bypass.

  first_boot_lock();

  //set other pins

  pinMode(switch_request, INPUT_PULLUP);
  pinMode(cia_sel1, INPUT_PULLUP);
  pinMode(relaypin1, OUTPUT);

  //On coldboot, start mode is grabbed from #define
  swapactive = START_MODE;
  //On coldboot, user timer is grabbed from #define. In NO_EEPROM version, this cannot change after flashing. Remove a second from value to account for microcontroller boot
  usertimer = (USER_TIMER_SETUP - 1);
  //We don't know what type of relay it is yet...
  relaychecked = false;
  //Blackout LEDs
  led_show(0, 0, 0);
}

void loop() {
  //Is relay checked? Do we have a value stored to latchingrelay? If not, relaychecked is false.
  if(!relaychecked)
  {
  //check type of relay. If latching, pin 1 becomes output. Otherwise it is a floating input
  latchingrelay = relaylatching();
  //Once checked, lockout routine - return 1 if checked.
  relaychecked = true;
  }
  //Reset relay into unswapped position
  reset_relay(latchingrelay);
  
  //Wait for sel0 to go low
  while (digitalRead(cia_sel0) == LOW);
  //Allow 20uS per bit. (Allow slightly more than needed). 32 bit x 20uS= 680uS.
  delayMicroseconds(690);

  //set relay depending on desired state and type of relay. Blue LED for swapped, green for unswapped.
  if (set_relay(swapactive, latchingrelay)) led_show(0, 0, LED_BRIGHT);
  else led_show(0, LED_BRIGHT, 0);
  //allow half a second LED on time
  delay(500);
  //before turning LED off...
  led_show(0, 0, 0);
  //wait for a 3 finger salute (CTRL A A from Amiga)
  while (digitalRead(switch_request) == LOW); 
  //CTRL A A pressed... Lets find out how long the user is pressing it for...
  //If held for [usertimer ( USER_TIMER_SETUP-1 )] return 1 and swap drive.
  //Otherwise to go start.
  if (check_for_reset() == 1)
  {
    swap_drives();
    
  }
}
