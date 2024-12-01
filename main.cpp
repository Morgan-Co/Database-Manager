#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <unordered_map>

struct Field {
	std::string name;
	std::string type;
	bool is_primary;
	Field(std::string name, std::string type, bool is_primary)
		: name(name), type(type), is_primary(is_primary) {}
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

void deleteEntry(sqlite3 *db, sqlite3_stmt *stmt, std::string& tableName) {
	int id;
	std::cout << "Enter an id: ";
	std::cin >> id;

	std::string query = "DELETE FROM " + tableName + " WHERE id = ?;";

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

int getAll(sqlite3 *db, sqlite3_stmt* stmt, std::string& tableName) {
	std::string sqlSelect = "SELECT * FROM " + tableName + ";";
	if (sqlite3_prepare_v2(db, sqlSelect.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int id = sqlite3_column_int(stmt, 0);
		const unsigned char* name = sqlite3_column_text(stmt, 1);
		int age = sqlite3_column_int(stmt, 2);

		std::cout << "ID: " << id << "\tName: " << name << "\tAge: " << age << std::endl;
	}
	return 0;
}

int getOnce(sqlite3 *db, sqlite3_stmt *stmt, std::string& tableName) {
	int userId;
	std::cout << "Enter user id: ";
	std::cin >> userId;

	std::string sqlSelectUser = "SELECT id, name, age FROM " + tableName + " WHERE id = ?;";

	if (sqlite3_prepare_v2(db, sqlSelectUser.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}

	sqlite3_bind_int(stmt, 1, userId);

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		int id = sqlite3_column_int(stmt, 0);
		const unsigned char* name = sqlite3_column_text(stmt, 1);
		int age = sqlite3_column_int(stmt, 2);

		std::cout << "The user has been found: " << std::endl << std::endl;
		std::cout << "ID: " << id << "\tName: " << name << "\tAge: " << age << std::endl << std::endl;
	}
	else {
		std::cout << "The user with ID " << userId << " was not found." << std::endl << std::endl;
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
		std::cout << "- " << tableName << std::endl;
	}

	return tableNames;
}

std::unordered_map<std::string, std::string> getTableFileds(sqlite3 *db, sqlite3_stmt * stmt, const std::string& tableName) {
	std::string query = "PRAGMA table_info(" + tableName + ");";
	std::unordered_map<std::string, std::string> fields;

	if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return fields;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const unsigned char* fieldName = sqlite3_column_text(stmt, 1);
		if (fieldName && std::string(reinterpret_cast<const char*>(fieldName)) != "id") {
			fields[std::string(reinterpret_cast<const char*>(fieldName))];
		}
		std::cout << "- " << fieldName << std::endl;
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
	int exit;
	std::string query =
		"CREATE TABLE IF NOT EXISTS " + tableName + " (id INTEGER PRIMARY KEY AUTOINCREMENT, ";

	for (auto it = tableFields.begin(); it != tableFields.end(); it++) {
		query += it->first + " " + it->second + " NOT NULL" + (std::next(it) != tableFields.end() ? ", " : "");
	
	}
	query += ");";
	std::cout << query << std::endl;

	exit = sqlite3_exec(db, query.c_str(), nullptr, nullptr, nullptr);
	if (exit != SQLITE_OK) {
		std::cout << "Error: " << sqlite3_errmsg(db) << std::endl;
		return false;
	}
	return true;
}

void addEntry(sqlite3* db, sqlite3_stmt* stmt, std::string& tableName) {
	std::string name;
	int age;
	std::unordered_map<std::string, std::string> fields = getTableFileds(db, stmt, tableName);
	//std::string query = "INSERT INTO " + tableName + " (name, age) VALUES (?, ?);";
	std::string query = "INSERT INTO " + tableName + " (";

	for (auto it = fields.begin(); it != fields.end(); it++) {
		std::cout << it->first << ": ";
		std::cin >> it->second;
		query += it->first + (std::next(it) != fields.end() ? ", " : "");
	}
	query += ") VALUES (";
	for (auto it = fields.begin(); it != fields.end(); it++) {
		query += (std::next(it) != fields.end() ? "?, " : "?");
	}
	query += ");";
	std::cout << query << std::endl;


	/*if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return;
	}


	sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, age);

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		std::cerr << "Request execution error: " << sqlite3_errmsg(db) << std::endl;
	}
	else {
		std::cout << "The data has been successfully added!" << std::endl;
	}*/

}

int main() {
	std::cout << "--------------------------------------------------------" << std::endl;
	std::cout << "DATABASE MANAGER" << std::endl;
	std::cout << "--------------------------------------------------------" << std::endl;
	while (true) {
		std::string action;
		std::cout << "Enter an action (*-create new database, o - open database): ";
		std::cin >> action;

		sqlite3* db = nullptr;
		sqlite3_stmt* stmt = nullptr;
		int exit;

		std::string db_name;
		std::cout << "Enter database name: ";
		std::cin >> db_name;
		if (action == "o")
		{
			exit = sqlite3_open(db_name.c_str(), &db);

			const char* sqlCreateTable =
				"CREATE TABLE IF NOT EXISTS users ("
				"id INTEGER PRIMARY KEY AUTOINCREMENT, "
				"name TEXT NOT NULL, "
				"age INTEGER NOT NULL);";

			// Выполняем запрос
			exit = sqlite3_exec(db, sqlCreateTable, nullptr, nullptr, nullptr);
			if (exit != SQLITE_OK) {
				std::cout << "Error: " << sqlite3_errmsg(db) << std::endl;
				return -1;
			}
			std::cout << "Enter an action (+ - create new table, s - select a table): ";
		}
		if (action == "*")
		{
			std::ofstream file(db_name);
			if (file.is_open()) {
				std::cout << "File " << db_name << " has been successfuly created." << std::endl;
				file.close();
			}
			else {
				std::cerr << "Failed to create a file " << db_name << std::endl;
			}
			std::cout << "Enter an action (+ - create new table): ";
		}

		std::cin >> action;

		std::string currentTable;
		bool exit_from_table = false;
		while (true) {

			if (action == "s") {
				getTables(db, stmt);
				std::cout << "Choose a table: ";
				std::cin >> currentTable;
				std::cout << "Switched on " << currentTable << std::endl;
			}
			else {
				//std::cout << "You must create new table!" << std::endl;
			}
			if (action == "+") {
				std::cout << "Enter a table name: ";
				std::cin >> currentTable;

				int fields_number;
				std::cout << "Enter the number of fields: ";
				std::cin >> fields_number;
				std::unordered_map<std::string, std::string> fields;
				for (int i = 0; i < fields_number; i++) {
					system("cls");
					std::string name;
					std::string type;

					std::cout << "Field " << (i + 1) << " -------------------" << std::endl;
					std::cout << "name: ";
					std::cin >> name;
					std::cout << "type: ";
					std::cin >> type;

					fields[name] = type;
				}
				system("cls");
				bool result = createTable(db, stmt, currentTable, fields);
				if (result) {
					std::cout << "Table " << currentTable << " has been successfully created!" << std::endl;
				}
			}

			while (true) {
				std::cout << "Enter an action (? - print info): ";
				std::cin >> action;
				system("cls");
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