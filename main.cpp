//
// Created by Selim Dalçiçek on 1.05.2025.
//

#include <iostream>
#include "database.cpp"
#include "CommandParser.cpp"

int main() {
    Database db;
    std::string input;

    fmt::print("Welcome to CQL. Type command below:\n");

    while (true) {
        fmt::print("\n> ");
        std::getline(std::cin, input);

        if (input == ".exit") {
            std::string choice;
            fmt::print("Do you want to save before exiting? (yes/no): ");
            std::getline(std::cin, choice);

            if (choice == "yes" || choice == "y") {
                std::string path;
                fmt::print("Enter filename (e.g. save.txt): ");
                std::getline(std::cin, path);

                try {
                    db.saveToFile(path);
                    fmt::print(" Saved to '{}'\n", path);
                } catch (const std::exception& e) {
                    fmt::print(" Save failed: {}\n", e.what());
                }
            }

            fmt::print("Goodbye!\n");
            break;
        }

        CommandParser::executeCommand(input, db);
    }

    return 0;
}


// DDL - Data Definition Language
// Used to define and modify the structure of the database

// Create the Cars table
// CREATE_TABLE Cars(ID INT,Brand STRING , Horsepower INT , Price FLOAT , Type STRING , Electric BOOL);
// Add a new column for Color
// ALTER TABLE Cars ADD Color STRING;

// Drop the table if needed.
// DROP_TABLE Cars;


// DML - Data Manipulation Language
// Used to insert, update, and modify the data inside the tables

// Insert car records
// INSERT INTO Cars VALUES (1,"Tesla" , 600 , 79999.99 , "Sedan" , true , "Blue");
// INSERT INTO Cars VALUES (2 , "BMW" , 340 , 559999.99 , "SUV" , false , "Black");
// INSERT INTO Cars VALUES (3,"Toyota" , 200 , 24999.99 , "Hatchback" , false  , "White");
// INSERT INTO Cars VALUES (4, "Mercedes-Benz" , 540 , 859999.99 , "Sport Coupe" , false , "Black");

// Update a record
// UPDATE Cars SET Electric = true WHERE Brand == "BMW";
// UPDATE Cars SET Color = "Red" WHERE Brand == "Toyota";


// DQL - Data Query Language
// Used to query and view data

// View everything
// SELECT * FROM Cars;

// View Specific columns
// SELECT Brand , Price FROM Cars;

// Query with WHERE
// SELECT * FROM Cars WHERE Horsepower > 300;
// SELECT Brand,Price FROM Cars WHERE Electric == true;
// SELECT * FROM Cars WHERE Type == "SUV";

// SELECT * FROM Cars WHERE Color == "Red";

// Save to a file
// SAVE TO "cars.txt";

// Load from file
// LOAD_FROM "cars.txt";



// TESTING EDGE CASES.
// -- Invalid column
// SELECT Engine FROM Cars;
//
// -- Missing WHERE
// UPDATE Cars SET Price = 0;
//
// -- Too few values
// INSERT INTO Cars VALUES ("Ford", 300, 35000.0);
