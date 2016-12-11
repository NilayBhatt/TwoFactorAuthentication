#define _XOPEN_SOURCE

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <curl/curl.h>
#include <stdio.h>
#include <mysql/mysql.h>
#include <string.h>
#include <openssl/md5.h>
#include <unistd.h>
#include <time.h>

int status = 0; //0 = logged out, 1 = logged in

#define FROM "<edmiralagic@gmail.com>"
#define authenticateSql = "SELECT user_id FROM users WHERE user_id = "
#define getNotesSql = "SELECT * FROM userNotes WHERE user_id = "

char* getEmail(char*);
int addUser(char*, char*, char*);
static char result [33];
void loginCommand(char*, char*);
void loggedInMessage();
void loggedInCommands(char *);
void addNote();


int sessionID;

static char *payload_text[13];

void finish_with_error(MYSQL *con){
	fprintf(stderr, "%s\n", mysql_error(con));
	mysql_close(con);
	exit(1);        
}

int addUser(char* username, char* password, char* email){
	MYSQL *con = mysql_init(NULL);

	if (con == NULL){
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  

	if (mysql_real_connect(con, "localhost", "root", "sa", 
		"pithentication", 0, NULL, 0) == NULL){
		finish_with_error(con);
	}  

	char buf[BUFSIZ] = "INSERT INTO users (username, password, email) VALUES ( '";
	
	//char* passwordHash = getMD5hash(password);
	strcat(buf, username);
	strcat(buf, "' , '");
	strcat(buf, password);
	strcat(buf, "' , '");
	strcat(buf, email);
	strcat(buf, "' )");
	
	if (mysql_query(con, buf)){
		finish_with_error(con);
		//mysql_free_result(result);
		mysql_close(con);
		return 0;
	}
	//mysql_free_result(result);
	mysql_close(con);
	return 1;
}

int authenticateUser(char* user, char* password) {
	MYSQL *con = mysql_init(NULL);

	if (con == NULL){
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  

	if (mysql_real_connect(con, "localhost", "root", "sa", 
		"pithentication", 0, NULL, 0) == NULL){
		finish_with_error(con);
	}   

	char buf[BUFSIZ] = "SELECT username FROM users WHERE username = '";
	strcat(buf,user);
	strcat(buf, "' AND password = '");
	strcat(buf, password);
	strcat(buf, "'");

	if (mysql_query(con, buf)){
		finish_with_error(con);
		mysql_close(con);
		return 0;
	}
	else{
		MYSQL_RES *result = mysql_store_result(con);
	
		if(mysql_num_fields(result) > 0){
			mysql_free_result(result);
			mysql_close(con);
			return 1;
		}
		mysql_free_result(result);
		mysql_close(con);
		return 0;
	}

}

char* getEmail(char* user){
	MYSQL *con = mysql_init(NULL);

	if (con == NULL){
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  

	if (mysql_real_connect(con, "localhost", "root", "sa", 
		"pithentication", 0, NULL, 0) == NULL){
		finish_with_error(con);
	}   

	char buf[BUFSIZ] = "SELECT email FROM users WHERE username = '";
	char *email;
	strcat(buf, user);
	strcat(buf, "'");
	if (mysql_query(con, buf)){
		finish_with_error(con);
	}

	MYSQL_RES *result = mysql_store_result(con);

	if(mysql_num_fields(result) > 0){
		MYSQL_ROW row = mysql_fetch_row(result);
		 email = row[0];
	}
	mysql_free_result(result);
	mysql_close(con);

	return email;
}

char* getMD5hash(char* string) 
{
	char *result = crypt(string, "1000");
	
 	return result;
 	
}

struct upload_status{
  int lines_read;
};
 
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp){
  struct upload_status *upload_ctx = (struct upload_status *)userp;
  const char *data;
 
  if((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
    return 0;
  }
 
  data = payload_text[upload_ctx->lines_read];
 
  if(data) {
    size_t len = strlen(data);
    memcpy(ptr, data, len);
    upload_ctx->lines_read++;
 
    return len;
  }
 
  return 0;
}

int emailCode(char *email){
  CURL *curl;
  CURLcode res = CURLE_OK;
  struct curl_slist *recipients = NULL;
  struct upload_status upload_ctx;
 
  upload_ctx.lines_read = 0;
 
  curl = curl_easy_init();
  if(curl) {
    /* Set username and password */ 
    curl_easy_setopt(curl, CURLOPT_USERNAME, "edmiralagic@gmail.com");
    curl_easy_setopt(curl, CURLOPT_PASSWORD, "Eadlmaigr2212");
    curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
 
	#ifdef SKIP_PEER_VERIFICATION
	    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	#endif
 
	#ifdef SKIP_HOSTNAME_VERIFICATION
	    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	#endif

    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, FROM);
 
    recipients = curl_slist_append(recipients, email);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
 
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
 
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    res = curl_easy_perform(curl);
 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    curl_slist_free_all(recipients);

    curl_easy_cleanup(curl);
  }
 
  return (int)res;
}

int isValidWelcome(char *key){
	if(strcmp(key, "login") != 0 && strcmp(key, "signup") != 0 && strcmp(key, "exit") != 0){
		return 0;
	}
	return 1;
}

char *welcomeMessage(){
	char command[4096];
	printf("\nWELCOME TO THE PITHENTICATION NOTEPAD APP\n");
	printf("\nAvailable commands..\n");
	printf("\"login\"\t\tWill prompt you for username and password for login\n");
	printf("\"signup\"\tWill prompt you for a variety of inputs to create an account\n");
	printf("\"exit\"\t\tWill exit the program\n");
	while(1){
		printf("\nWhat would you like to do: ");
		scanf("%s", command);
		if(isValidWelcome(command)){
			break;
		}
		printf("\nUh oh! Only use the commands listed above or else....");
	}
	return command;
}

char *getCode(){
	char charset[36] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char code[9];
	int i;
	int index;
	for(i = 0; i < 8; i++){
		index = (rand() % 35) + 1;
		code[i] = charset[index];
	}
	code[8] = '\0';
	char *newCode = code;
	return newCode;
}

char *getPasswordInput(){
	char password[4096];
	char c;
	int i;
	while((c=getc(stdin)) != '\n'){
		password[i] = c;
		printf("*");
		i++;	
	}
	password[i] = '\0';
	return password;
}

void adjustEmailPayload(char *email, char *code){
	//printf("EMAILAGAIN -> %s", Eemail);
	payload_text[0] = "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n";
	payload_text[1] = "To: <\"";
	payload_text[2] = email;
	payload_text[3] = "\">\r\n";
	payload_text[4] = "From: " FROM "(Pithentication Support)\r\n";
	payload_text[5] = "Subject: Pithentication Verify Code\r\n";
 	payload_text[6] =  "\r\n";
 	payload_text[7] =  "You are one step closer to activating your account!\r\n";
 	payload_text[8] = "\r\n";
 	payload_text[9] = "Your verification code is: ";
 	payload_text[10] = code;
 	payload_text[11] = "\r\n";
	payload_text[12] = NULL;
	
	//printf("EMAIL ->>> %s", payload_text[2]);
}

void signUpPrompt(){
	char username[4096];
	char email[4096];
	char verCode[9];
	char password[4096];
	char tempPass[4096];
	char temp2Pass[4096];
	printf("\nYou are one step closer to having your own Pithentication account!\n");
	while(1){
		printf("Please enter your desired username (required): ");
		scanf("%s", username);
		if(username != NULL){
			break;
		}
	}
	while(1){
		strcpy(tempPass, getpass("Please enter your desired password (required): "));
		strcpy(temp2Pass, getpass("Please re-enter your desired password (required): "));
		if(strcmp(temp2Pass, tempPass) == 0 && tempPass != NULL){
			strcpy(password, getMD5hash(tempPass));
			break;
		}
		printf("\nUh oh! The passwords did not match.. lets try again!\n");
	}
	while(1){
		printf("Please enter your email (required): ");
		scanf("%s", email);
		if(email != NULL){ //perhaps later on do isValidEmail()
			break;
		}
		printf("\nUh oh! You need to enter something for your email!");
	}
	//char *tempCode = getCode();
	//getCode method was acting funny so I added it here
	char charset[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	char code[9];
	int i;
	int index;
	for(i = 0; i < 8; i++){
		index = (rand() % 35) + 1;
		code[i] = charset[index];
	}
	code[8] = '\0';
	char *newCode = code;
	adjustEmailPayload(email, newCode);
	int emailStatus = emailCode(email);
	
	printf("\nYour account is almost ready!\nTo complete account signup please enter to verification code we sent to your email: ");
	scanf("%s", verCode);

	if(strcmp(newCode, verCode) != 0){
		printf("\nAuthentication failed.. Ending program\n");
		exit(0);
	}
	else{
		int userStatus = addUser(username, password, email);
		if(userStatus){
			printf("\nYour account has been successfully registered with Pithentication!");
			loginCommand(username, password);
		}
		else{
			printf("There was an error adding your account... Ending program\n");
			exit(0);
		}
	}
}

int getID(char *username){
	
	MYSQL *con = mysql_init(NULL);

	if (con == NULL){
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  

	if (mysql_real_connect(con, "localhost", "root", "sa", 
		"pithentication", 0, NULL, 0) == NULL){
		finish_with_error(con);
	}   

	char buf[BUFSIZ] = "SELECT user_id FROM users WHERE username = '";
	int id; 	
	strcat(buf, username);
	strcat(buf, "'");
	if (mysql_query(con, buf)){
		finish_with_error(con);
	}

	MYSQL_RES *result = mysql_store_result(con);

	if(mysql_num_fields(result) > 0){
		MYSQL_ROW row = mysql_fetch_row(result);
		 id = atoi(row[0]);
	}
	mysql_free_result(result);
	mysql_close(con);

	return id;
}

void loginCommand(char *username, char *password){
	int authStatus = authenticateUser(username, password);
	if(authStatus){
		status = 1;
		sessionID = getID(username);
		loggedInMessage();
	}
	else{
		status = 0;
	}
}

void addNote(){
	char title[4096];
	char body[4096];
	char temp[100];
	printf("\nPlease enter the title of your note: ");
	scanf("%s", title);
	printf("\nPlease enter the body of your note: ");
	scanf("%s", body);
	
	MYSQL *con = mysql_init(NULL);

	if (con == NULL){
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  

	if (mysql_real_connect(con, "localhost", "root", "sa", 
		"pithentication", 0, NULL, 0) == NULL){
		finish_with_error(con);
	}  
	
	char buf[BUFSIZ] = "INSERT INTO notes (user_id, title, body) VALUES ( ";
	sprintf(temp, "%d", sessionID);
	//char* passwordHash = getMD5hash(password);
	strcat(buf, temp);
	strcat(buf, ", '");
	strcat(buf, title);
	strcat(buf, "', '");
	strcat(buf, body);
	strcat(buf, "' )");

	if (mysql_query(con, buf)) 
	{
		finish_with_error(con);
		return 0;
	}

	return 1;
	
}

void loginPrompt(){
	char username[4096];
	printf("\nWelcome to Pithentication! Please login to your account below.\n");
	printf("Please enter your username: ");
	scanf("%s", username);
	char *tempPassword = getpass("Please enter your password: ");
	char *password = getMD5hash(tempPassword);
	//DBmethodToGetEmailBasedOnUsername
	//char *email = getEmail(username);
	//char *code = getCode();
	if(authenticateUser(username, password)){
		char charset[36] = "0123456789abcdefghijklmnopqrstuvwxyz";
		char code[9];
		int i;
		int index;
		for(i = 0; i < 8; i++){
			index = (rand() % 35) + 1;
			code[i] = charset[index];
		}
		code[8] = '\0';
		char *newCode = code;
		char verCode[9];
		char newEmail[4096];
	
		strcpy(newEmail,getEmail(username));
		adjustEmailPayload(newEmail, code);
		
		
		int emailStatus = emailCode(newEmail);
		
		
		printf("A verification code has been sent to your registered email..");
		printf("\nPlease enter the verification code: ");
		scanf("%s", verCode);
		if(strcmp(newCode, verCode) == 0){
			loginCommand(username, password);
		}
		else{
				printf("\nUh oh! You've entered the wrong verification code.. Ending program.\n");
				exit(0);
		}
	}
	else{
		printf("Username password combo does not match anything we have in our records.. Ending program.\n");
		exit(0);
	}
}

void loggedInCommands(char *key){
	if(strcmp(key, "addnote") == 0){
			addNote();
			printf("\nCongratulations!!! You created a note. You can look at it in the DB. Good Bye! \n"); 
	}
	else if(strcmp(key, "logout") == 0){
		printf("\nGoodbye, have a great day!");
		
	}
	exit(0);
}

void loggedInMessage(){
	printf("\n\n*********************************************");
	printf("\n*                                           *");
	printf("\n*  Congratulations! You are now logged in.  *");
	printf("\n*                                           *");
	printf("\n*********************************************\n\n");
	
	printf("\nNow that you are logged in, you have access to the following commands:\n");
	printf("\n\"addnote\"\t\tWill create a new note and add it to your account.");
	printf("\n\"logout\"\t\tWill log you out...............");
	
	char command[4096];
	printf("\n\nEnter your next command: ");
	scanf("%s", command);
	loggedInCommands(command);
}

void welcomeSwitch(char *key){
	if(strcmp(key, "login") == 0){
		loginPrompt();
	}
	else if(strcmp(key, "signup") == 0){
		signUpPrompt();
	}
	else if(strcmp(key, "exit") == 0){
		printf("\nGoodbye, have a great day!\n\n\n");
		exit(0);
	}
	else{
		printf("\nYour command is not recognized... Ending program.\n");
		exit(0);
	}
}

int main(int argc, char *argv[]){
	srand(time(NULL));
	//print introductory message
	char *welcomeChar = welcomeMessage();
	//switch statement for scanf -- either login or sign up
	welcomeSwitch(welcomeChar);

	return 0;
}
