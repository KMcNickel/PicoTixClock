#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "include/terminal.h"

void Terminal::readInChar(char c)
{
    if(c == '\n' || c == '\t') {}
    else if(c == '\r')
    {
        showPrompt();
    }
    else
    {
        currentCommand.push_back(c);
        displayChar(c);
    }
}

void Terminal::displayChar(char c)
{
    if(usePutChar) putchar((int) c);
    else printChar(c);
}

void Terminal::displayString(char * s)
{
    if(usePrintf) printf("%s", s);
    else printString(s);
}

void Terminal::displayString(std::string s)
{
    if(usePrintf) printf("%s", s.c_str());
    else printString((char *) s.c_str());
}

void Terminal::showPrompt()
{
    std::string fullPrompt = "\r\n";
    fullPrompt.append(prompt);
    fullPrompt.append(" >");
    displayString(fullPrompt);
}

void Terminal::setPrompt(std::string str)
{
    prompt = str;
}

Terminal::Terminal(){};

Terminal::Terminal(void (*printStrFunc)(char *))
{
    printString = printStrFunc;
    usePrintf = false;
}

Terminal::Terminal(void (*printCharFunc)(char))
{
    printChar = printCharFunc;
    usePutChar = false;
}

Terminal::Terminal(void (*printStrFunc)(char *), void (*printCharFunc)(char))
{
    printChar = printCharFunc;
    printString = printStrFunc;
    usePrintf = false;
    usePutChar = false;
}