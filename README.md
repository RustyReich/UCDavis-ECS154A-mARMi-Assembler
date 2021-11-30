# UCDavis-ECS154A-mARMi-Assembler
This is an assembler for the mARMi CPU that we were assigned to design in ECS154A in Fall 2021 at UC Davis

This assembler allows you to write traditional-style assembly code and have it converted to hex instructions that you can import into logisim.

Blank lines will automaticall be ignored, and in-line comments are supported using the '#' character

How to use:
1. Write a program
```
# This program computes the factorial of the value at address 0 in RAM
# The factorial will be stored in address 1 in RAM

MOV r3, 0	# r3 will always point to [0]

MOV r2, 1	# Factorial is always at least 1
STR r2, r2, r3	# Store 1 in [1]

MOV r1, 1 	# Our decrement amount
LDR r0, r3, r3 	# [0] -> r0

MUL r2, r2, r0	# _LOOP
SUB r0, r0, r1	# decrement r0
BNE -3		# jmp back to _LOOP if r0 != 0
STR r2, r1, r3	# Store final result in [1] 
HALT
```
2. Save program to the same directory as mARMi_Assembler
3. Open a terminal window and navigate to the directory with mARMi_Assembler and your program file
4. Run:
    `./mARMi_Assembler NAME_OF_FILE` on linux or `mARMi_Assembler.exe NAME_OF_FILE` on windows
5. A file named `NAME_OF_FILE_output` will be generated in the same directory
    
    Here is the example `Factorial_output` from above:
    ```
    v2.0 raw
    b000 a001 1893 9001 10c3 2490 2201 7ffd 
    1853 0600 
    ```
6. Load 'NAME_OF_FILE_output' into the ROM module in logisim and run your program!
