#include "string.h"
#include "stdio.h"
#include "stdmm.h"
#include "stddef.h"

#define TextmodeWidth 80

int strcmp(char *string1, char *string2)
{

    for (int i = 0; string1[i] != '\0' || string2[i] != '\0'; i++)
    {
        if (string1[i] == string2[i])
        {
            continue;
        }
        else
        {
            return 1;
            break;
        }
        if (string1[i] == '\0' || string2[i] == '\0')
        {
            break;
        }
    }
    return 0;
}

void stringStream(int line, char *destination, int SelectWord)
{
    char WordBuffer[80];
    int CurrentWord = 1;
    int WordIndex = 0;
    char *LineBuffer = NULL;
    LineBuffer = (char *)pmm_alloc_block();

    if (LineBuffer == NULL)
    {
        print(":( Kernel PANIC:\nFile: string.c\nFunction: stringStream", 'R');
    }
    readLine(line, LineBuffer);

    for (int i = 0; LineBuffer[i] != '\0' || LineBuffer[i] != '\n'; i++)
    {
        if (i > TextmodeWidth)
        {
            break;
        }
        if (LineBuffer[i] == ' ')
        {
            CurrentWord++;
            continue;
        }
        if (SelectWord == CurrentWord)
        {
            WordBuffer[WordIndex] = LineBuffer[i];
            destination[WordIndex] = LineBuffer[i];
            WordIndex++;
        }
    }
    WordBuffer[WordIndex++] = '\0';
    destination[WordIndex++] = '\0';
    // print(WordBuffer, 'R');
}
