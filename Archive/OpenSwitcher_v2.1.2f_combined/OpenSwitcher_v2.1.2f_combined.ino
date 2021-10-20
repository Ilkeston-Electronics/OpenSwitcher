//OpenSwitcher firmware v2.1.2f
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
#define LED_BRIGHT 8 //All LED apart from red warning... LED brightness. 0 - 255 (0x00 - 0xFF)
#define USE_EEPROM //Enable EEPROM functions, saving swapped state, custom user drive swap time and relay type.

static bool latchingrelay;
static bool swapactive; //global
static bool relaychecked;
static int usertimer;

#ifdef USE_EEPROM
#include <FlashStorage.h>
FlashStorage(sys_prep, bool);
FlashStorage(swapdrives_active, bool);
FlashStorage(user_timer, int);
#endif

Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);

#ifdef USE_EEPROM
int read_eeprom_drive()
{
  int val = 0;
  val = swapdrives_active.read();
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
  if (read_eeprom_drive() == val) return 0;
  return 1;
}

int write_eeprom_usertimer(const int val)
{
  user_timer.write(val);
  if (read_eeprom_usertimer() == val) return 0;
  return 1;
}
#endif

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

bool swap_drives(bool currentdrive)
{
  if (!currentdrive) currentdrive = true; else currentdrive = false;
#ifdef USE_EEPROM
  write_eeprom_drive(currentdrive);
#endif
  return currentdrive;
}

bool internal_disabled(bool currentstatus)
{
  if (!currentstatus) currentstatus = true; else currentstatus = false;
  return currentstatus;
}

static bool relayreset;
static bool internaldisabled;
int set_relay(const bool swap, const bool relaytype)
{
  if (relaytype)
  {
    if (swap)
    {
      digitalWrite(relaypin1, HIGH);
      delayMicroseconds (250000);
      digitalWrite(relaypin1, LOW);
      return 2;
    }
    else
    {
      digitalWrite(relaypin2, HIGH);
      delayMicroseconds (250000);
      digitalWrite(relaypin2, LOW);
      internaldisabled = false;
      relayreset = false;
      return 1;
    }
  }
  else
  {
    if (swap)
    {
      digitalWrite(relaypin1, HIGH);
      return 2;
    }
    else
    {
      digitalWrite(relaypin1, LOW);
      internaldisabled = false;
      relayreset = false;
      return 1;
    }

  }
}

