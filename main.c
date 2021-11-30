#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

typedef struct nibble 
{

    char data[5];

} nibble;

#define TOKEN_TOO_LARGE     1
#define UNKNOWN_INSTRUCTION 2

void remove_all_chars(char* str, char c);
char* int_to_binary_string(char* num, unsigned short len);
char nibble_to_char(nibble nib);
bool ALUinstruction(char* instruction);
const char* ALUop(char* instruction);
void invert_binary_string(char* str);
void add_one_to_binary_string(char* str);
bool stringEmpty(char* str);
void removeComments(char* str);

int main(int argc, char* argv[])
{

    FILE* input = NULL;
    FILE* output = NULL;

    //Open assembly file
    input = fopen(argv[1], "r");
    
    //Create output machine code file as "{input file name}_output"
    char* outFilename = NULL;
    outFilename = calloc(sizeof(*outFilename), strlen(argv[1]));
    strcpy(outFilename, argv[1]);
    strcat(outFilename, "_output\0");
    output = fopen(outFilename, "w");
    free(outFilename);

    //Print logisim header
    fprintf(output, "v2.0 raw\n");

    char currentInput[255];

    if (input == NULL)
        exit(EXIT_FAILURE);
    else
    {
    
        int currentLine = 1;

        //Error handlers
        bool error = false;
        char* errorString;
        int errorType = 0;

        //Process current input
        while(fgets(currentInput, sizeof(currentInput), input) != NULL && !error)
        {            

            //Remove in-line comments
            removeComments(currentInput);
            
            //Skip empty lines
            if (!stringEmpty(currentInput))
            {
                
                //Store the current instruction in binary form
                char currentBinary[17];
                strcpy(currentBinary, "\0");

                //Convert currentInput to lowercase
                for (unsigned short i = 0; i < strlen(currentInput); i++)
                    currentInput[i] = tolower(currentInput[i]);

                //Remove all commas
                remove_all_chars(currentInput, ',');
                //Remove all newlines
                remove_all_chars(currentInput, '\n');

                //Each instruction is made of 4 nibbles
                nibble Nibbles[4];
                strcpy(Nibbles[0].data, "\0");
                strcpy(Nibbles[1].data, "\0");
                strcpy(Nibbles[2].data, "\0");
                strcpy(Nibbles[3].data, "\0");

                //Each input is made up of an instruction and 1-3 arguments
                    //Initialize all of these to be empty so that we can check for
                    //their existence using strlen
                char instruction[] = { '\0', '\0', '\0', '\0', '\0', '\0'};
                char arg1[] = { '\0', '\0', '\0', '\0', '\0', '\0'};
                char arg2[] = { '\0', '\0', '\0', '\0', '\0', '\0'};
                char arg3[] = { '\0', '\0', '\0', '\0', '\0', '\0'};

                //Split currentInput into intruction, arg1, arg2, arg3
                char *token = strtok(currentInput, " ");
                unsigned short currentToken = 0;
                while (token != NULL)
                {
                    
                    //Buffer overflow error. Token is too large
                    if (strlen(token) > sizeof(arg1) / sizeof(char) - 1)
                    {

                        error = true;
                        errorString = token;
                        errorType = TOKEN_TOO_LARGE;
                        break;

                    }

                    if (currentToken == 0)
                        strcpy(instruction, token);
                    else if (currentToken == 1)
                        strcpy(arg1, token);
                    else if (currentToken == 2)
                        strcpy(arg2, token);
                    else if (currentToken == 3)
                        strcpy(arg3, token);


                    token = strtok(NULL, " ");
                    currentToken += 1;

                }

                //Catch buffer overflow errors
                if (error)
                    break;

                //Strings to hold register addresses
                char *RDstring = NULL;
                char *RNstring = NULL;
                char *RMstring = NULL;

                //String to hold immediate value
                char *IMMstring = NULL;
                
                //Store a few boolean variables to identify different types of
                //instructions
                bool MOV_register_register = false;
                bool MOV_flags = false;

                if (arg2[0] == 'r')
                    MOV_register_register = true;
                else if (strcmp(arg2, "flags") == 0)
                    MOV_flags = true;

                //Remove 'r' from args
                remove_all_chars(arg1, 'r');
                remove_all_chars(arg2, 'r');
                remove_all_chars(arg3, 'r');

                //This is the section where we parse the 
                if (ALUinstruction(instruction) == true)
                {

                    strcat(currentBinary, "001");

                    //Append the correct ALU OPcode
                    strcat(currentBinary, ALUop(instruction));
                    
                    RDstring = int_to_binary_string(arg1, 3);
                    RNstring = int_to_binary_string(arg2, 3);
                    RMstring = int_to_binary_string(arg3, 3);

                    //Write RnRdRm to currentBinary
                    strcat(currentBinary, RNstring);
                    strcat(currentBinary, RDstring);
                    //If arg3 doesn't exist, that means we have a NOT instruction
                        //Not instruction only takes 2 registers as input, so just
                        //write "000" in place of the last register
                    if (strlen(arg3) != 0)
                        strcat(currentBinary, RMstring);
                    else
                        strcat(currentBinary, "000");
                    
                }
                else if (strcmp(instruction, "mov") == 0)
                {

                    if (MOV_register_register)   //MOV Rd, Rn
                    {

                        strcat(currentBinary, "0000100");

                        RDstring = int_to_binary_string(arg1, 3);
                        RNstring = int_to_binary_string(arg2, 3);

                        strcat(currentBinary, RNstring);
                        strcat(currentBinary, RDstring);
                        strcat(currentBinary, "000");

                    }
                    else if (MOV_flags) //MOV Rd, Flags
                    {

                        strcat(currentBinary, "0000101");
                        
                        RDstring = int_to_binary_string(arg1, 3);

                        strcat(currentBinary, "000");
                        strcat(currentBinary, RDstring);
                        strcat(currentBinary, "000");

                    }
                    else    //MOV rd, imm
                    {

                        strcat(currentBinary, "1");

                        //Convert register ID to to binary string
                        RDstring = int_to_binary_string(arg1, 3);

                        strcat(currentBinary, RDstring);

                        //Convert imm to binary string
                        IMMstring = int_to_binary_string(arg2, 12);

                        //Copy over all imm to currentBinary
                        strcat(currentBinary, IMMstring);

                    }

                }
                else if (strcmp(instruction, "str") == 0)
                {

                    strcat(currentBinary, "0001100");

                    RDstring = int_to_binary_string(arg1, 3);
                    RNstring = int_to_binary_string(arg2, 3);
                    RMstring = int_to_binary_string(arg3, 3);

                    strcat(currentBinary, RNstring);
                    strcat(currentBinary, RDstring);
                    strcat(currentBinary, RMstring);

                }
                else if (strcmp(instruction, "ldr") == 0)
                {

                    strcat(currentBinary, "0001000");

                    RDstring = int_to_binary_string(arg1, 3);
                    RNstring = int_to_binary_string(arg2, 3);
                    RMstring = int_to_binary_string(arg3, 3);

                    strcat(currentBinary, RNstring);
                    strcat(currentBinary, RDstring);
                    strcat(currentBinary, RMstring);

                }
                else if (instruction[0] == 'b') //branch instructions
                {

                    strcat(currentBinary, "01");

                    if (strcmp(instruction, "b") == 0)
                        strcat(currentBinary, "00");
                    else if (strcmp(instruction, "beq") == 0)
                        strcat(currentBinary, "10");
                    else if (strcmp(instruction, "bne") == 0)
                        strcat(currentBinary, "11");
                    else
                    {
                
                        error = true;
                        errorString = instruction;
                        errorType = UNKNOWN_INSTRUCTION;

                    }

                    //Convert imm to binary string
                    IMMstring = int_to_binary_string(arg1, 12);

                    //Copy over all imm to currentBinary
                    strcat(currentBinary, IMMstring);

                }
                else if (strcmp(instruction, "halt") == 0)
                    strcat(currentBinary, "0000011000000000");
                else if (strcmp(instruction, "nop") == 0)
                    strcat(currentBinary, "0000000000000000");
                else
                {
                
                    error = true;
                    errorString = instruction;
                    errorType = UNKNOWN_INSTRUCTION;

                }

                //Free memory
                if (RDstring != NULL)
                {

                    free(RDstring);
                    RDstring = NULL;

                }
                if (RMstring != NULL)
                {

                    free(RMstring);
                    RMstring = NULL;

                }
                if (RNstring != NULL)
                {

                    free(RNstring);
                    RNstring = NULL;

                }
                if (IMMstring != NULL)
                {

                    free(IMMstring);
                    IMMstring = NULL;

                }

                //Catch unknown instruction errors
                if (error)
                    break;

                //Transfer binary data over to nibbles
                for (unsigned short i = 0; i < 16; i++)
                    Nibbles[i / 4].data[i % 4] = currentBinary[i];

                //Print final HEX of currentInput to output
                for (unsigned short i = 0; i < 4; i++)
                    fprintf(output, "%c", nibble_to_char(Nibbles[i]));
                fprintf(output, " ");

                //8 instructions per line
                if (currentLine % 8 == 0 && currentLine >= 8)
                    fprintf(output, "\n");
                currentLine++;

            }

        }

        if (error)
        {

            if (errorType == TOKEN_TOO_LARGE)
                printf("ERROR: token '%s' too large\n", errorString);
            else if (errorType == UNKNOWN_INSTRUCTION)
                printf("ERROR: unknown instruction '%s'\n", errorString);

        }

    }

    fclose(input);
    fclose(output);

}

