//v2.1.2g

//What relay is fitted to your board?
#define LATCHING_RELAY // If latching relay fitted, UNCOMMENT. If not, comment out.

//Some handy board defaults...
//Trinket
//#define TRINKET //Activates Trinket defaults

//Pi_Pico
#define PI_PICO //Activates Pi_Pico defaults

//Other_Board... Manual setup. You are on your own!
//#define OTHER_BOARD

#ifdef TRINKET
//#define USE_FAKE_EEPROM //VIrtual EEPROM actually burns into the chip very very slightly. You get about 10,000 writes at least before it starts forgetting things.

//#define TURN_OFF_TRINKET_DOTSTAR //USE_TRINKET_DOTSTAR should be commented out and LED_IS_GPIO uncommented if using a TRINKET with a DOTSTAR LED. Why anyone would use this, I don't know...
#define LED_IS_GPIO 13 //Generic GPIO with an LED bolted on. But we have DotStar...
#define USE_TRINKET_DOTSTAR
#define USE_BLINK_BASED_INFO_LED
//#define USE_COLOUR_BASED_INFO_LED // Will make things difficult on mono-LED boards
#define cia_sel1 4
#define cia_sel0 3
#define switch_request 2
#define relaypin2 1
#define relaypin1 0
#endif

#ifdef PI_PICO
//#define LED_IS_BUILTIN //Actually GP25. Does the same thing as LED_IS_GPIO 25... See below...
#define LED_IS_GPIO 5 //Where is your LED? This example, it is GP5. Internal LED is on GP25, but it has a macro above called BUILTIN. LED_IS_BUILTIN and LED_IS_GPIO 25 are exactly the same thing...
#define USE_BLINK_BASED_INFO_LED
//#define USE_COLOUR_BASED_INFO_LED // Will make things difficult on mono-LED boards as it relies on just colours to give a result. For people who have poor eyesight.
#define USE_REAL_EEPROM //Activate Pico EEPROM. Yes it is REAL and made to be written to
#define cia_sel1 4
#define cia_sel0 3
#define switch_request 2
#define relaypin2 1
#define relaypin1 0
#endif


////////////////////END OF BASIC OPTIONS


////////////////////SET UP ACTUAL OpenSwitcher Options below.



//CURRENTLY_NOT_IMPLEMENTED.IGNORE THIS LINE//#define RELAY_NOT_FITTED // Not implemented or Testing. Using just transistors/gates. All SEL0/1 input or output is just forwarded through MCU.

#define START_MODE 0 //Boot up in swapped state? 0 = No, 1 = Yes
#define INTERNAL_DF1_DISABLED 0 //0 = Internal DF1 enabled (when swapped of course). 1 = Disabled. Probably will never get changed from here. Done on the Amiga.
#define USER_TIMER_SETUP 3 //How long to hold CTRL A A before something happens
#define LED_TIMEOUT 4 //How long to show RGB LED for before turning off
#define RGB_LED_BRIGHT 128 //All LED apart from red warning... LED brightness. 0 - 255 (0x00 - 0xFF)
#define SIMPLE_DRIVE_ID // DF0 drive ID in a simple way, just waiting and assuming it has completed within 690 uS. Comment out to do the actual count. Broken code?

//only for single colour LEDs. Changes timing for the information and "mode change" "flashes"
#define FIRST_FLASH_ON_TIME 600000
#define FIRST_FLASH_OFF_TIME 100000 // Irrelevent really as timer runs with LED off...
#define OTHER_FLASHES_ON_TIME 50000
#define OTHER_FLASHES_OFF_TIME 250000


#ifdef LED_IS_GPIO
#define led_pin LED_IS_GPIO
#endif
#ifdef LED_IS_BUILTIN
#define led_pin LED_BUILTIN
#endif

#ifdef USE_TRINKET_DOTSTAR
#include <Adafruit_DotStar.h>
Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
#endif

#ifdef TURN_OFF_TRINKET_DOTSTAR
#include <Adafruit_DotStar.h>
Adafruit_DotStar strip = Adafruit_DotStar(1, INTERNAL_DS_DATA, INTERNAL_DS_CLK, DOTSTAR_BGR);
#endif

static volatile bool swapactive; //global
static int reset_timer;
static volatile bool internaldisabled;
static bool latching_relay;

#ifdef USE_FAKE_EEPROM
#include <FlashAsEEPROM.h>
#define USE_EEPROM
#endif
#ifdef USE_REAL_EEPROM
#include <EEPROM.h>
#define USE_EEPROM
#endif


#ifdef USE_EEPROM
int read_eeprom_drive()
{
  int val = 0;
  val = (EEPROM.read(1));
  return val;
}

