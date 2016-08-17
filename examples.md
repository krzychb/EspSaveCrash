## Examples

The following example Arduino sketches (*.ino) are available with this library:

  1. [SimpleCrash](https://github.com/krzychb/EspSaveCrash/blob/master/examples/SimpleCrash/SimpleCrash.ino) - trigger sample exceptions and show them on a serial monitor.
  2. [ExtendedCrashTester](https://github.com/krzychb/EspSaveCrash/blob/master/examples/ExtendedCrashTester/ExtendedCrashTester.ino) - extended version of SimpleCrash above. It allows triggering additional sample exceptions.
  3. [RemoteCrashCheck](https://github.com/krzychb/EspSaveCrash/blob/master/examples/RemoteCrashCheck/RemoteCrashCheck.ino) - check the list and details of saved exceptions remotely with web browser.

To use examples first you need to [install](https://www.arduino.cc/en/Guide/Libraries#toc2) [EspSaveCrash](https://github.com/krzychb/EspSaveCrash) library on your PC.

## Table of Contents
- [Simple Crash](#simple-crash)
  - [Try It](#try-it)
  - [Set Up](#set-up)
  - [Print Out ](#print-out)
  - [Trigger Exceptions](#trigger-exceptions)
- [Extended Tester](#extended-tester)
- [Remote Crash Check](#remote-crash-check)
- [Library Configuration](#library-configuration)


## Simple Crash

This [example sketch](https://github.com/krzychb/EspSaveCrash/blob/master/examples/SimpleCrash/SimpleCrash.ino) demonstrates how to:
  * set up the sketch, so it will automatically capture exceptions and save it in flash memory of ESP module,
  * print out saved crash information on a serial monitor,
  * trigger sample exceptions to quickly show functionality of this library.


### Try It

Upload [SimpleCrash.ino](https://github.com/krzychb/EspSaveCrash/blob/master/examples/SimpleCrash/SimpleCrash.ino) to your ESP module and open serial monitor. The following message should be displayed:

```
SimpleCrash.ino
No any crashes saved

Press a key + <enter>
0 : attempt to divide by zero
e : attempt to read through a pointer to no object
c : clear crash information
```

Now press `0` + ENTER. This will execute code that attempts to divide by zero. As result module will drop an exception and restart. The following will be displayed.

```
Attempting to divide by zero ...

Exception (0):
epc1=0x40107001 epc2=0x00000000 epc3=0x00000000 excvaddr=0x00000000 depc=0x00000000

ctx: cont 
sp: 3ffefa50 end: 3ffefc20 offset: 01a0

>>>stack>>>
3ffefbf0:  00000000 00000000 3ffeebd0 40201f5c  
3ffefc00:  3fffdad0 00000000 3ffeebec 40202a24  
3ffefc10:  feefeffe feefeffe 3ffeec00 40100114  
<<<stack<<<

 ets Jan  8 2013,rst cause:2, boot mode:(3,6)

load 0x4010f000, len 1384, room 16 
tail 8
chksum 0x2d
csum 0x2d
v00000000
~ld

SimpleCrash.ino
Crash information recovered from EEPROM
Crash # 1 at 65772 ms
Reason of restart: 2
Exception cause: 0
epc1=0x40107001 epc2=0x00000000 epc3=0x00000000 excvaddr=0x00000000 depc=0x00000000
>>>stack>>>
3ffefbf0: 00000000 00000000 3ffeebd0 40201f5c 
3ffefc00: 3fffdad0 00000000 3ffeebec 40202a24 
3ffefc10: feefeffe feefeffe 3ffeec00 40100114 
<<<stack<<<
EEPROM space available: 0x01ab bytes

Press a key + <enter>
0 : attempt to divide by zero
e : attempt to read through a pointer to no object
c : clear crash information
```

To drop another exception press `e` + ENTER. Module will restart again. Now you will see two exceptions reported.

```
SimpleCrash.ino
Crash information recovered from EEPROM
Crash # 1 at 65772 ms
Reason of restart: 2
Exception cause: 0
epc1=0x40107001 epc2=0x00000000 epc3=0x00000000 excvaddr=0x00000000 depc=0x00000000
>>>stack>>>
3ffefbf0: 00000000 00000000 3ffeebd0 40201f5c 
3ffefc00: 3fffdad0 00000000 3ffeebec 40202a24 
3ffefc10: feefeffe feefeffe 3ffeec00 40100114 
<<<stack<<<
Crash # 2 at 66594 ms
Reason of restart: 2
Exception cause: 28
epc1=0x40201f87 epc2=0x00000000 epc3=0x00000000 excvaddr=0x00000000 depc=0x00000000
>>>stack>>>
3ffefbf0: 00000000 00000000 3ffeebd0 40201f85 
3ffefc00: 3fffdad0 00000000 3ffeebec 40202a24 
3ffefc10: feefeffe feefeffe 3ffeec00 40100114 
<<<stack<<<
EEPROM space available: 0x0159 bytes

Press a key + <enter>
0 : attempt to divide by zero
e : attempt to read through a pointer to no object
c : clear crash information
```

You may trigger more exceptions and should see all of them displayed in a list as above. Once reserved flash memory is full, at the end of the list, you will see a message as follows:

```
No more EEPROM space available to save crash information!
```

At this point no more new crash information will be written to the flash. To save new exceptions, you need to clear memory or reserve more memory space. Press `c` + <enter> to clear the memory. You will be now able to capture new exceptions. 

To increase memory space and save move information please refer to chapter [Library Configuration](library-configuration) below.


### Set Up

To make your sketch capturing exceptions, first include [EspSaveCrash](https://github.com/krzychb/EspSaveCrash) library:

```cpp
#include "EspSaveCrash.h"
```

That's it and that all. From now on, if you upload such sketch to the module, crash information will be saved to module's flash memory.


### Print Out

Printing out saved exception is done by entering the following single line in your code:

```cpp
SaveCrash.print();
```

**Note:** This line may be included anywhere in the sketch after initializing serial `Serial.begin`.


### Trigger Exceptions

If your sketch is not crashing instantly, it is a good idea to check the library in action in advance. To do so you may use the code snippets inside the `loop()`. The code is reading characters typed on the keyboard:

```cpp
void loop(void)
{
  if (Serial.available() > 0)
  {
    char inChar = Serial.read();
    
    // trigger exception
    (...)

  }
}
```

Depending on the key pressed exceptions are triggered. For example, after pressing `0` the following code is executed:

```cpp
  case '0':
    Serial.println("Attempting to divide by zero ...");
    int result, zero;
    zero = 0;
    result = 1 / zero;
    Serial.print("Result = ");
    Serial.println(result);
    break;
```

**Note:** You do not need to include this code in your sketch at all. It is provided only to simulate some exceptions and check how this library works.


## Extended Tester

[ExtendedCrashTester](https://github.com/krzychb/EspSaveCrash/blob/master/examples/ExtendedCrashTester/ExtendedCrashTester.ino) is an extended version of [Simple Crash](#simple-crash) above. It provides additional functionality:

  * triggers 'write through a pointer to no object' exception
  * activates software and hardware WDT - [Watchdog Timer](https://en.wikipedia.org/wiki/Watchdog_timer)
  * allows to reset and restart module from the sketch

It also provides more diagnostic information like printing out time in milliseconds when particular exception was triggered.

**Note:** The hardware WDT is the last resort watchdog triggered by the h/w timer. It is provided in case when the software WDT is not performing. Crash due to h/w WDT cannot be captured by software.


## Remote Crash Check

Your module may be in some remote location where it is inconvenient to use a serial monitor. In such case you can access crash information with a web browser.

This functionality is demonstrated with [RemoteCrashCheck](https://github.com/krzychb/EspSaveCrash/blob/master/examples/RemoteCrashCheck/RemoteCrashCheck.ino) example sketch. 

To make it run update SSID and password to your network and upload it to module. Open serial monitor to check provided IP. 

```
Connecting to sensor-net ........ connected
No any crashes saved
Web server started, open 192.168.1.116 in a web browser

? - help
```

You may then trigger some exceptions like described in examples discussed above.

Once module connects to network, after opening provided IP in a web browser, you will be able to see all captured crash information like in example below.

![alt text](extras/crash-info-in-web-browser.png "Sample crash information in a web browser")


## Library Configuration

Crash data are saved using the [EEPROM](https://github.com/esp8266/Arduino/blob/master/doc/libraries.md#eeprom) library that is provided together with [esp8266 / Arduino](https://github.com/esp8266/Arduino) core.

If you like to change flash memory space reserved for storing crash information use define below. The maximum value is 4096 (0x1000) bytes as defined for the [EEPROM](https://github.com/esp8266/Arduino/blob/master/doc/libraries.md#eeprom) library.

```cpp
#define SAVE_CRASH_SPACE_SIZE       0x0200  // space reserved to store crash data
```

You may have other applications already using EEPROM. With parameter `SAVE_CRASH_EEPROM_OFFSET` below you can allocate initial EEPROM space for the other applications. Optionally use for this purpose upper EEPROM space (i.e. above `SAVE_CRASH_EEPROM_OFFSET` + `SAVE_CRASH_SPACE_SIZE`).

```cpp
#define SAVE_CRASH_EEPROM_OFFSET    0x0010  // adjust it to reserve space to store other data in EEPROM
```

Picture below shows relation between both configuration parameters and total available EEPROM space.

![alt text](extras/eeprom-layout.png "EEPROM layout")

