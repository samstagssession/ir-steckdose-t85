Build howto
-----------
- sudo apt-get install avr-libc binutils-avr gcc-avr avrdude
- make

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