int read_eeprom_internal_drive()
{
  int val = 0;
  val = (EEPROM.read(2));
  return val;
}

int read_eeprom_usertimer()
{
  int val = 0;
  val = (EEPROM.read(3));
  return val;
}

int write_eeprom_drive(const bool val)
{
  EEPROM.write(1, val);
  if (read_eeprom_drive() == val) return 0;
  return 1;
}

int write_eeprom_internal_drive(const bool val)
{
  EEPROM.write(2, val);
  if (read_eeprom_internal_drive() == val) return 0;
  return 1;
}

int write_eeprom_usertimer(const int val)
{
  EEPROM.write(3, val);
  if (read_eeprom_usertimer() == val) return 0;
  return 1;
}
#endif

bool toggle_swap_drives(bool currentdrive)
{
  if (!currentdrive) currentdrive = true; else
  {
    currentdrive = false;
  }
  return currentdrive;
}

bool toggle_internal_disabled(bool currentstatus)
{
  if (!currentstatus) currentstatus = true; else currentstatus = false;
  return currentstatus;
}

void set_relay(const bool swap)
{
  if (latching_relay)
  {
    if (swap)
    {
      digitalWrite(relaypin1, HIGH);
      delayMicroseconds (250000);
      digitalWrite(relaypin1, LOW);
    }
    else
    {
      digitalWrite(relaypin2, HIGH);
      delayMicroseconds (250000);
      digitalWrite(relaypin2, LOW);
    }
  }
  else
  {
    if (swap)
    {
      digitalWrite(relaypin1, HIGH);
    }
    else
    {
      digitalWrite(relaypin1, LOW);
    }

  }
}

void led_show(const int red, const int green, const int blue)
{

#ifdef USE_TRINKET_DOTSTAR
  strip.setPixelColor(0, red, green, blue);
  strip.show();
#endif
  if (red == 0 && green == 0 && blue == 0) digitalWrite(led_pin, LOW);
  else digitalWrite(led_pin, HIGH);

}

void led_flash(const int red, const int green, const int blue, const int count, uint32_t pause1, uint32_t pause2)
{

  if (count == 0) return;
  if (pause2 == 0) pause2 = pause1;
  for (int i = 0; i < count; ++i)
  {
#ifdef USE_TRINKET_DOTSTAR
    strip.setPixelColor(0, red, green, blue);
    strip.show();
#endif
    digitalWrite(led_pin, HIGH);
    delayMicroseconds(pause1);
#ifdef USE_TRINKET_DOTSTAR
    strip.setPixelColor(0, 0, 0, 0);
    strip.show();
#endif
    digitalWrite(led_pin, LOW);
    delayMicroseconds(pause2);
  }
}

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
      led_flash (RGB_LED_BRIGHT, RGB_LED_BRIGHT, 0, 1, 25000, 200000);
      ++count;
      ticktimer1 = millis();
    }
    if ((digitalRead(switch_request) == HIGH) && (count <= 15))
    {
      if (count <= 1) count = 1;
      //led_flash (RGB_LED_BRIGHT, RGB_LED_BRIGHT, 0, 3, 25000, 158000);
      res = 1;
    }
    if (res) break;
    if (count >= 10)
    {
      //default values
#ifdef USE_EEPROM
      EEPROM.write(0, 0);
#endif
      led_flash (RGB_LED_BRIGHT, 0, 0, 10, 50000, 50000);
      while (true) led_flash (RGB_LED_BRIGHT, 0, 0, 1, 500000, 500000);
    }

  }
  return count;
  //reset();
}

int check_for_reset_rgb()
{
  //On coldboot, user timer is grabbed from #define. In NO_EEPROM version, this cannot change after flashing. Remove a second from value to account for microcontroller boot
  uint16_t timer_correction;
  if (reset_timer > 1) timer_correction = 500; else timer_correction = 250; //correct timer to account for bootloader delay of 0.5s. If untra-fast timer is required(1 sec), allow 250 mS free so user can actually reset without swapping modes.
  unsigned long ticktimer1 = millis();

  while (digitalRead(switch_request) == HIGH)
  {
    unsigned long ticktimer2 = millis();

    if ((ticktimer2 - ticktimer1) == ((reset_timer * 1000) - timer_correction) && (digitalRead(switch_request) == HIGH))
    {
      led_flash (RGB_LED_BRIGHT, RGB_LED_BRIGHT, RGB_LED_BRIGHT, 3, 25000, 158000);
      if (digitalRead(switch_request) == LOW) return 1;
    }

    if ((ticktimer2 - ticktimer1) == (((reset_timer + 2) * 1000) - timer_correction)  && (digitalRead(switch_request) == HIGH))
    {
      led_flash (0, RGB_LED_BRIGHT, RGB_LED_BRIGHT, 3, 25000, 158000);
      if (digitalRead(switch_request) == LOW) return 2;
    }
#ifdef USE_EEPROM
    if ((ticktimer2 - ticktimer1) == (((reset_timer + 4) * 1000) - timer_correction) && (digitalRead(switch_request) == HIGH))
    {
      led_flash (RGB_LED_BRIGHT, RGB_LED_BRIGHT, 0, 3, 25000, 158000);
      if (digitalRead(switch_request) == LOW) return 3;
    }
#endif
  }
  //if(res) ticktimer1 = millis();
  return 0;
}

