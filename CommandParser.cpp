#include "CommandParser.hpp"
#include <sstream>
#include <iostream>
#include <algorithm> // for std::transform
#include "fmt/xchar.h"


// Identify which command type this input matches (case-insensitive)

CommandType CommandParser::identifyCommand(const std::string& input) {
    std::string upper = input;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper.starts_with("CREATE_TABLE")) return CommandType::CREATE_TABLE;
    if (upper.starts_with("INSERT INTO")) return CommandType::INSERT;
    if (upper.starts_with("SELECT")) return CommandType::SELECT;
    if (upper.starts_with("DROP_TABLE")) return CommandType::DROP_TABLE;
    if (upper.starts_with("ALTER TABLE") || upper.starts_with("ALTER_TABLE")) return CommandType::ALTER_TABLE;
    if (upper.starts_with("UPDATE")) return CommandType::UPDATE;
    if (upper.starts_with("SAVE TO")) return CommandType::SAVE_TO;
    if (upper.starts_with("LOAD_FROM")) return CommandType::LOAD_FROM;

    return CommandType::UNKNOWN;
}

// Main command dispatcher

void CommandParser::executeCommand(const std::string& input, Database& db) {
    switch (identifyCommand(input)) {

        // CREATE_TABLE Students(ID INT, Name STRING, ...)
        case CommandType::CREATE_TABLE: {
            size_t parenIndex = input.find('(');
            if (parenIndex == std::string::npos) {
                std::cerr << " Syntax error: missing opening parenthesis.\n";
                return;
            }

            std::string header = input.substr(0, parenIndex);
            std::istringstream headerStream(header);
            std::string cmd, tableName;
            headerStream >> cmd >> tableName;

            size_t start = input.find('(');
            size_t end = input.find(')');
            if (start == std::string::npos || end == std::string::npos) {
                std::cerr << " Syntax error: missing column definition.\n";
                return;
            }

            std::string columnDefs = input.substr(start + 1, end - start - 1);
            std::istringstream columnStream(columnDefs);
            std::vector<Column> columns;
            std::string token;

            while (std::getline(columnStream, token, ',')) {
                std::istringstream pairStream(token);
                std::string name, type;
                pairStream >> name >> type;

                // Ensure case-insensitive type handling
                std::transform(type.begin(), type.end(), type.begin(), ::toupper);

                if (type == "INT") columns.push_back({name, DataType::INT});
                else if (type == "FLOAT") columns.push_back({name, DataType::FLOAT});
                else if (type == "STRING") columns.push_back({name, DataType::STRING});
                else if (type == "BOOLEAN" || type == "BOOL") columns.push_back({name, DataType::BOOL});
                else std::cerr << " Unknown column type: " << type << "\n";
            }

            db.createTable(tableName, columns);
            fmt::println(" Table '{}' created with {} columns:", tableName, columns.size());
            for (const auto& col : columns) {
                fmt::println("- {:<12} : {}", col.name, dataTypeToString(col.type));
            }
            break;
        }        // ========== INSERT INTO Students VALUES (...) ==========
        case CommandType::INSERT: {
            std::istringstream stream(input);
            std::string cmd, into, tableName;
            stream >> cmd >> into >> tableName;

            // Extract values inside parentheses
            size_t openParen = input.find('(');
            size_t closeParen = input.find(')');
            if (openParen == std::string::npos || closeParen == std::string::npos) {
                std::cerr << " Syntax error in INSERT command.\n";
                return;
            }

            std::string raw = input.substr(openParen + 1, closeParen - openParen - 1);
            std::vector<Value> values;
            std::string current;
            bool inQuotes = false;

            // Split values safely (handles quoted strings)
            for (char c : raw) {
                if (c == '"') inQuotes = !inQuotes;
                if (c == ',' && !inQuotes) {
                    std::string trimmed;
                    std::remove_copy_if(current.begin(), current.end(), std::back_inserter(trimmed), ::isspace);

                    if (trimmed.front() == '"' && trimmed.back() == '"')
                        values.push_back(trimmed.substr(1, trimmed.size() - 2));
                    else if (trimmed == "true" || trimmed == "false")
                        values.push_back(trimmed == "true");
                    else if (trimmed.find('.') != std::string::npos)
                        values.push_back(std::stof(trimmed));
                    else
                        values.push_back(std::stoi(trimmed));

                    current.clear();
                } else {
                    current += c;
                }
            }

            // Handle last value
            if (!current.empty()) {
                std::string trimmed;
                std::remove_copy_if(current.begin(), current.end(), std::back_inserter(trimmed), ::isspace);

                if (trimmed.front() == '"' && trimmed.back() == '"')
                    values.push_back(trimmed.substr(1, trimmed.size() - 2));
                else if (trimmed == "true" || trimmed == "false")
                    values.push_back(trimmed == "true");
                else if (trimmed.find('.') != std::string::npos)
                    values.push_back(std::stof(trimmed));
                else
                    values.push_back(std::stoi(trimmed));
            }

            // Locate target table
            Table* table = db.getTable(tableName);
            if (!table) {
                std::cerr << " Table not found: " << tableName << "\n";
                return;
            }

            // Insert row and handle duplicate key errors
            try {
                table->addRow(values);
                fmt::println(" Row inserted into '{}'.", tableName);
            } catch (const std::exception& e) {
                std::cerr << " Insert error: " << e.what() << "\n";
            }

            break;
        }
        //  DROP_TABLE Students
        case CommandType::DROP_TABLE: {
            std::istringstream stream(input);
            std::string command, tableName;
            stream >> command >> tableName;

            if (!tableName.empty() && tableName.back() == ';') {
                tableName.pop_back();
            }

            try {
                db.dropTable(tableName);
                fmt::println(" Table '{}' deleted.", tableName);
            } catch (const std::exception& e) {
                std::cerr << " Drop error: " << e.what() << "\n";
            }
            break;
        }

        // ========== SELECT * FROM Students; or SELECT Name, GPA FROM ... ==========
        case CommandType::SELECT: {
            std::string query = input;
            if (!query.empty() && query.back() == ';') query.pop_back();

            std::string upperQuery = query;
            std::transform(upperQuery.begin(), upperQuery.end(), upperQuery.begin(), ::toupper);

            size_t fromPos = upperQuery.find("FROM");
            if (fromPos == std::string::npos) {
                std::cerr << " SELECT syntax error: missing FROM.\n";
                return;
            }

            std::string colPart = query.substr(7, fromPos - 7);
            std::string afterFrom = query.substr(fromPos + 5);

            std::string tableName;
            std::string condition;

            size_t wherePos = upperQuery.find("WHERE");
            if (wherePos != std::string::npos) {
                tableName = afterFrom.substr(0, wherePos - fromPos - 6);
                condition = query.substr(wherePos + 6);
            } else {
                tableName = afterFrom;
            }
            // Remove all whitespace characters from tableName (leading, trailing, and in-between)
            tableName.erase(std::remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

            Table* table = db.getTable(tableName);
            if (!table) {
                std::cerr << " Table not found: " << tableName << "\n";
                return;
            }

            std::vector<int> selectedIndex;
            std::vector<std::string> selectedNames;
            // * successfully find
            if (colPart.find('*') != std::string::npos) {
                for (size_t i = 0; i < table->columns.size(); ++i) {
                    selectedIndex.push_back(i);
                    selectedNames.push_back(table->columns[i].name);
                }
            } else {
                std::istringstream stream(colPart);
                std::string col;
                while (std::getline(stream, col, ',')) {
                    col.erase(std::remove_if(col.begin(), col.end(), ::isspace), col.end());
                    bool found = false;
                    for (size_t i = 0; i < table->columns.size(); ++i) {
                        if (table->columns[i].name == col) {
                            selectedIndex.push_back(i);
                            selectedNames.push_back(col);
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        std::cerr << " Column not found: " << col << "\n";
                        return;
                    }
                }
            }

            for (const auto& colName : selectedNames) {
                fmt::print("{:<15}", colName);
            }
            fmt::print("\n");

            for (const auto& row : table->rows) {
                bool show = true;

                if (!condition.empty()) {
                    std::istringstream condStream(condition);
                    std::string condCol, op, valueStr;
                    condStream >> condCol >> op >> valueStr;

                    int condIndex = -1;
                    for (size_t i = 0; i < table->columns.size(); ++i) {
                        if (table->columns[i].name == condCol) {
                            condIndex = static_cast<int>(i);
                            break;
                        }
                    }

                    if (condIndex == -1) {
                        std::cerr << " WHERE column not found: " << condCol << "\n";
                        continue;
                    }
                    // WHERE BLOCK IMPLEMENTATION
                    const Value& actual = row.values[condIndex];
                    op.erase(std::remove_if(op.begin(), op.end(), ::isspace), op.end());
                    if (std::holds_alternative<int>(actual)) {
                        int val = std::get<int>(actual);
                        int cmp = std::stoi(valueStr);
                        if (op == "==") show = val == cmp;
                        else if (op == "!=") show = val != cmp;
                        else if (op == ">")  show = val > cmp;
                        else if (op == "<")  show = val < cmp;
                        else if (op == ">=") show = val >= cmp;
                        else if (op == "<=") show = val <= cmp;
                        else {
                            std::cerr << " Unsupported operator: " << op << "\n";
                        }
                    } else if (std::holds_alternative<float>(actual)) {
                        float val = std::get<float>(actual);
                        float cmp = std::stof(valueStr);
                        if (op == "==") show = val == cmp;
                        else if (op == "!=") show = val != cmp;
                        else if (op == ">")  show = val > cmp;
                        else if (op == "<")  show = val < cmp;
                        else if (op == ">=") show = val >= cmp;
                        else if (op == "<=") show = val <= cmp;
                        else {
                            std::cerr << " Unsupported operator: " << op << "\n";
                        }
                    } else if (std::holds_alternative<std::string>(actual)) {
                        std::string cmp = valueStr;
                        if (cmp.front() == '"' && cmp.back() == '"') {
                            cmp = cmp.substr(1, cmp.size() - 2);
                        }
                        show = std::get<std::string>(actual) == cmp;
                    } else if (std::holds_alternative<bool>(actual)) {
                        show = std::get<bool>(actual) == (valueStr == "true");
                    }
                }

                if (show) {
                    for (int index : selectedIndex) {
                        std::visit([](const auto& val) {
                            fmt::print("{:<15}", val);
                        }, row.values[index]);
                    }
                    fmt::print("\n");
                }
            }

            break;
        }
        // ========== ALTER TABLE Students ADD Gender BOOL ==========
        case CommandType::ALTER_TABLE: {
            std::string cleanInput = input;
            if (!cleanInput.empty() && cleanInput.back() == ';') cleanInput.pop_back();
            std::replace(cleanInput.begin(), cleanInput.end(), '_', ' ');

            std::istringstream stream(cleanInput);
            std::string alter, tableKeyword, tableName, addKeyword, columnName, typeStr;
            stream >> alter >> tableKeyword >> tableName >> addKeyword >> columnName >> typeStr;

            if (alter != "ALTER" && alter != "alter") return;
            if (tableKeyword != "TABLE" && tableKeyword != "table") return;

            Table* t = db.getTable(tableName);
            if (!t) {
                std::cerr << " Table not found: " << tableName << "\n";
                return;
            }
            // convert entire string to uppercase.
            std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), ::toupper);
            // like if user inputs int , inT , InT we will handle it well.

            DataType type;
            if (typeStr == "INT") type = DataType::INT;
            else if (typeStr == "FLOAT") type = DataType::FLOAT;
            else if (typeStr == "STRING") type = DataType::STRING;
            else if (typeStr == "BOOL" || typeStr == "BOOLEAN") type = DataType::BOOL;
            else {
                std::cerr << " Invalid column type: " << typeStr << "\n";
                return;
            }

            t->addColumn(columnName, type);
            fmt::println(" Column '{}' added to table '{}'.", columnName, tableName);
            break;
        }

        //  UPDATE Students SET Gender = true WHERE Name == "Selim"
        case CommandType::UPDATE: {
            // Copy the input and remove semicolon at the end if present
            std::string cleanInput = input;
            if (!cleanInput.empty() && cleanInput.back() == ';') cleanInput.pop_back();
            // find the position of SET and WHERE keywords.
            size_t setPos = cleanInput.find("SET"); //we declare with the size_t is the type C++ uses for sizes and indexes. It’s actually an unsigned integer, which means it can’t be negative.
            size_t wherePos = cleanInput.find("WHERE");

            // Ensure that UPDATE command has both SET and WHERE clauses.
            if (setPos == std::string::npos || wherePos == std::string::npos) {
                std::cerr << " UPDATE syntax error: must include SET and WHERE.\n"; //std::cerr is used to print error messages to the console.
                return;
            }
            // Extract the table name and remove spaces.
            std::string tableName = cleanInput.substr(7, setPos - 7);
            tableName.erase(std::remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

            // Extract the SET and WHERE clauses as seperate Strings.
            std::string setClause = cleanInput.substr(setPos + 4, wherePos - setPos - 4);
            std::string whereClause = cleanInput.substr(wherePos + 6);

            // Parse SET clause
            std::string targetCol, newValueStr;
            {
                std::istringstream setStream(setClause);
                std::getline(setStream, targetCol, '=');
                std::getline(setStream, newValueStr);
                targetCol.erase(std::remove_if(targetCol.begin(), targetCol.end(), ::isspace), targetCol.end());
                newValueStr.erase(std::remove_if(newValueStr.begin(), newValueStr.end(), ::isspace), newValueStr.end());
            }

            // Parse WHERE clause: condCol op condValueStr
            std::string condCol, op, condValueStr;
            {
                std::istringstream condStream(whereClause);
                condStream >> condCol >> op >> condValueStr;
                condCol.erase(std::remove_if(condCol.begin(), condCol.end(), ::isspace), condCol.end());
                condValueStr.erase(std::remove_if(condValueStr.begin(), condValueStr.end(), ::isspace), condValueStr.end());
            }

            Table* table = db.getTable(tableName);
            if (!table) {
                std::cerr << " Table not found: " << tableName << "\n";
                return;
            }

            int targetIndex = -1, condIndex = -1;
            for (size_t i = 0; i < table->columns.size(); ++i) {
                if (table->columns[i].name == targetCol) targetIndex = i;
                if (table->columns[i].name == condCol) condIndex = i;
            }

            if (targetIndex == -1 || condIndex == -1) {
                std::cerr << " Column not found.\n";
                return;
            }

            // Prepare value for SET
            Value newValue;
            const DataType& targetType = table->columns[targetIndex].type;
            try {
                if (targetType == DataType::INT) newValue = std::stoi(newValueStr);
                else if (targetType == DataType::FLOAT) newValue = std::stof(newValueStr);
                else if (targetType == DataType::STRING) {
                    if (newValueStr.front() == '"' && newValueStr.back() == '"')
                        newValue = newValueStr.substr(1, newValueStr.size() - 2);
                    else newValue = newValueStr;
                } else if (targetType == DataType::BOOL) newValue = (newValueStr == "true");
            } catch (...) {
                std::cerr << " Type mismatch in SET value.\n";
                return;
            }

            // Update matching rows
            int updatedCount = 0; // for display how many rows are updated.
            for (auto& row : table->rows) {
                const Value& actual = row.values[condIndex];
                bool match = false;

                // Match the actual row value with the WHERE condition
                if (std::holds_alternative<int>(actual)) match = std::get<int>(actual) == std::stoi(condValueStr); // std::stoi -> string to an integer.
                else if (std::holds_alternative<float>(actual)) match = std::get<float>(actual) == std::stof(condValueStr); // std::stof -> string to an float.
                else if (std::holds_alternative<std::string>(actual)) {
                    std::string cmp = condValueStr;
                    if (cmp.front() == '"' && cmp.back() == '"')
                        cmp = cmp.substr(1, cmp.size() - 2);
                    match = std::get<std::string>(actual) == cmp;
                } else if (std::holds_alternative<bool>(actual)) {
                    match = std::get<bool>(actual) == (condValueStr == "true");
                }

                if (match) {
                    row.values[targetIndex] = newValue;
                    updatedCount++;
                }
            }

            fmt::println(" {} row(s) updated in '{}'.", updatedCount, tableName);
            break;
        }

        case CommandType::SAVE_TO: {
            size_t quoteStart = input.find('"');
            size_t quoteEnd = input.rfind('"');
            if (quoteStart == std::string::npos || quoteEnd == std::string::npos || quoteEnd <= quoteStart) {
                std::cerr << " SAVE TO syntax error. Use: SAVE TO \"filename.db\";\n";
                return;
            }
                    // Extract the text between quotation marks (e.g., "file.txt" → file.txt)
            std::string path = input.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            try {
                db.saveToFile(path);
                fmt::println(" Database saved to '{}'.", path);
            } catch (const std::exception& e) {
                std::cerr << " Save error: " << e.what() << "\n";
            }
            break;
        }
        case CommandType::LOAD_FROM: {
            size_t quoteStart = input.find('"');
            size_t quoteEnd = input.rfind('"');
            if (quoteStart == std::string::npos || quoteEnd == std::string::npos || quoteEnd <= quoteStart) {
                std::cerr << " LOAD FROM syntax error. Use: LOAD FROM \"filename.db\";\n";
                return;
            }

            std::string path = input.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            try {
                db.loadFromFile(path);
                fmt::println(" Database loaded from '{}'.", path);
            } catch (const std::exception& e) {
                std::cerr << " Load error: " << e.what() << "\n";
            }
            break;
        }


        // ========== UNKNOWN ==========
        case CommandType::UNKNOWN: {
            fmt::println(" Unknown command.\n");
            break;
        }
    }
}

