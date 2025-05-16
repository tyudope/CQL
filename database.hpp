//
// Created by Selim Dalçiçek on 1.05.2025.
//

#pragma once

#include <string>
#include <vector>
#include <variant>  // for storing multiple possible types in one variable.

// representing supported datatypes in the database.
enum class DataType{

  INT,
  FLOAT,
  STRING,
  BOOL
  };

// Define a type that can store any value of the supported data types.
// std::variant ensures type safety and avoids void pointers.
using Value = std::variant<int , float , std::string, bool>;

// A single column in a table, defined by a name and data type.
struct Column{
  std::string name;
  DataType type;
  };

// A row in a table -- contains a list of values (one per column)
// Each value is stored using std::variant, typed safely.
struct Row{
  std::vector<Value> values; // row data, each entry corresponds to a column.
  };


// A table structure, containing:
// - Name of the table
// - List of columns defining schema
// - List of rows storing actual data.
struct Table{
  std::string name; // table name ( e.g students.)
  std::vector<Column> columns; // Schema : (list of columns)
  std::vector<Row> rows;       // Actual data : list of rows
  std::string primaryKeyColumn = "ID";
  void addColumn(const std::string& columnName , DataType type); // add a new column to the table
  void addRow(const std::vector<Value>& values);    // add a new row to the table, with type-checked values.
  void showTable() const;
  };




// A database is collection of tables.
struct Database{
  std::vector<Table> tables; // List of all tables in the database

  void createTable(const std::string& tableName , const std::vector<Column>& columns);// Create a new table with the given name and columns
  void dropTable(const std::string& tableName); // Remove a table by name
  void saveToFile(const std::string& path) const;
  void loadFromFile(const std::string &path);
  Table* getTable(const std::string& tableName); // Get a pointer to a table by name, or nullptr if not found.

  };

// Utility function: Convert a DataType enum to a readable string
// Example: DataType::INT -> "INT"
std::string dataTypeToString(DataType type);