int check_for_reset_mono_blink()
{

  //On coldboot, user timer is grabbed from #define. In NO_EEPROM version, this cannot change after flashing. Remove a second from value to account for microcontroller boot
  uint16_t timer_correction;
  int cycle_timer = reset_timer;
  if (reset_timer > 1) timer_correction = 0; else timer_correction = 250; //correct timer to account for bootloader delay of 0.5s. If untra-fast timer is required(1 sec), allow 250 mS free so user can actually reset without swapping modes.
  unsigned long ticktimer1 = millis();
  int res = 0;
  int count = 1;
  while (digitalRead(switch_request) == HIGH)
  {
    unsigned long ticktimer2 = millis();

    if ((ticktimer2 - ticktimer1) == ((cycle_timer * 1000) - timer_correction) && (digitalRead(switch_request) == HIGH))
    {
      if (digitalRead(switch_request) == LOW) return count;
      if (count == 1) led_flash (0, RGB_LED_BRIGHT, 0, count, FIRST_FLASH_ON_TIME, FIRST_FLASH_OFF_TIME);
      if (count == 2) led_flash (0, 0, RGB_LED_BRIGHT, count, OTHER_FLASHES_ON_TIME, OTHER_FLASHES_OFF_TIME);
      if (count == 3) led_flash(0, RGB_LED_BRIGHT, RGB_LED_BRIGHT, count, OTHER_FLASHES_ON_TIME, OTHER_FLASHES_OFF_TIME);
      if (count == 4) led_flash(RGB_LED_BRIGHT, RGB_LED_BRIGHT, 0, count, OTHER_FLASHES_ON_TIME, OTHER_FLASHES_OFF_TIME);
      if (digitalRead(switch_request) == LOW) return count;
      ++count;
#ifdef USE_EEPROM
      if (count > 4) count = 1;
#endif
#ifndef USE_EEPROM
      if (count > 3) count = 1;
#endif
      cycle_timer = 1;
      //delayMicroseconds(500000);
      ticktimer1 = millis();
    }
  }
  return 0;
}

void program_settings()
{



#ifdef USE_REAL_EEPROM
  EEPROM.begin(4);
#endif

#ifdef USE_EEPROM
  if (EEPROM.read(0) == 1)
  {
    led_show(0, 0, 0);
    reset_timer = read_eeprom_usertimer();
    internaldisabled = read_eeprom_internal_drive();
    if (!internaldisabled) swapactive = read_eeprom_drive();
    else swapactive = true;
  }
  else
  {
    led_show(0, 0, 0);
    write_eeprom_drive(START_MODE);
    write_eeprom_internal_drive(INTERNAL_DF1_DISABLED);
    write_eeprom_usertimer(USER_TIMER_SETUP);
    reset_timer = read_eeprom_usertimer();
    internaldisabled = read_eeprom_internal_drive();
    if (!internaldisabled) swapactive = read_eeprom_drive();
    else swapactive = true;
    EEPROM.write(0, 1);
    EEPROM.commit();
    led_flash (RGB_LED_BRIGHT, 0, RGB_LED_BRIGHT, 5, 50000, 40000);
  }

#else

  reset_timer = USER_TIMER_SETUP;
  internaldisabled = INTERNAL_DF1_DISABLED;
  if (internaldisabled) swapactive = true;
  else swapactive = START_MODE;
#endif
}


//set up pins
//check relay type

//is eeprom used?
//  Yes - Get values. If first flash, program with defaults.
//  No - use #defined values

