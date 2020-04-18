# Instructions
For valid instructions, see [6502 Instruction set](https://www.masswerk.at/6502/6502_instruction_set.html)
## Example

### Indirect addressing
```assembly
JMP ($8000)  ; Indirect JMP
LDA ($00), Y ; Indirect Y-indexed

```

### Absolute addressing
```assembly
STA $2008 ; Directly referencing an address
JMP print ; A label is also an absolute address
```

### Relative addressing
```assembly
BNE loop
BEQ loop2
```

### Immediate addressing
Immediate values are denoted by a # followed by a 8bit value  
Hexadecimal values and decimal values are allowed e.g. $80 or 10  
The hexadecimal notation that is supported uses $, not 0x  
```assembly
LDA #$66
LDY #10
```

Since labels are 16 bit addresses, to load an address into a register you have to do it in 2 steps  
and requires special syntax
```assembly
LDA #<ok ; ok is a label to a string. < means the low part(LSB)
LDX #>ok ; ok_high (MSB)
```

## Special symbols
### .org
The location counter will jump to the address suceeding the .org symbol  
Must be a 16bit address in hexadecimal notation
```assembly
.org $FFFC
```

### .db
Allocate one or more bytes  
Strings, hexadecimal values and decimal values are allowed
```assembly
data: ; Give the data a label to be able to reference it later
.db "Hello world", 10, 0 ; 10 is ascii for new line, 0 is null terminator
```

### .dw
Allocate 2 bytes = 1 word  
Hexadecimal addresses 16 bit and labels are allowed
```assembly
.dw $8000
.dw main
```

## Labels
Supported characters are [a-z] and _  
Examples:
```assembly
label:
another_label:
```

## Comments
Comments are written with the semicolon character ;  
```assembly
; Multiple
; lines of
; comments
```

## Macros
Macros can be used to define one or more lines of code that can be called by a macro name followed by arguments  
Max arguments is set to 3 atm, can be changed later if its needed. Arguments are a space separated list  
%1, %2, %3 is used to reference the arguments in the macro  
All macros start with #macro NAME N  
NAME can be anything
N is 0, 1, 2 or 3

Macros are simply text that gets replaced before the assembling occurs, so no special logic for macros, they are simply cut and paste
```assembly
#macro LOAD_Y_CMP 1
LDY %1
CPY $2000
#endmacro

#macro PRINT_A 0 STA $2009 #endmacro

#macro ADC_CMP 3
LDA %1
ADC %2
CMP %3
#endmacro
```
They are used later by just using the macro name followed by its arguments
```assembly
LDA #$00
PRINT_A ; 0 arguments

LOAD_Y_CMP #$0A ; 1 argument

ADC_CMP #$10 #$20 #$30 ; 3 argument example
```
