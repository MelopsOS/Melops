#include "TInterpreter.h"
#include "string.h"
#include "stdio.h"

char CommandCountStr_buffer[4];
char CommandStr_buffer[50];
char CommandParameter1_buffer[50];
char CommandParameter2_buffer[50];

int CommandCount;

int Command_token;
int CommandParameter1_token;
int CommandParameter2_token;

int DebugMode = 0;

typedef enum
{
    PRINT,
    SYS,
    DOC,
    RED,
    CLEAR,
    YELLOW,
    BLUE,
    BLACK,
    GREEN
} Token;

void ClearBuffers()
{
    CommandCountStr_buffer[0] = '\0';
    CommandStr_buffer[0] = '\0';
    CommandParameter1_buffer[0] = '\0';
    CommandParameter2_buffer[0] = '\0';

    Command_token = -1;
    CommandParameter1_token = -1;
    CommandParameter2_token = -1;
}

Token t;

void Tokennizer(int line)
{
    stringStream(line, CommandCountStr_buffer, 1);
    stringStream(line, CommandStr_buffer, 2);
    stringStream(line, CommandParameter1_buffer, 3);
    stringStream(line, CommandParameter2_buffer, 4);

    if (DebugMode == 1)
    {
        NewLine();
        print(CommandCountStr_buffer, 'R');
        NewLine();
        print(CommandStr_buffer, 'R');
        NewLine();
        print(CommandParameter1_buffer, 'R');
        NewLine();
        print(CommandParameter2_buffer, 'R');
        NewLine();
    }
}
void Lexer()
{
    CommandCount = StringParseInt(CommandCountStr_buffer);
    if (DebugMode == 1)
    {
        char DebugCommandCountStr[4];
        IntParseString(CommandCount, DebugCommandCountStr);
        print(DebugCommandCountStr, 'R');
        NewLine();
    }
    // Command Identifier
    if (strcmp(CommandStr_buffer, "print") == 0)
    {
        Command_token = PRINT;
        if (DebugMode == 1)
        {
            print("Command: print\n", 'R');
        }
    }
    else if (strcmp(CommandStr_buffer, "sys") == 0)
    {
        Command_token = SYS;
        if (DebugMode == 1)
        {
            print("Command: sys\n", 'R');
        }
    }
    else if (strcmp(CommandStr_buffer, "doc") == 0)
    {
        if (DebugMode == 1)
        {
            print("Command: doc\n", 'R');
        }
        Command_token = DOC;
    }
    else if (strcmp(CommandStr_buffer, "clear") == 0)
    {
        Command_token = CLEAR;
    }
}
void Interpreter(int line)
{
    ClearBuffers();
    Tokennizer(line);
    Lexer();
    char color;
    if (Command_token == PRINT)
    {
        if (strcmp(CommandParameter2_buffer, "r") == 0)
        {
            color = 'R';
            print("Color: RED\n", 'R');
        }
        else if (strcmp(CommandParameter2_buffer, "b") == 0)
        {
            color = 'B';
            print("Color: BLUE\n", 'B');
        }
        else if (strcmp(CommandParameter2_buffer, "s") == 0)
        {
            color = 'W';
            print("Color: BLUE\n", 'B');
        }
        else if (strcmp(CommandParameter2_buffer, "y") == 0)
        {
            color = 'Y';
            print("Color: YELLOW\n", 'Y');
        }
        clearScreen();
        for (int i = 0; i < CommandCount; i++)
        {

            print(CommandParameter1_buffer, color);
            NewLine();
            if (i > 23)
            {
                break;
            }
        }
    }
    else if (Command_token == SYS)
    {
        clearScreen();
        print("|0     0|0  |00000 |0     ", 'W');
        print("                \n", 'B');
        print("|0 0  0 |0  |0     |0     ", 'W');
        print("|00  |000   |00 \n", 'B');
        print("|0  00  |0  |000   |0     ", 'W');
        print("|0  0 |0  0 |0  \n", 'B');
        print("|0      |0  |0     |0     ", 'W');
        print("|0  0 |000    0|\n ", 'B');
        print("|0      |0  |00000 |000000", 'W');
        print("|00  |0    00|  \n", 'B');

        NewLine();
        NewLine();
        print("Melops/OS Version 26.3\n", 'W');
        print("Visit https://melopsos.github.io To get the newest Version\n", 'W');
        print("Released 21.06.2026\n", 'Y');

        print("type /clear/ to get to the terminal!\n", 'W');
    }
    else if (Command_token == DOC)
    {
        clearScreen();
        print("Melops/Documentation\n", 'Y');
        NewLine();
        print("Terminal/Documentation\n", 'Y');
        print("Syntax:\n", 'W');
        print("Count(Example 001) --- Command --- Parameter 1 --- Parameter 2\n", 'W');
        print("Commandlist\n", 'W');
        print("print - Prints to the terminal - Parameter1 = Text, Parameter = Color\n", 'W');
        print("doc - enters the Documentation\n", 'W');
        print("sys - shows Melops version\n", 'W');
        print("clear - clears screen\n", 'W');
    }
    else if (Command_token == CLEAR)
    {
        clearScreen();
        print("--- Melops/OS/ Terminal ---\n", 'Y');
    }
    ClearBuffers();
    return;
}