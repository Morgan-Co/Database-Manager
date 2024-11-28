#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstdlib>

void print_info() {
	std::cout << "+ - add new user\n";
	std::cout << "- - delete user\n";
	std::cout << "g - get user\n";
	std::cout << "ga - get all data\n";
	std::cout << "x - exit from table\n";
	std::cout << "del - delete database\n";
	std::cout << std::endl;
}

void addEntry(sqlite3 *db, sqlite3_stmt *stmt, std::string &tableName) {
	std::string name;
	int age;

	std::cout << "Enter a name: ";
	std::cin >> name;
	std::cout << "Enter an age: ";
	std::cin >> age;

	std::string sqlInsert = "INSERT INTO " + tableName + " (name, age) VALUES (?, ?);";

	if (sqlite3_prepare_v2(db, sqlInsert.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
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
	}

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

//int close
int getTables(sqlite3 *db, sqlite3_stmt *stmt) {
	const char* sqlTables = "SELECT name FROM sqlite_master WHERE type = 'table' AND name NOT LIKE 'sqlite_%';";

	if (sqlite3_prepare_v2(db, sqlTables, -1, &stmt, nullptr) != SQLITE_OK) {
		std::cerr << "SQL query preparation error: " << sqlite3_errmsg(db) << std::endl;
		return -1;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		const unsigned char* tableName = sqlite3_column_text(stmt, 0);
		std::cout << "- " << tableName << std::endl;
	}
}

int main() {
	std::cout << "--------------------------------------------------------" << std::endl;
	std::cout << "DATABASE MANAGER" << std::endl;
	std::cout << "--------------------------------------------------------" << std::endl;
	while (true) {
		std::string action;
		std::cout << "Enter an action (*-create new database, o - open database): ";
		std::cin >> action;

		sqlite3* db;
		sqlite3_stmt* stmt = nullptr;
		int exit;

		if (action == "*")
		{
			std::string db_name;
			std::cout << "Enter database name: ";
			std::cin >> db_name;
			exit = sqlite3_open(db_name.c_str(), &db);

			if (exit != SQLITE_OK) {
				std::cout << "Error: " << sqlite3_errmsg(db) << std::endl;
				return -1;
			}

			std::cout << "Enter an action (+ - create new table, s - select a table): ";
			std::cin >> action;

			std::string currentTable;
			if (action == "s") {
				getTables(db, stmt);
				std::cout << "Choose a table: ";
				std::cin >> currentTable;
				std::cout << "Switched on " << currentTable << std::endl;
			}
			if (action == "+") {
				std::cout << "Enter a table name: ";
				std::cin >> currentTable;
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
					sqlite3_close(db);
					std::cout << "Database has been closed!" << std::endl;
					break;
				}
			}
		}
	}
	return 0;
}