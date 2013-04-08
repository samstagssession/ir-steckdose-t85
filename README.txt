Build howto
-----------
- sudo apt-get install avr-libc binutils-avr gcc-avr avrdude
- make
- make upload
- disconnect programmer (slave rst, miso, mosi, sck)

$ make
avr-gcc  -mmcu=attiny85 -Wall -gdwarf-2 -std=gnu99 -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -MD -MP -MT main.o -MF dep/main.o.d  -c  main.c
avr-gcc  -mmcu=attiny85 -Wall -gdwarf-2 -std=gnu99 -DF_CPU=8000000UL -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -MD -MP -MT irmp.o -MF dep/irmp.o.d  -c  irmp.c
avr-gcc -mmcu=attiny85 -Wl,-Map=irmp.map main.o irmp.o     -o irmp.elf
avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature  irmp.elf irmp.hex
avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex irmp.elf irmp.eep || exit 0
avr-objdump -h -S irmp.elf > irmp.lss

AVR Memory Usage
----------------
Device: attiny85

Program:    3228 bytes (39.4% Full)
(.text + .data + .bootloader)

Data:         58 bytes (11.3% Full)
(.data + .bss + .noinit)

$ make upload 
avrdude -c arduino -p attiny85 -P /dev/ttyUSB0 -b 19200 -U flash:w:irmp.hex

avrdude: please define PAGEL and BS2 signals in the configuration file for part ATtiny85
avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.02s

avrdude: Device signature = 0x1e930b
avrdude: NOTE: FLASH memory has been specified, an erase cycle will be performed
         To disable this feature, specify the -D option.
avrdude: erasing chip
avrdude: please define PAGEL and BS2 signals in the configuration file for part ATtiny85
avrdude: reading input file "irmp.hex"
avrdude: input file irmp.hex auto detected as Intel Hex
avrdude: writing flash (3224 bytes):

Writing | ################################################## | 100% 5.48s

avrdude: 3224 bytes of flash written
avrdude: verifying flash memory against irmp.hex:
avrdude: load data flash data from input file irmp.hex:
avrdude: input file irmp.hex auto detected as Intel Hex
avrdude: input file irmp.hex contains 3224 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 3.58s

avrdude: verifying ...
avrdude: 3224 bytes of flash verified

avrdude: safemode: Fuses OK

avrdude done.  Thank you.



IR-Steckdose v0.5 ( by samstagssession@gmail.com )
-------------------------------------------------------
- Problem / motivation
-- My new lcd-tv got terrible build-in speakers so i connected it to my hifi amplifier
-- This old amp hasn't a remote and was mostly forgotten to switched off consuming a lot quiescent current all night long

- Code / hardware
-- This is an modified IRMP-Code  ( many thx!!! to IRMP project for doing the main ir decoding http://www.mikrocontroller.net/articles/IRMP )
-- We have implemented an atmel attiny85 into a "Conrad TV-Zwischenstecker KGQ01-01".
-- Original firmware was rediculous switching on every arbitrary remote around and switching off when load current falling below (not adjustable) threshold after a few minutes
-- The original build-in pic controller (most likely a pic18F84A) was disabled (tri stated) by pulling reset to gnd permanently so we can leave controller in place
-- The attiny85 was then piggybacked onto the old pic
-- All other circuitry inside was reused (excepting load current sense)

- Usage
-- Toggling load on/off by pressing the button
-- Hold button a about 5 seconds for teach-in mode (indicated by slow blinking led)
-- Then the received code is stored into attiny85 eeprom
-- All received codes from the used remote switches on
-- Only the above teached in ir code stored in eeprom switches off (so tv and amp can be switched off synchronously)

- Demo video
-- http://www.youtube.com/watch?v=SRs98dIe2WE&feature=player_detailpage



IRMP - Infrared Multi Protocol Decoder
--------------------------------------

Version IRMP:  2.2.2 25.05.2012
Version IRSND: 2.2.2 05.06.2012

Dokumentation:
 
   http://www.mikrocontroller.net/articles/IRMP
