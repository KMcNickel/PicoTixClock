#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>

class Terminal
{
    public:
        typedef struct {
            char shortMatch;
            std::string longMatch;
            std::string description;
            void (*callback)(int argc, char * argv[]);
        } commandOptions;

        Terminal();
        Terminal(void (*printStrFunc)(char *));
        Terminal(void (*printCharFunc)(char));
        Terminal(void (*printStrFunc)(char *), void (*printCharFunc)(char));
        Terminal(bool usePutChar, bool usePrintf);

        void readInChar(char c);
        void setPrompt(std::string);
        void showPrompt();
        void addCommand(commandOptions opt);
        void addCommand(char shortMatch, 
                        std::string longMatch, 
                        std::string description, 
                        void (*callback)(int argc, char * argv[]));

    private:
        bool usePutChar = true;
        bool usePrintf = true;
        void (*printString)(char *);
        void (*printChar)(char);
        std::string currentCommand;
        std::string prompt;
        std::list<commandOptions> options;

        void displayChar(char c);
        void displayString(char * s);
        void displayString(std::string);
};