bool reset_relay(const bool relaytype)
{
  if (relaytype)
  {
    digitalWrite(relaypin2, HIGH); delayMicroseconds (250000); digitalWrite(relaypin2, LOW); return 0;
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
  //On coldboot, user timer is grabbed from #define. In NO_EEPROM version, this cannot change after flashing. Remove a second from value to account for microcontroller boot
#ifndef USE_EEPROM
  usertimer = (USER_TIMER_SETUP - 1);
#else
  usertimer = (read_eeprom_usertimer() - 1);
#endif
  unsigned long ticktimer1 = millis();

  while (digitalRead(switch_request) == HIGH)
  {
    unsigned long ticktimer2 = millis();

    if ((ticktimer2 - ticktimer1) == (usertimer * 1000) && (digitalRead(switch_request) == HIGH))
    {
      led_flash (LED_BRIGHT, LED_BRIGHT, LED_BRIGHT, 3, 50000, 316000);
      if (digitalRead(switch_request) == LOW) return 1;
    }
    #ifdef USE_EEPROM
    if ((ticktimer2 - ticktimer1) == ((usertimer + 2) * 1000) && (digitalRead(switch_request) == HIGH))
    {
      led_flash (LED_BRIGHT, LED_BRIGHT, 0, 3, 50000, 316000);
      if (digitalRead(switch_request) == LOW) return 2;
    }
    #endif
    if ((ticktimer2 - ticktimer1) == ((usertimer + 4) * 1000) && (digitalRead(switch_request) == HIGH))
    {
      led_flash (0, LED_BRIGHT, LED_BRIGHT, 3, 50000, 316000);
      if (digitalRead(switch_request) == LOW) return 3;
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




bool flipflop_reset()
{
  bool res = false;
  unsigned long ticktimer1 = micros();
  int count = 1;
  while (count <= 2)
  {
    while (digitalRead(cia_sel0) == HIGH);
    while (digitalRead(cia_sel0) == LOW)
    {
      ++count;
      while (digitalRead(cia_sel0) == LOW);
    }
    unsigned long ticktimer2 = micros();
    if ((ticktimer2 - ticktimer1) > 100) break;
  }
  if (count == 2) return true;
}

void drive_id_sent()
{
  int count = 0;
  while (count < 32)
  {
    while (digitalRead(cia_sel0) == HIGH){};
    while (digitalRead(cia_sel0) == LOW)
    {
      ++count;
      while (digitalRead(cia_sel0) == LOW){};
    }
  }
}
#ifdef USE_EEPROM
int set_user_timer()
{
  int count = 0;
  int res = 0;
  unsigned long ticktimer1 = millis();
  while (digitalRead(switch_request) == HIGH);
  while (true)
  {
    unsigned long ticktimer2 = millis();
    if (ticktimer2 > (ticktimer1 + 1000))
    {
      led_flash (LED_BRIGHT, LED_BRIGHT, 0, 1, 25000, 200000);
      ++count;
      ticktimer1 = millis();
    }
    if ((digitalRead(switch_request) == HIGH) && (count <= 15))
    {
      if (count <= 2) count = 2;
      led_flash (LED_BRIGHT, LED_BRIGHT, 0, count, 25000, 475000);

      write_eeprom_usertimer(count);
      res = 1;
    }
    if (res) break;
    if (count >= 10)
    {
      //default values
      sys_prep.write(0);
      led_flash (LED_BRIGHT, 0, 0, 10, 50000, 50000);
      while (digitalRead(switch_request) == LOW){};
      SCB->AIRCR = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | SCB_AIRCR_SYSRESETREQ_Msk);
    }

  }
  return res;
  //reset();
}
#endif

int new_flash_setup()
{

#ifdef USE_EEPROM
  if (sys_prep.read() == 1) return 0;
  else
  {
    led_show(0, 0, 0);
    write_eeprom_drive(START_MODE);
    write_eeprom_usertimer(USER_TIMER_SETUP);
    sys_prep.write(1);
    led_flash (LED_BRIGHT, LED_BRIGHT, LED_BRIGHT, 3, 50000, 117000);

  }
  
  return 1;
  #else
  return 0;
  #endif
}

void setup() {

  //lock system if not plugged in to Amiga. Link keyboard reset and 5v to bypass.
  #ifndef TEST_MODE
  first_boot_lock();
  #endif

  //set other pins

  pinMode(switch_request, INPUT_PULLUP);
  pinMode(cia_sel1, INPUT_PULLUP);
  pinMode(relaypin1, OUTPUT);
#ifndef USE_EEPROM
  //On coldboot, start mode is grabbed from #define
  swapactive = START_MODE;

  //We don't know what type of relay it is yet...
  
#else
  new_flash_setup();
  swapactive = read_eeprom_drive();
#endif
latchingrelay = relaylatching();
  relayreset = false;
  //internal drive as DF1 is enabled initially
  internaldisabled = false;
  //Blackout LEDs
  led_show(0, 0, 0);
}

void loop() {
  static int led_res = 0;
  //if relay already reset, skip
  if (!relayreset)
  {
    //do we need to reset relay? If drive unswapped, no. If swapped and internal disabled, no. If swapped and internal enabled, yes.
    if (swapactive && !internaldisabled) reset_relay(latchingrelay);
#ifdef USE_EEPROM
    //grab any new eeprom values while we are here
    swapactive = read_eeprom_drive();
    usertimer = (read_eeprom_usertimer() - 1); // remove a second to account for boot
    if (usertimer < 1) usertimer = 1;
#endif
    relayreset = true;
  }

  //Wait for sel0 to go low
  if (digitalRead(cia_sel0) == LOW)
  {
    //Pulled low twice apparently. Like MTR0 line. Resets flipflop. Count and wait for this to complete...
    if (flipflop_reset())
    {
      //if flipflop reset, continue
      if (relayreset)
      {
        //Count how many times SEL0 goes low
        drive_id_sent();
        //set relay depending on desired state and type of relay. Blue LED for swapped, green for unswapped.
        led_res = (set_relay(swapactive, latchingrelay));  
        if (internaldisabled) led_res = 3;
      }
    }
  }
  //set a timer to show result
  unsigned long ticktimer1 = millis();
  unsigned long ticktimer2 = millis();
  while ((ticktimer1 + 4000 ) > ticktimer2)
  {
    if (led_res == 1) led_show(0, LED_BRIGHT, 0);
    else
    if (led_res == 2) led_show(0, 0, LED_BRIGHT);
    else
    if (led_res == 3) led_show(0, LED_BRIGHT, LED_BRIGHT);
    //increment now unused result by one incase CTRL A A was pressed while LED was showing...
    if (digitalRead(switch_request) == HIGH) {led_res = 4; break;}
    ticktimer2 = millis();
  }
  led_show(0,0,0);
  //wait for a 3 finger salute (CTRL A A from Amiga)
  
  if (led_res == 4 || (digitalRead(switch_request) == LOW))
  {
    while (digitalRead(switch_request) == LOW);
    //CTRL A A pressed... Lets find out how long the user is pressing it for...
    //If held for [usertimer ( USER_TIMER_SETUP-1 )] return 1 and swap drive.
    //If held for [usertimer+4 ( USER_TIMER_SETUP-1 )] return 2 and disable internal drive by not allowing relay to reset.
    //Otherwise to go start.
    int res = check_for_reset();
    if (res == 0) relayreset = false;
    else if (res == 1)
    {
      swapactive = swap_drives(swapactive);
      //if internal is disabled, force enabled condition
      if (internaldisabled) internal_disabled(true);
      //allow relay to reset on next cycle
      relayreset = false;
    }
#ifdef USE_EEPROM
    else if (res == 2)
    {
      set_user_timer();
    relayreset = false;
    }
#endif
    else if (res == 3)
    {
      internaldisabled = internal_disabled(internaldisabled);
      //user requested disable internal drive. Drive must be swapped or need to be swapped too...
      if (internaldisabled) swap_drives(false);
      //allow relay to reset on next cycle
      relayreset = false;
    }
  }


}
