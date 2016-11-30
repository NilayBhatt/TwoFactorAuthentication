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

int status = 0; //0 = logged out, 1 = logged in

#define FROM "<edmiralagic@gmail.com>"
#define authenticateSql = "SELECT user_id FROM users WHERE user_id = "
#define getNotesSql = "SELECT * FROM userNotes WHERE user_id = "

static char *payload_text[11];

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

	char buf[BUFSIZ] = "INSERT INTO users (user_id, password, email) VALUES ( ";
	
	//char* passwordHash = getMD5hash(password);
	strcat(buf, username);
	strcat(buf, ", ");
	strcat(buf, password);
	strcat(buf, ", ");
	strcat(buf, email);
	strcat(buf, " )");
	printf("%s\n", buf);
	if (mysql_query(con, buf)){
		finish_with_error(con);
		mysql_free_result(result);
		mysql_close(con);
		return 0;
	}
	mysql_free_result(result);
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

	char buf[BUFSIZ] = "SELECT username FROM users WHERE username = ";
	strcat(buf,user);
	strcat(buf, " AND password = ");
	strcat(buf, password);

	if (mysql_query(con, buf)){
		finish_with_error(con);
	}

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

char* getUserEmail(char* user){
	MYSQL *con = mysql_init(NULL);

	if (con == NULL){
		fprintf(stderr, "mysql_init() failed\n");
		exit(1);
	}  

	if (mysql_real_connect(con, "localhost", "root", "sa", 
		"pithentication", 0, NULL, 0) == NULL){
		finish_with_error(con);
	}   

	char buf[BUFSIZ] = "SELECT email FROM users WHERE username = ";
	char *email;
	strcat(buf, user);

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
	unsigned char* result = malloc(MD5_DIGEST_LENGTH);
 	MD5(string, strlen(string), result);

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
	printf("\"exit\"\t\tWill exit the program\n\n");
	while(1){
		printf("What would you like to do: ");
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

void adjustEmailPayload(char *email, char *code){
	char temp[200];
	strcpy(temp, "To: <");
	strcat(temp, email);
	strcat(temp, ">\r\n");
	char temp2[200];
	strcpy(temp2, "Your verification code is: ");
	strcat(temp2, code);
	strcat(temp2, "\r\n");
	payload_text[0] = "Date: Mon, 29 Nov 2010 21:54:29 +1100\r\n";
	payload_text[1] = temp;
	payload_text[2] = "From: " FROM "(Pithentication Support)\r\n";
	payload_text[3] = "Subject: Pithentication Verify Code\r\n";
 	payload_text[4] =  "\r\n";
 	payload_text[5] =  "You are one step closer to activating your account!\r\n";
 	payload_text[6] = "\r\n";
 	payload_text[7] = "Your verification code is: ";
 	payload_text[8] = code;
 	payload_text[9] = "\r\n";
	payload_text[10] = NULL;
}

void signUpPrompt(){
	char username[4096];
	char password[4096];
	char tempPass[4096];
	char email[4096];
	char verCode[9];
	printf("\nYou are one step closer to having your own Pithentication account!\n");
	while(1){
		printf("Please enter your desired username (required): ");
		scanf("%s", username);
		if(username != NULL){
			break;
		}
	}
	while(1){
		printf("Please enter your desired password (required): ");
		scanf("%s", password);
		printf("Please re-enter your desired password (required): ");
		scanf("%s", tempPass);
		if(strcmp(password, tempPass) == 0 && password != NULL){
			break;
		}
		printf("\nUh oh! The passwords did not match.. lets try again!");
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

void loginCommand(char *username, char *password){
	int authStatus = authenticateUser(username, password);
	if(authStatus){
		status = 1;
		loggedInMessage();
	}
	else{
		status = 0;
	}
}

void loginPrompt(){
	char username[4096];
	char password[4096];
	printf("\nWelcome to Pithentication! Please login to your account below.\n");
	printf("Please enter your username: ");
	scanf("%s", username);
	printf("Please enter your password: ");
	scanf("%s", password);
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
		char *email = getEmail(username);
		adjustEmailPayload(email, code);
		int emailStatus = emailCode(email);
		printf("A verification code has been sent to your registered email..");
		printf("\nPlease enter the verification code: ");
		scanf("%s", verCode);
		if(strcmp(newCode, verCode) == 0){
			loginCommand(username, password);
		}
	}
}

void loggedInMessage(){
	printf("\nCongratulations! You are now logged in.");
	printf("\nYou now have access teo the following commands: ");
	printf("\nEat my ass;;;;;");
}

void welcomeSwitch(char *key){
	if(strcmp(key, "login") == 0){
		loginPrompt();
	}
	else if(strcmp(key, "signup") == 0){
		signUpPrompt();
	}
	else if(strcmp(key, "exit") == 0){
		printf("\nGoodbye, have a great day!\n\n");
		exit(0);
	}
}

int main(int argc, char *argv[]){
	//print introductory message
	char *welcomeChar = welcomeMessage();
	//switch statement for scanf -- either login or sign up
	welcomeSwitch(welcomeChar);

	return 0;
}