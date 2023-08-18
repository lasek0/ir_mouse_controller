#Atmega328p
# 32Kb flash
# 1Kb eeprom
# 2K SRAM

all:

CC=avr-gcc
OD=avr-objdump
OC=avr-objcopy
LD=avr-ld
PROG=avrdude
CRT_PATH=/usr/lib/avr/lib/avr5

clean:
	rm -f main.bin main.out main.o a.out move

main:
	$(CC) -mmcu=atmega328p main.c -O3 -o main.out
	$(OC) -O binary -j.text main.out main.bin

flash:
	sudo $(PROG) -p ATMEGA328P -c stk500v1 -P /dev/ttyUSB0 -b 57600 -U flash:w:main.bin:r

dump:
	$(OD) -xd main.out
	hexdump -Cv main.bin
	#$(OD) -m avr5 -b binary -D main.bin # disassembly flash binary

test:
	sudo ./move /dev/ttyUSB0 /dev/input/event7

move:
	gcc move.c -lm -o move
	

