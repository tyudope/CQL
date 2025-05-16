//
// Created by Selim Dalçiçek on 1.05.2025.
//
#include <fmt/ranges.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include "database.hpp" // use double quotes for the file.
#include<vector>
#include <stdexcept> // For std::runtime_error
#include <fstream> // for file operations
#include <string>
#include <sstream>

// Converts a DataType enum to a readable string (used for debugging/errors).
std::string dataTypeToString(DataType type) {
    switch (type) {
        case DataType::INT: return "INT";
        case DataType::FLOAT: return "FLOAT";
        case DataType::STRING: return "STRING";
        case DataType::BOOL:{
            return "BOOLEAN";
        }
        default : return "INVALID";
    }
}

//Add a new column to the table and update existing rows with default values.
void Table::addColumn(const std::string& columnName , DataType type) {
    columns.push_back({columnName, type});
    for (auto& row : rows) {
        switch (type) {
            case DataType::INT: {
                row.values.push_back(0);
                break;
            }
            case DataType::FLOAT: {
                row.values.push_back(0.0f);
                break;
            }
            case DataType::STRING: {
                row.values.push_back("");
                break;
            }
            case DataType::BOOL: {
                row.values.push_back(false);
                break;
            }
        }
    }
}

//Add a new row after checking value types


// I WOULD LIKE TO SHOW SOMETHING LIKE THAT
// |    ID      |   NAME     |
// |    1       |   Selim    |
// |    3       |   Weronika |
// |    4       |   Ewa      |
// |    5       |   Zuzanna  |
#include <iostream>  // Also useful for fallback output
#include <fmt/core.h>  // Still works with fmt for other printing

void Table::showTable() const {
    // Print column headers
    for (const auto& column : columns) {
        fmt::print("{:<12}", column.name);  // padded columns
    }
    fmt::print("\n");
    // Print rows
    for (const auto& row : rows) {
        for (const auto& value : row.values) {
            std::visit([](const auto& v) { // std::visit is the correct way to acces value isnide a std::Variant.
                fmt::print("{:<12}", v);  // print each value with padding
            }, value);
        }
        fmt::print("\n");
    }
}
void Table::addRow(const std::vector<Value>& values) {
    if (values.size() != columns.size()) {
        throw std::runtime_error("Value count does not match column count.");
    }

    // Find primary key column index
    int pkIndex = -1;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].name == primaryKeyColumn) {
            pkIndex = static_cast<int>(i);
            break;
        }
    }

    if (pkIndex == -1) {
        throw std::runtime_error("Primary key column not found.");
    }

    // Get new row's PK value
    const Value& newPK = values[pkIndex];

    // Check if any existing row has same PK value
    for (const auto& row : rows) {
        if (row.values.size() > pkIndex && row.values[pkIndex] == newPK) {
            throw std::runtime_error("Primary key violation: duplicate value in '" + primaryKeyColumn + "'");
        }
    }

    rows.push_back({values});
}



// Create a new table and add to database
void Database::createTable(const std::string& tableName , const std::vector<Column>& columns) {
    for (const auto& table : tables) {
        if (table.name == tableName) {
            throw std::runtime_error("Table already exists");
        }
    }
    Table newTable;
    newTable.name = tableName;
    newTable.columns = columns;
    tables.push_back(newTable);
}

// Drop a table by name
void Database::dropTable(const std::string& tableName) {
    for (auto it = tables.begin(); it != tables.end(); it++) {
        if (it->name == tableName) {
            tables.erase(it);// delete the table by the given name.
            return;
        }
    }
    throw std::runtime_error("Table does not exist");
}

// Get a pointer to a table by name
Table* Database::getTable(const std::string& tableName) {
    for (auto& table : tables) {
        if (table.name == tableName) {
            return &table;
        }
    }
    return nullptr; // return null pointer if the given table name does not exist.
}


