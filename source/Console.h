/*
 * Console.h
 *
 *  Created on: 22.12.2019
 *      Author: Marcin
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "mbed.h"
#include <string>
#include <vector>
#include <map>
#include <utility>

enum class KeyCode : int
{
    LF = 10,
    CR = 13,
    Escape = 27,
    Tilde = 126,
    Backspace = 8
};

using CommandVector = std::vector<std::string>;
using CommandContainer = std::pair<std::string, Callback<void(CommandVector)>>;

class Console
{
public:
    Console();
    void handler(void);
    void registerCommand(std::string command, std::string helpText, Callback<void(CommandVector)> commandCallback);
    void displayHelp(CommandVector cv);
private:
    void executeCommand(void);
    CommandVector commandElements;
    std::map<std::string, CommandContainer> commands;
};

#endif /* CONSOLE_H_ */