void removeComments(char* str)
{
    
    //Check if there is a comment
    if (strchr(str, '#') != NULL)
    {
        
        //Replace all character after '#' with '\0'
        char* commentPointer = strchr(str, '#');
        for (unsigned short i = 0; i < strlen(commentPointer); i++)
            commentPointer[i] = '\0';

    }

}

bool stringEmpty(char* str)
{

    //If a line only contains spaces and newlines, it is considered empty
    for (unsigned short i = 0; i < strlen(str); i++)
        if (str[i] != ' ' && str[i] != '\n')
            return false;
    return true;

}

const char* ALUop(char* instruction)
{

    if (strcmp(instruction, "add") == 0)
        return "0000";
    else if (strcmp(instruction, "sub") == 0)
        return "0001";
    else if (strcmp(instruction, "mul") == 0)
        return "0010";
    else if (strcmp(instruction, "mulu") == 0)
        return "0011";
    else if (strcmp(instruction, "div") == 0)
        return "0100";
    else if (strcmp(instruction, "mod") == 0)
        return "0101";
    else if (strcmp(instruction, "and") == 0)
        return "0110";
    else if (strcmp(instruction, "or") == 0)
        return "0111";
    else if (strcmp(instruction, "eor") == 0)
        return "1000";
    else if (strcmp(instruction, "not") == 0)
        return "1001";
    else if (strcmp(instruction, "lsl") == 0)
        return "1010";
    else if (strcmp(instruction, "lsr") == 0)
        return "1011";
    else if (strcmp(instruction, "asr") == 0)
        return "1100";
    else if (strcmp(instruction, "rol") == 0)
        return "1101";
    else if (strcmp(instruction, "ror") == 0)
        return "1110";
    else
        return "1111";

}