void setup() {
  // put your setup code here, to run once:
  pinMode(switch_request, INPUT_PULLUP);
  pinMode(cia_sel1, INPUT_PULLUP);
  pinMode(relaypin1, OUTPUT);

  pinMode(cia_sel0, INPUT_PULLUP);

#ifdef TURN_OFF_TRINKET_DOTSTAR
  strip.setPixelColor(0, 0, 0, 0);
  strip.show();
#endif
  pinMode(led_pin, OUTPUT);
  if (digitalRead(cia_sel0) == HIGH)
  {
    while (digitalRead(cia_sel0) == HIGH) led_flash(2, 0, 0, 1, 50000, 950000);
  }
#ifdef LATCHING_RELAY
  latching_relay = true;
  pinMode(relaypin2, OUTPUT);
#else
  latching_relay = false;
  pinMode(relaypin2, INPUT_PULLUP);
#endif

  program_settings();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (internaldisabled == true) set_relay(true);
  else
  {
    set_relay(false);
#ifdef SIMPLE_DRIVE_ID
    while (digitalRead(cia_sel0) == LOW);
    delayMicroseconds(690);
#endif
    set_relay(swapactive);
  }

  //set a timer to show result

  volatile int res = 0;
if (!swapactive && !internaldisabled) res = 1;
  else if (swapactive && !internaldisabled) res = 2;
  else if (internaldisabled) res = 3;
  if (res)
  {
    if (res == 1) led_flash (0, RGB_LED_BRIGHT, 0, res, FIRST_FLASH_ON_TIME, FIRST_FLASH_OFF_TIME);
    if (res == 2) led_flash (0,0, RGB_LED_BRIGHT, res, OTHER_FLASHES_ON_TIME, OTHER_FLASHES_OFF_TIME);
    if (res == 3) led_flash (0, RGB_LED_BRIGHT, RGB_LED_BRIGHT, res, OTHER_FLASHES_ON_TIME, OTHER_FLASHES_OFF_TIME);
  }

  led_show(0, 0, 0);

  //wait for a 3 finger salute (CTRL A A from Amiga)
  volatile int reset_res = 0;
  int new_timer_setting = 0;

  //CTRL A A pressed... Lets find out how long the user is pressing it for...
  //If held for [usertimer ( USER_TIMER_SETUP-1 )] return 1 and swap drive.
  //If held for [usertimer+4 ( USER_TIMER_SETUP-1 )] return 2 and disable internal drive by not allowing relay to reset.
  //Otherwise to go start.
#ifdef USE_COLOUR_BASED_INFO_LED
  if (res == 4 || (digitalRead(switch_request) == LOW))
  {
    while (digitalRead(switch_request) == LOW);



    reset_res = check_for_reset_rgb();
    if (reset_res == 1)
    {
      internaldisabled = false;
      swapactive = toggle_swap_drives(swapactive);
    }
    else if (reset_res == 2)
    {
      internaldisabled = toggle_internal_disabled(internaldisabled);
      if (internaldisabled) swapactive = true;
      else swapactive = false;
    }
    else if (reset_res == 3)
    {
      led_flash (0, RGB_LED_BRIGHT, RGB_LED_BRIGHT, 5, 50000, 40000);
      delayMicroseconds(1000000);
      new_timer_setting = set_user_timer();
    }
  }
#endif
#ifdef USE_BLINK_BASED_INFO_LED
  if (res == 4 || (digitalRead(switch_request) == LOW))
  {
    while (digitalRead(switch_request) == LOW);
    reset_res = check_for_reset_mono_blink();

    if (reset_res == 1)
    {
      swapactive = false;
      internaldisabled = false;
    }
    else if (reset_res == 2)
    {
      swapactive = true;
      internaldisabled = false;
    }
    else if (reset_res == 3)
    {
      internaldisabled = toggle_internal_disabled(internaldisabled);
      if (internaldisabled) swapactive = true;
      else swapactive = false;
    }
    else if (reset_res == 4)
    {
      led_flash (0, RGB_LED_BRIGHT, RGB_LED_BRIGHT, 5, 50000, 40000);
      delayMicroseconds(1000000);
      new_timer_setting = set_user_timer();
    }
  }
#endif

  if (reset_res)
  {
    int eeprom_res = 0;
#ifdef USE_EEPROM
    if (read_eeprom_drive() != swapactive) {
      write_eeprom_drive(swapactive);
      eeprom_res = 1;
    }
    if (read_eeprom_internal_drive() != internaldisabled) {
      write_eeprom_internal_drive(internaldisabled);
      eeprom_res = 2;
    }
    if (new_timer_setting)
    {
      if (read_eeprom_usertimer() != new_timer_setting) {
        reset_timer = new_timer_setting;
        write_eeprom_usertimer(new_timer_setting);
        new_timer_setting = 0;
        eeprom_res = 3;
      }
    }
    EEPROM.commit();
#endif
    led_flash (RGB_LED_BRIGHT, 0, RGB_LED_BRIGHT, 5, 50000, 40000);
    eeprom_res = 0;
    reset_res = 0;


  }
}
