//Amiga OpenSwitcher firmware. (c)2021 Matthew Bostock.
// v2.1.2d

//Operating Guide//Upon power on, unconnected (ie plugged into PC), red light will be flashing. This means successfully flashed... Please plug into OpenSwitcher.
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

#define START_MODE 0
#define USER_TIMER_SETUP 3
#define SLEEP_TIMEOUT 8
FlashStorage(sys_prep, bool);
FlashStorage(swapdrives_active, bool);
FlashStorage(relay_latching, bool);
FlashStorage(user_timer, int);

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

int read_eeprom_usertimer()
{
  int val = 0;
  val = user_timer.read();
  //val = test_mode_swap;
  return val;
}

int write_eeprom_drive(const bool val)
{
  swapdrives_active.write(val);
  if(read_eeprom_drive() == val) return 0;
  return 1;
}

int write_eeprom_relay(const bool val)
{
  relay_latching.write(val);
  if(read_eeprom_relay() == val) return 0;
  return 1;
}

int write_eeprom_usertimer(const int val)
{
 user_timer.write(val);
 if(read_eeprom_usertimer() == val) return 0;
  return 1;
}

int newflash_initial_settings()
{


  if (sys_prep.read() == 1) return 0;
  else
  {
    led_show(0, 0, 0);
    pinMode(relaypin2, INPUT_PULLUP);
    if (digitalRead(relaypin2) != HIGH)
      write_eeprom_relay(1);
    else write_eeprom_relay(0);

    write_eeprom_drive(START_MODE);
    write_eeprom_usertimer(USER_TIMER_SETUP);
    sys_prep.write(1);
    
  }
  return 1;
}
void first_boot_lock()
{
  pinMode(cia_sel0, INPUT_PULLUP);
  if (digitalRead(cia_sel0) == HIGH) standby(1);
}

void reset()
{
  SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk);
}

void standby(const int option)
{
  if (option == 1)
  {
    while (true) led_flash(2, 0, 0, 1, 50000, 950000);
    //attachInterrupt(switch_request, reset, FALLING);
  }
  
  led_show(0, 0, 0);
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

int check_for_reset()
{
  static int res = 0;
  ticktimer1 = millis();
  while (digitalRead(switch_request) == HIGH)
  {
    ticktimer2 = millis();

    if (ticktimer2 >= (ticktimer1 + (usertimer * 1000)) && (digitalRead(switch_request) == HIGH))
    {
      if (!res)
      {
        res = 1;
        led_flash (64, 64, 64, 3, 50000,316000);
      }
      if (ticktimer2 < (ticktimer1 + ((usertimer + 2) * 1000)) && (digitalRead(switch_request) == LOW)) return res;
    }
    if (ticktimer2 >= (ticktimer1 + ((usertimer + 2) * 1000)) && (digitalRead(switch_request) == HIGH))
    {
      if (res == 1)
      {
        led_flash (64, 64, 0, 3, 50000,316000);
        res = 2;
        if (ticktimer2 < (ticktimer1 + ((usertimer + 4) * 1000)) && (digitalRead(switch_request) == LOW)) return res;
      }
    }
    if (ticktimer2 >= (ticktimer1 + (10) * 1000) && (digitalRead(switch_request) == HIGH))
    {
      if (res == 2)
      {
        while (digitalRead(switch_request) == HIGH);
        reset();
      }
    }
  }
  //if(res) ticktimer1 = millis();
  return 0;
}

int swap_drives(const int mode)
{
  if (!swapactive || mode == 0)
  {

    write_eeprom_drive(1);
    return 1;

  } 
  if (swapactive || mode == 1)
  {

    write_eeprom_drive(0);
    return 0;

  }
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

int set_user_timer()
{
  usertimer = 0;
  static int res = 0;
  ticktimer1 = millis();
  while (digitalRead(switch_request) == HIGH);
  while (true)
  {
    ticktimer2 = millis();
    if (ticktimer2 > (ticktimer1 + 1000))
    {
      led_flash (64, 64, 0, 1, 100000, 100000);
      ++usertimer;
      ticktimer1 = millis();
    }
    if ((digitalRead(switch_request) == HIGH) && (usertimer <= 10))
    {
      if (usertimer <= 1) usertimer = 1;


      write_eeprom_usertimer(usertimer);
      res = 1;
    }
    if (res) break;
    if (usertimer >= 10)
    {
      //default values
      sys_prep.write(0);
      led_flash (64, 0, 0, 10, 50000, 50000);
      attachInterrupt(switch_request, reset, RISING);
      standby(0);
    }

  }
  return res;
  //reset();
}

void setup() {
 
  //lock system if not plugged in to Amiga. Link keyboard reset and 5v to bypass. Set reset sense pin.

  first_boot_lock();

  //write initial values. Check type of relay. Done once.
  if (newflash_initial_settings())
  {
    delayMicroseconds(250000);
    led_flash(64, 64, 64, 3, 50000, 316000);
    delayMicroseconds(250000);
  }
  //grab eeprom values globally
  swapactive = read_eeprom_drive();
  relaylatching = read_eeprom_relay();
  usertimer = (read_eeprom_usertimer() - 1); // remove a second to account for boot
  if (usertimer < 1) usertimer = 1;
  led_show(0, 0, 0);
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

}

void loop() {
  
 if (digitalRead(switch_request) == HIGH)
  {
    int res = check_for_reset();
    if (res == 1)
    {
      swap_drives(2);
        reset();
    }
    else if (res == 2)
    {
      set_user_timer();
      reset();
    }
    else reset();
  }
  while (digitalRead(cia_sel0) == LOW);
  delayMicroseconds(690);
  if (set_relay()) led_show(0, 0, 64); else led_show(0, 64, 0);
  attachInterrupt(switch_request, reset, RISING);
  ticktimer1 = millis();
  while (ticktimer2 <= (ticktimer1 + (SLEEP_TIMEOUT * 1000)))
  {
    ticktimer2 = millis();
  }
  standby(0);
}
