#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>

static int callback(void *data, int argc, char **argv, char **azColName)
{
	int i;

	printf("argc is %d\n", argc);
	for(i = 0; i < argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
}

int main()
{
	char *sql_select, *sql_insert;
	char *zErrMsg = 0;
	const char *data = "Callback fct called\n";
	sqlite3 *db;
	int rc;

	rc = sqlite3_open("db.sqlite3", &db);
	if (rc) {
		printf("Cannot open database\n");
	}

	sql_select = "SELECT * from vprofile_client";

	sql_insert = "INSERT INTO vprofile_client(name, email, address, phone_number, \
		      card_id, contract_id, contract_type, username, password) VALUES\
		      ('Cristina Georgiana', 'cristina.opriceana@yahoo.com', 'Str. X, Nr 29', \
		       +123456789, 'ADASFEDSFDDFF', 'CONTR_3', 'CONTR', 'cristina.georgiana', 'test123');";

	if (db == NULL) {
		printf("DB IS NULL\n");
	}
	//rc = sqlite3_exec(db, sql, callback, (void *) data, &zErrMsg);
	rc = sqlite3_exec(db, sql_insert, callback, (void *) data, &zErrMsg);

	if (rc) {
		printf("Cannot execute query %d %s\n", rc, zErrMsg);
	}
	sqlite3_close(db);	
	return 0;
}