// Save to a file
void Database::saveToFile(const std::string& path) const {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + path);
    }

    for (const auto& table : tables) {
        file << "TABLE " << table.name << "\n";

        // Write all columns on one line
        file << "COLUMNS:";
        for (size_t i = 0; i < table.columns.size(); ++i) {
            const auto& col = table.columns[i];
            file << " " << col.name << " " << dataTypeToString(col.type);
            if (i < table.columns.size() - 1) file << ",";
        }
        file << "\n";

        // Write rows
        for (const auto& row : table.rows) {
            file << "ROW:";
            // Loop through each value in the row and write it to the file
            // Use std::visit to handle each type in the variant safely
            for (size_t i = 0; i < row.values.size(); ++i) {
                std::visit([&file](const auto& val) {
                    if constexpr (std::is_same_v<decltype(val), std::string>)
                        file << " \"" << val << "\""; // wrap strings in quotes
                    else if constexpr (std::is_same_v<decltype(val), bool>)
                        file << " " << (val ? "true" : "false"); // convert bools to text
                    else
                        file << " " << val; // write numbers directly
                }, row.values[i]);

                if (i < row.values.size() - 1) file << ",";
            }
            file << "\n";
        }

        file << "END_TABLE\n";
    }

    file.close();
}


void Database::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + path);
    }

    tables.clear(); // Reset current database
    std::string line;
    Table currentTable;
    //read the file line-by-line and handle each table/row/column based on keywords.
    while (std::getline(file, line)) {
        if (line.starts_with("TABLE ")) {
            currentTable = Table();
            currentTable.name = line.substr(6); // get table name
        }
        else if (line.starts_with("COLUMNS:")) {
            currentTable.columns.clear();
            std::string cols = line.substr(8);
            std::istringstream ss(cols);
            std::string token;

            // Split a line by commas, extracting one token at a time (e.g., column definitions or values).
            while (std::getline(ss, token, ',')) {
                std::istringstream pairStream(token);
                std::string name, typeStr;
                pairStream >> name >> typeStr;

                DataType type;
                if (typeStr == "INT") type = DataType::INT;
                else if (typeStr == "FLOAT") type = DataType::FLOAT;
                else if (typeStr == "STRING") type = DataType::STRING;
                else if (typeStr == "BOOL" || typeStr == "BOOLEAN") type = DataType::BOOL;
                else throw std::runtime_error("Unknown column type: " + typeStr);

                currentTable.columns.push_back({name, type});
            }
        }
        else if (line.starts_with("ROW:")) {
            std::vector<Value> rowValues;
            std::string rowData = line.substr(4);
            std::istringstream valStream(rowData);
            std::string token;
            size_t colIndex = 0;

            // Split the row line by commas to get each value string.
            // Then remove any leading/trailing whitespace from the token
            while (std::getline(valStream, token, ',')) {
                token.erase(0, token.find_first_not_of(" \t")); // remove leading whitespaces.
                token.erase(token.find_last_not_of(" \t") + 1);    // remove trailing whitespaces.

                if (colIndex >= currentTable.columns.size()) {
                    throw std::runtime_error("Too many values in a row for table " + currentTable.name);
                }

                DataType type = currentTable.columns[colIndex].type;

                try {
                    if (type == DataType::INT) {
                        rowValues.push_back(std::stoi(token));
                    } else if (type == DataType::FLOAT) {
                        rowValues.push_back(std::stof(token));
                    } else if (type == DataType::STRING) {
                        if (token.front() == '"' && token.back() == '"') {
                            token = token.substr(1, token.size() - 2);
                        }
                        rowValues.push_back(token);
                    } else if (type == DataType::BOOL) {
                        if (token == "true" || token == "1") rowValues.push_back(true);
                        else if (token == "false" || token == "0") rowValues.push_back(false);
                        else throw std::runtime_error("Invalid BOOL value: " + token);
                    }
                } catch (...) {
                    throw std::runtime_error("Value conversion failed for: '" + token +
                        "' in column " + currentTable.columns[colIndex].name);
                }

                colIndex++;
            }

            if (colIndex != currentTable.columns.size()) {
                throw std::runtime_error("Row value count does not match column count in table " + currentTable.name);
            }

            currentTable.rows.push_back({rowValues});
        }
        else if (line == "END_TABLE") {
            tables.push_back(currentTable);
        }
    }

    file.close();
}