bool ALUinstruction(char* instruction)
{

    return (strcmp(instruction, "add") == 0 || strcmp(instruction, "sub") == 0 ||
            strcmp(instruction, "mul") == 0 || strcmp(instruction, "mulu") == 0 ||
            strcmp(instruction, "div") == 0 || strcmp(instruction, "mod") == 0 ||
            strcmp(instruction, "and") == 0 || strcmp(instruction, "or") == 0 ||
            strcmp(instruction, "eor") == 0 || strcmp(instruction, "not") == 0 ||
            strcmp(instruction, "lsl") == 0 || strcmp(instruction, "lsr") == 0 ||
            strcmp(instruction, "asr") == 0 || strcmp(instruction, "rol") == 0 ||
            strcmp(instruction, "ror") == 0);

}

char nibble_to_char(nibble nib)
{

    if (strcmp(nib.data, "0000") == 0)
        return '0';
    else if (strcmp(nib.data, "0001") == 0)
        return '1';
    else if (strcmp(nib.data, "0010") == 0)
        return '2';
    else if (strcmp(nib.data, "0011") == 0)
        return '3';
    else if (strcmp(nib.data, "0100") == 0)
        return '4';
    else if (strcmp(nib.data, "0101") == 0)
        return '5';
    else if (strcmp(nib.data, "0110") == 0)
        return '6';
    else if (strcmp(nib.data, "0111") == 0)
        return '7';
    else if (strcmp(nib.data, "1000") == 0)
        return '8';
    else if (strcmp(nib.data, "1001") == 0)
        return '9';
    else if (strcmp(nib.data, "1010") == 0)
        return 'a';
    else if (strcmp(nib.data, "1011") == 0)
        return 'b';
    else if (strcmp(nib.data, "1100") == 0)
        return 'c';
    else if (strcmp(nib.data, "1101") == 0)
        return 'd';
    else if (strcmp(nib.data, "1110") == 0)
        return 'e';
    else
        return 'f';

}

void invert_binary_string(char* str)
{

    for (unsigned short i = 0; i < strlen(str); i++)
        if (str[i] == '0')
            str[i] = '1';
        else if (str[i] == '1')
            str[i] = '0';

}

void add_one_to_binary_string(char* str)
{

    //convert string back to integer
    int num = 0;
    for (unsigned short i = 0; i < strlen(str); i++)
        if (str[i] == '1')
            num += pow(2.0, strlen(str) - i - 1);
    
    //Add one
    num += 1;

    //Convert new number to binary string and copy back to str
    sprintf(str, "%d", num);
    char* tempString = int_to_binary_string(str, 12);
    for (unsigned short i = 0; i < strlen(tempString); i++)
        str[i] = tempString[i];

    free(tempString);

}

char* int_to_binary_string(char* num, unsigned short len)
{

    char tempString[] = "000000000000\0";

    int n = atoi(num);
    if (n < 0)
        n *= -1;

    short count = 11;
    while (count >= 0)
    {

        tempString[count] = (n % 2) + 48;
        n = n / 2;

        count--;

    }

    if (atoi(num) < 0)
    {

        invert_binary_string(tempString);
        add_one_to_binary_string(tempString);

    }

    char* returnString = calloc(sizeof(*returnString), len + 1);
    returnString[len] = '\0';
    for (unsigned short i = 0; i < len; i++)
        returnString[i] = tempString[12 - len + i];

    return returnString;

}

void remove_all_chars(char* str, char c) 
{
    
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';

}