#include <sqlite3.h>
#include <stdlib.h>
#include <stdio.h>

static int callback(void *data, int argc, char **argv, char **azColName)
{
	int i;

	printf("argc is %d\n", argc);
	for(i = 0; i<argc; i++){
	
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}

}

int main(){
	
	sqlite3 *db;
	int rc;
	char *sql;
	char *zErrMsg = 0;
	const char *data = "Callback fct called\n";

	rc = sqlite3_open("db.sqlite3", &db);
	if (rc) {
		printf("Cannot open database\n");
	}

	sql = "SELECT * from vprofile_client;";

	//sql = "SELECT name FROM db.sqlite3 WHERE type='table'";
	//sql = "SELECT name FROM sqlite_temp_master WHERE type='table'";
	
	if (db == NULL) {
		printf("DB IS NULL\n");
	}
	rc = sqlite3_exec(db, sql, callback, (void *) data, &zErrMsg);
	if (rc) {
		printf("Cannot execute query %d %s\n", rc, zErrMsg);
	}
	sqlite3_close(db);	
	return 0;
}
