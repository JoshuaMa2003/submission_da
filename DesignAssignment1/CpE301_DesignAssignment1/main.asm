;
; CpE301_DesignAssignment1.asm
;
; Created: 2/25/2024 8:04:52 PM
; Author : joshu
;


.include <m328pdef.inc>

//initialize stack pointer
LDI R16, HIGH(RAMEND) //load high byte of RAMEND into r16
OUT SPH, R16 //set stack pointer high byte
LDI R16, LOW(RAMEND) //load low byte of ramend into r16
OUT SPL, R16 //set stack pointer low byte

//main program intilization
CLR R17 //clear r17 to use as low byte of sum
CLR R18 //clear r18 to use as high byte of sum
LDI R19, 16 //load 16 into r19 to use as loop counter

//initialize z-pointer to the start of sequence in program memory
LDI ZH, HIGH(0x1EEF) //set z pointer high part of sequence start address
LDI ZL, LOW(0x1EEF) //set z pointer low part of sequence start address

MAIN_LOOP:
CPI R19, 0 //check if counter is zero
BREQ SRAM_STORE //if counter 0, branch to storage section

//load next number from program memory and add it to sum
LPM R16, Z+ //load byte at Z into r16 and increment z
ADD R17, R16 //add r16 to the sum's lower byte
ADC R18, R1 //add carry to the sum's higher byte

DEC R19 //decrement loop counter
RJMP MAIN_LOOP //repeat loop


//store calculated sum in sram
SRAM_STORE:
LDI XH, HIGH(0x0480) //middle of sram address high byte
LDI XL, LOW(0x0480) //middle of sram address low byte
ST X+, R17 //store lower byte of sum
ST X+, R18 //store higher byte of sum

//prepare to store sum in the middle of EEPROM
LDI YH, HIGH(0x01FF >> 2) //middle of eeprom address high byte
LDI YL, LOW(0x01FF >> 2) //middle of eeprom address low byte
MOV R16, R17 //prepare lwer byte of sum for storage in eeprom
CALL STORE_EEPROM //store lower byte
INC YL //increment eeprom address
MOV R16, R18 //prepare higher byte of sum for storage in eeprom
CALL STORE_EEPROM //store higher byte


//eeprom storage
STORE_EEPROM:
SBIC EECR, EEPE //wait iff eeprom write is in progress
RJMP STORE_EEPROM //jump to eeprom storage
OUT EEARH, YH //set eeprom address high byte
OUT EEARL, YL //set eeprom address low byte
OUT EEDR, R16 //set data to be writte
SBI EECR, EEMPE //master write enable
SBI EECR, EEPE //start eeprom wrtie
RET //return

.ORG 0x1EEF >> 1
SEQUENCE:
.DW 0x4865, 0x6c6c, 0x6f2c, 0x206d, 0x7920, 0x6e61, 0x6d65, 0x2069, 0x7320, 0x4a6f, 0x7368, 0x7561, 0x204d, 0x6172, 0x7469, 0x6e65