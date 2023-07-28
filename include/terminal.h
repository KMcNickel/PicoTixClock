#include <stdio.h>
#include <stdlib.h>
#include <string>

class Terminal
{
    public:
        Terminal();
        Terminal(void (*printStrFunc)(char *));
        Terminal(void (*printCharFunc)(char));
        Terminal(void (*printStrFunc)(char *), void (*printCharFunc)(char));
        Terminal(bool usePutChar, bool usePrintf);
        void readInChar(char c);
        void setPrompt(std::string);
        void showPrompt();

    private:
        bool usePutChar = true;
        bool usePrintf = true;
        void (*printString)(char *);
        void (*printChar)(char);
        std::string currentCommand;
        std::string prompt;

        void displayChar(char c);
        void displayString(char * s);
        void displayString(std::string);
};