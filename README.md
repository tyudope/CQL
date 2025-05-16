# SimpleDB – C++ In-Memory Database Engine

This project implements a simple SQL-like database system in C++20. It supports creating tables, inserting and querying data, updating values, and saving/loading the database from file. The syntax mimics SQL but is parsed and executed manually within the program.

---

## 📚 Features

### 🏗️ Data Definition Language (DDL)
- `CREATE_TABLE` – create tables with typed columns (`INT`, `FLOAT`, `STRING`, `BOOL`)
- `DROP_TABLE` – delete an existing table
- `ALTER TABLE ... ADD COLUMN` – add new columns to an existing table
- ✅ Primary key is enforced on the first column (e.g. `ID`, `Brand`, etc.)

### ✍️ Data Manipulation Language (DML)
- `INSERT INTO` – add new rows with type enforcement and primary key checking
- `UPDATE ... SET ... WHERE ...` – update values in rows conditionally

### 🔎 Data Query Language (DQL)
- `SELECT * FROM table` – show all columns
- `SELECT col1, col2 FROM table` – show specific columns
- `WHERE` support with all types and operators: `==`, `!=`, `>`, `<`, `>=`, `<=`

---

## 💾 File Persistence

- `SAVE TO "filename"` – saves the database state to a human-readable file
- `LOAD FROM "filename"` – restores data and tables from a file
- `.exit` – cleanly exits and asks if the user wants to save

---

## 🛠️ Technologies

- **Language:** C++20
- **Library:** [fmt](https://github.com/fmtlib/fmt) (included using `FetchContent`)
- **Build System:** CMake

---

## 🧪 Sample Commands

```sql
CREATE_TABLE Cars(Brand STRING, Horsepower INT, Price FLOAT, Electric BOOL);
INSERT INTO Cars VALUES ("Tesla", 670, 79999.99, true);
SELECT Brand, Price FROM Cars WHERE Electric == true;
UPDATE Cars SET Price = 74999.99 WHERE Brand == "Tesla";
SAVE TO "cars.txt";
LOAD FROM "cars.txt";
```

---

## 📦 Submission Notes

- ✅ Only `.cpp`, `.hpp`, and `CMakeLists.txt` included in the `.zip`
- ✅ All dependencies handled with `FetchContent()` (no external folders)
- ✅ Demo video file provided separately (max 3 minutes, no code or audio)
- ❌ No IDE folders, build artifacts, or Git metadata

---

## 🎓 Author

**Selim Dalçiçek**  
Polsko-Japońska Akademia Technik Komputerowych  
Group: 36  
Student ID: s33894
