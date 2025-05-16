//
// Created by Selim Dalçiçek on 9.05.2025.
//

#pragma once

#include "database.hpp"
#include <string>
#include <fmt/ranges.h>
#include <fmt/format.h>


enum class CommandType{
  CREATE_TABLE,
  INSERT,
  SELECT,
  DROP_TABLE,
  ALTER_TABLE,
  UPDATE,
  SAVE_TO,
  LOAD_FROM,
  UNKNOWN
};

// CommandParser is a utility class that interprets and executes user commands.
// It uses identifyCommand() to detect the command type,
// and executeCommand() to carry out the appropriate database action.
class CommandParser {
public:
  static CommandType identifyCommand(const std::string& command);       // Detects command type
  static void executeCommand(const std::string& command, Database& db); // Executes the parsed command
};
