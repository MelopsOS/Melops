#include "stdio.h"
#include "stdint.h"

#define TextmodeHeight 25
#define TextmodeWidth 80

#define VGA 0xB8000

int cursor_pos = 0;

void readLine(int line, char *destination)
{
    if (line < 0 || line >= TextmodeHeight)
    {
        return;
    }

    uint16_t *vga_buffer = (uint16_t *)VGA;
    int start_line = line * TextmodeWidth;

    char debugstring[TextmodeWidth + 1];

    for (int col = 0; col < TextmodeWidth; col++)
    {
        uint16_t entry = vga_buffer[start_line + col];

        char c = (char)(entry & 0xFF);

        if (c == '\n')
        {
            break;
        }
        debugstring[col] = c;
        destination[col] = c;
    }
    debugstring[TextmodeWidth] = '\0';
    destination[TextmodeWidth] = '\0';

    // print(destination, 'R');
    //  print(debugstring, 'R');
}
/*JZ macht mir Angst*/
void clearScreen()
{
    char colorScheme = 'w';
    cursor_pos = 0;

    unsigned char *vga_buffer = (unsigned char *)VGA;
    int clearColor = (colorScheme == 'w') ? 0xF0 : 0x00;

    cursor_pos = 0;
    for (int i = 0; i < (TextmodeHeight * TextmodeWidth); i++)
    {
        vga_buffer[cursor_pos * 2] = ' ';
        vga_buffer[cursor_pos * 2 + 1] = clearColor;
        cursor_pos++;
    }
    cursor_pos = 0;
}

void print(char *output, char Color)
{
    char colorScheme = 'w';
    unsigned char *vga_buffer = (unsigned char *)VGA;
    int printColor;

    switch (Color)
    {
    case 'Y':
        printColor = (colorScheme == 'w') ? 0xF6 : 0x0E;
        break;
    case 'B':
        printColor = (colorScheme == 'w') ? 0xF9 : 0x09;
        break;
    case 'G':
        printColor = (colorScheme == 'w') ? 0xFA : 0x0A;
        break;
    case 'R':
        printColor = (colorScheme == 'w') ? 0xFC : 0x0C;
        break;
    case 'W':
        printColor = (colorScheme == 'w') ? 0xF0 : 0x07;
        break;
    case 'w':
        printColor = (colorScheme == 'w') ? 0xFF : 0xFF;
        break;
    default:
        printColor = (colorScheme == 'w') ? 0xFC : 0x0C;
    }

    for (int i = 0; output[i] != '\0'; i++)
    {
        if (cursor_pos > (TextmodeHeight * TextmodeWidth))
        {
            cursor_pos = 0;
        }
        if (output[i] == '\n')
        {
            cursor_pos = ((cursor_pos / TextmodeWidth) + 1) * TextmodeWidth;
            continue;
        }
        vga_buffer[cursor_pos * 2] = output[i];
        vga_buffer[cursor_pos * 2 + 1] = printColor;
        cursor_pos++;
    }
}
void IntParseString(int num, char *destination)
{

    char string[3];
    int bufferPos = 0;
    int number_buffer[3];

    int buffer;
    int buffer1;
    for (int i = 0; i < 3; i++)
    {

        switch (i)
        {
        case 0:
            buffer = num / 100;
            break;
        case 1:
            buffer = (num / 10) % 10;
            break;
        case 2:
            buffer = num % 10;
            break;
        default:
            print("Parsing err\nCould not parse /int/ to /string/", 'R');
        }
        number_buffer[i] = buffer;
    }

    for (int i = 0; i < 3; i++)
    {

        switch (number_buffer[i])
        {
        case 1:
            string[i] = '1';
            break;
        case 2:
            string[i] = '2';
            break;
        case 3:
            string[i] = '3';
            break;
        case 4:
            string[i] = '4';
            break;
        case 5:
            string[i] = '5';
            break;
        case 6:
            string[i] = '6';
            break;
        case 7:
            string[i] = '7';
            break;
        case 8:
            string[i] = '8';
            break;
        case 9:
            string[i] = '9';
            break;
        case 0:
            string[i] = '0';
            break;
        default:
            string[i] = '0';
        }
    }

    for (int i = 0; i < 3; i++)
    {
        destination[i] = string[i];
    }
    destination[3] = '\0';
    destination[4] = '\0';
}
int StringParseInt(char *str)
{
    int number = 0;
    int DigitBuffer[3];

    for (int i = 0; i < 3; i++)
    {

        DigitBuffer[i] = str[i] - '0';
    }

    number = (DigitBuffer[0] * 100) + (DigitBuffer[1] * 10) + (DigitBuffer[2]);

    return number;
}
void NewLine()
{
    print("\n", 'R');
}
