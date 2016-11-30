#include <stdio.h>
#include <mysql/mysql.h>
#include <string.h>
#include <openssl/md5.h>

#define authenticateSql = "SELECT user_id FROM users WHERE user_id = "
#define getNotesSql = "SELECT notes FROM userNotes WHERE user_id = "


void finish_with_error(MYSQL *con)
{
	fprintf(stderr, "%s\n", mysql_error(con));
	mysql_close(con);
	//exit(1);        
}

int authenticateUser(char* password, char* user, MYSQL *con) 
{
	char buf [BUFSIZ] = "SELECT user_id FROM users WHERE user_id = ";
	strcat(buf,user);
	strcat(buf, " AND password = ");
	strcat(buf, password);

	if (mysql_query(con, buf)) 
	{
		finish_with_error(con);
	}

	MYSQL_RES *result = mysql_store_result(con);

	if(mysql_num_fields(result) > 0) 
	{
		return 1;
	}

	return 0;


}

char* getUserEmail(char* user, MYSQL *con)
{
	char buf[BUFSIZ] = "SELECT user_email FROM users WHERE user_id = ";
	char *email;
	strcat(buf, user);

	if (mysql_query(con, buf)) 
	{
		finish_with_error(con);
	}

	MYSQL_RES *result = mysql_store_result(con);

	if(mysql_num_fields(result) > 0) 
	{
		MYSQL_ROW row = mysql_fetch_row(result);
		email = row[0];
	}

	return email;
}

char* getMD5hash(char* string) 
{
	unsigned char* result = malloc(MD5_DIGEST_LENGTH);
	MD5(string, strlen(string), result);

	return result;
}

char[][][] getUserNotes(char* user, MYSQL *con)
{
	char buf[BUFSIZ] = "SELECT notes_id, title, body FROM user_notes WHERE fk_user_id = ";
	strcat(buf, user);

	if (mysql_query(con, buf)) 
	{
		finish_with_error(con);
	}

	MYSQL_RES *result = mysql_store_result(con);
	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;
	char notesArray[10][10][10];
	int i = 0;
	while ((row = mysql_fetch_row(result))) 
	{ 
		
		char* id, title, body;
		char *token;

		id = row[0];
		title = row[1];
		body = row[2];

		notesArray[i][i][i] = {id, title, body}; 
		i++;
		//printf("%s ", row[i] ? row[i] : "NULL"); 
		printf("\n"); 
	}

	return notesArray;	
}

int deleteNote(char* id, MYSQL* con)
{
	char buf[BUFSIZ] = "DELETE * FROM user_notes WHERE fk_user_id = ";
	strcat(buf, user);

	if (mysql_query(con, buf)) 
	{
		finish_with_error(con);
		return 0;
	}

	return 1;
}


int addUser(char* username, char* email, char* password, char* FirstName, char* LastName, MYSQL *con)
{
	char buf[BUFSIZ] = "INSERT INTO users (user_id, password, user_email, user,FirstName, user_Lastname) VALUES ( ";
	
	char* passwordHash = getMD5hash(password);
	strcat(buf, username);
	strcat(buf, ", ");
	strcat(buf, passwordHash);
	strcat(buf, ", ");
	strcat(buf, email);
	strcat(buf, ", ");
	strcat(buf, FirstName);
	strcat(buf, ", ");
	strcat(buf, LastName);
	strcat(buf, " )");

	if (mysql_query(con, buf)) 
	{
		finish_with_error(con);
		return 0;
	}

	return 1
	
}


int main(int argc, char **argv)
{      
	MYSQL *con = mysql_init(NULL);

	if (con == NULL)
	{
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  

	if (mysql_real_connect(con, "localhost", "root", "sa", 
		"pithentication", 0, NULL, 0) == NULL) 
	{
		finish_with_error(con);
	}    

	if (mysql_query(con, "SELECT * FROM users")) 
	{
		finish_with_error(con);
	}

	MYSQL_RES *result = mysql_store_result(con);

	if (result == NULL) 
	{
		finish_with_error(con);
	}

	int num_fields = mysql_num_fields(result);

	MYSQL_ROW row;

	while ((row = mysql_fetch_row(result))) 
	{ 
		for(int i = 0; i < num_fields; i++) 
		{ 
			printf("%s ", row[i]); 
		} 
		printf("\n"); 
	}

	mysql_free_result(result);
	mysql_close(con);

	exit(0);
}
