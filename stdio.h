#ifndef STDIO_H_
#define STDIO_H_

extern int cursor_pos;
void readLine(int line, char *destination);
void clearScreen();
void print(char *output, char Color);
void IntParseString(int num, char *destination);
int StringParseInt(char *str);
void NewLine();
#endif