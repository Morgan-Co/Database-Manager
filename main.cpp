#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <unordered_map>

struct Field {
private:
	std::string name;
	std::string type;
	std::string value;
public:
	Field(std::string name, std::string type)
		: name(name), type(type){}

	void setValue(const std::string& value) {
		this->value = value;
	}

	std::string getName() const {
		return name;
	}
	std::string getType() const {
		return type;
	}
	std::string getValue() const {
		return value;
	}
};

void print_info() {
	std::cout << "+ - add new user\n";
	std::cout << "- - delete user\n";
	std::cout << "g - get user\n";
	std::cout << "ga - get all data\n";
	std::cout << "x - close table\n";
	std::cout << "del - delete table\n";
	std::cout << std::endl;
}

void deleteEntry(sqlite3 *db, sqlite3_stmt *stmt, const std::string& tableName) {
	int id;
	std::cout << "Enter an id: ";
	std::cin >> id;

	const std::string query = "DELETE FROM " + tableName + " WHERE id = ?;";

	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK) {
		std::cerr << "Value binding error: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		std::cerr << "Request execution error: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	std::cout << "Entry with ID " << id << " has been successfully deleted from table \"" << tableName << "\"." << std::endl;
}

int getAll(sqlite3 *db, sqlite3_stmt* stmt, const std::string& tableName) {
	const std::string sqlSelect = "SELECT * FROM " + tableName + ";";
	if (sqlite3_prepare_v2(db, sqlSelect.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const int id = sqlite3_column_int(stmt, 0);
		const unsigned char* name = sqlite3_column_text(stmt, 1);
		const int age = sqlite3_column_int(stmt, 2);

		std::cout << "ID: " << id << "\tName: " << name << "\tAge: " << age << std::endl;
	}
	return 0;
}

int getOnce(sqlite3 *db, sqlite3_stmt *stmt, const std::string& tableName) {
	int userId;
	std::cout << "Enter user id: ";
	std::cin >> userId;

	const std::string sqlSelectUser = "SELECT * FROM " + tableName + " WHERE id = ?;";

	if (sqlite3_prepare_v2(db, sqlSelectUser.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}

	sqlite3_bind_int(stmt, 1, userId);


	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		const int columnCount = sqlite3_column_count(stmt);
		for (int i = 0; i < columnCount; i++)
		{
			const char* columnName = sqlite3_column_name(stmt, i);
			const unsigned char* columnValue = sqlite3_column_text(stmt, i);
			std::cout << columnName << ": " << (columnValue ? reinterpret_cast<const char*>(columnValue) : "NULL") << std::endl;
		}
	}
	return 0;
}

std::vector<std::string> getTables(sqlite3 *db, sqlite3_stmt *stmt) {
	const char* sqlTables = "SELECT name FROM sqlite_master WHERE type = 'table' AND name NOT LIKE 'sqlite_%';";
	std::vector<std::string> tableNames;

	if (sqlite3_prepare_v2(db, sqlTables, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return tableNames;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const unsigned char* tableName = sqlite3_column_text(stmt, 0);
		tableNames.push_back(std::string(reinterpret_cast<const char*>(tableName)));
	}

	return tableNames;
}

std::vector<Field> getTableFields(sqlite3 *db, sqlite3_stmt * stmt, const std::string& tableName) {
	std::string query = "PRAGMA table_info(" + tableName + ");";
	std::vector<Field> fields;

	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return fields;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		std::string fieldName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
		std::string fieldType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
		Field field(fieldName, fieldType);
		if (fieldName != "id") {
			fields.push_back(field);
		}
	}
	return fields;
}

void deleteTable(sqlite3 *db, sqlite3_stmt *stmt, const std::string& tableName) {
	std::string query = "DROP TABLE IF EXISTS " + tableName + ";";

	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		std::cerr << "Request execution error: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	std::cout << "Table \n" << tableName << "\" has been successfully deleted!" << std::endl;

}

bool createTable(sqlite3* db, sqlite3_stmt* stmt, const std::string& tableName, const std::unordered_map<std::string, std::string>& tableFields) {
	std::string query =
		"CREATE TABLE IF NOT EXISTS " + tableName + " (id INTEGER PRIMARY KEY AUTOINCREMENT, ";

	for (auto it = tableFields.begin(); it != tableFields.end(); ++it) {
		query += it->first + " " + it->second + " NOT NULL" + (std::next(it) != tableFields.end() ? ", " : "");

	}
	query += ");";
	std::cout << query << std::endl;

	int exit = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
	if (exit != SQLITE_OK) {
		std::cout << "Error: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	return true;
}

void addEntry(sqlite3* db, sqlite3_stmt* stmt, const std::string& tableName) {
	std::string name;
	int age;
	std::vector<Field> fields = getTableFields(db, stmt, tableName);
	std::string query = "INSERT INTO " + tableName + " (";

	for (auto i = 0; i < fields.size(); i++) {
		std::cout << fields[i].getName() << ": ";
		std::string value;
		std::cin >> value;
		fields[i].setValue(value);
		query += fields[i].getName() + (i != fields.size() - 1 ? ", " : "");
	}
	query += ") VALUES (";
	for (int i = 0; i < fields.size(); i++) {
		query += (i != fields.size() - 1 ? "?, " : "?");
	}
	query += ");";
	std::cout << query << std::endl;


	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return;
	}

	for (auto& field : fields) {
		if (field.getType() == "TEXT") {
			sqlite3_bind_text(stmt, 1, field.getValue().c_str(), -1, SQLITE_TRANSIENT);
		} else if (field.getType() == "INTEGER") {
			sqlite3_bind_int(stmt, 2, std::stoi(field.getValue()));
		}
	}
	if (sqlite3_step(stmt) != SQLITE_DONE) {
		std::cerr << "Request execution error: " << sqlite3_errmsg(db) << std::endl;
	}
	else {
		std::cout << "The data has been successfully added!" << std::endl;
	}

}

int main() {
	std::cout << "--------------------------------------------------------" << std::endl;
	std::cout << "DATABASE MANAGER" << std::endl;
	std::cout << "--------------------------------------------------------" << std::endl;
	while (true) {
		std::string action;
		std::cout << "Enter an action (* - create new database, o - open database): ";
		std::cin >> action;

		sqlite3* db = nullptr;
		sqlite3_stmt* stmt = nullptr;
		int exit;

		std::string db_name;

		if (action == "o")
		{
			std::cout << "Enter database name: ";
			std::cin >> db_name;
			std::ifstream file(db_name);
			if (!file) {
				std::cerr << "Failed to open database file: " << db_name << std::endl;
				continue;
			}
			exit = sqlite3_open(db_name.c_str(), &db);
			if (exit != SQLITE_OK) {
				std::cout << "Error: " << sqlite3_errmsg(db) << std::endl;
			}
			std::cout << "Enter an action (+ - create new table, s - select a table): ";
		}
		else if (action == "*")
		{
			std::cout << "Enter database name: ";
			std::cin >> db_name;
			std::ofstream file(db_name);
			if (file.is_open()) {
				std::cout << "File " << db_name << " has been successfully created." << std::endl;
				file.close();
			}
			else {
				std::cerr << "Failed to create a file " << db_name << std::endl;
			}
			std::cout << "Enter an action (+ - create new table): ";
		}
		else if (action == "exit") {
			break;
		}
		else {
			std::cout << "Enter correct action!" << std::endl;
			continue;
		}

		std::cin >> action;

		std::string currentTable;
		bool exit_from_table = false;
		while (action != "exit") {
			if (action == "s") {
				std::vector<std::string> tableNames = getTables(db, stmt);
				if (tableNames.empty()) {
					std::cout << "There are no tables in database! Create new table" << std::endl;
					action = "+";
					continue;
				}
				for (auto& tableName : tableNames) {
					std::cout << "- " << tableName << std::endl;
				}
				std::cout << "Choose a table: ";
				std::cin >> currentTable;
				std::cout << "Switched on " << currentTable << std::endl;
			}
			if (action == "+") {
				std::cout << "Enter a table name: ";
				std::cin >> currentTable;

				int fields_number;
				std::cout << "Enter the number of fields: ";
				std::cin >> fields_number;
				std::unordered_map<std::string, std::string> fields;
				for (int i = 0; i < fields_number; i++) {
					std::system("clear");
					std::string name;
					std::string type;

					std::cout << "Field " << (i + 1) << " -------------------" << std::endl;
					std::cout << "name: ";
					std::cin >> name;
					std::cout << "type: ";
					std::cin >> type;

					fields[name] = type;
				}
					std::system("clear");
				bool result = createTable(db, stmt, currentTable, fields);
				if (result) {
					std::cout << "Table " << currentTable << " has been successfully created!" << std::endl;
				}
			}

			while (true) {
				std::cout << "Enter an action (? - print info): ";
				std::cin >> action;
				std::system("clear");
				if (action == "?") print_info();
				if (action == "+") addEntry(db, stmt, currentTable);
				if (action == "-") deleteEntry(db, stmt, currentTable);
				if (action == "ga") getAll(db, stmt, currentTable);
				if (action == "g") getOnce(db, stmt, currentTable);
				if (action == "x") {
					action = "s";
					break;
				}
				if (action == "del") {
					char confirm;
					std::cout << "Are you sure you want to delete the table " << currentTable << "(y - yes, n - no): ";
					std::cin >> confirm;
					if (confirm == 'y') {
						deleteTable(db, stmt, currentTable);
						break;
					}
					else if(confirm == 'n') {
						break;
					}
				}
			}
		}
	}
	return 0;
}