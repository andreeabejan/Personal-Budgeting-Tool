#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>


/* portul folosit */

#define PORT 2728

extern int errno;		/* eroarea returnata de unele apeluri */

/*variabile necesarii bazei de date sql*/
sqlite3 *db;
sqlite3_stmt *res;
sqlite3_stmt *stmt;
char *sql;
char *zErrMsg = 0;
int error = 0;
int fd;		         	/* descriptor folosit pentru 
				                   parcurgerea listelor de descriptori */

/*NotUsed - additional data, 
argc - nr. of columns, 
**argv - array of strings, 
**azColName - array of strings representing column names*/

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
   int i;
   for(i = 0; i<argc; i++) {
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   *(int *)NotUsed = 1;
   return 0;
}

void BazaDeDate()
{
  /*Open a database file*/
  int rc = sqlite3_open("database.db", &db);
  if(rc != SQLITE_OK){
    printf("[server] Eroare la deschiderea bazei de date");
    fflush (stdout);
    exit(0);
  }

  sqlite3_prepare_v2(db,"SELECT ID, password, name, logged_in, card1, card2, savings, NRdepuneri, NRretrageri FROM utilizatori", -1, &stmt, 0);

  while (sqlite3_step(stmt) != SQLITE_DONE){
      printf("%d|", sqlite3_column_int(stmt,0));
      printf("%s|", sqlite3_column_text(stmt,1));
      printf("%s|", sqlite3_column_text(stmt,2));
      printf("%d|", sqlite3_column_int(stmt,3));
      printf("%d|", sqlite3_column_int(stmt,4));
      printf("%d|", sqlite3_column_int(stmt,5));
      printf("%d|", sqlite3_column_int(stmt,6));
      printf("%d|", sqlite3_column_int(stmt,7));
      printf("%d|", sqlite3_column_int(stmt,8));
      printf("\n");
      fflush (stdout);
  }


  sqlite3_finalize(stmt);
}

int char_in_int (char nr[])
{int n = 0;
  for(int i=0; i<strlen(nr); i++)
    n = n*10 + (nr[i]-'0');
  return n;
}

int nu_te_poti_loga_de_doua_ori(char nume[])
{
  int found = 0;
  int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        fflush(stdout);
        return 0;
    }

    const char *numele_gasit = nume; 

    char sql[1000];
    int result = snprintf(sql, sizeof(sql), "SELECT * FROM utilizatori WHERE name = '%s' AND logged_in != 0", numele_gasit);

  
    rc = sqlite3_exec(db, sql, callback, &found, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //found = 1; // If rows are found, set found flag to 1
    }
    return found;
}

void cresteNRdepuneri(int descriptor)
{
  int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }

  const char *sql_query = "UPDATE utilizatori SET NRdepuneri = NRdepuneri + 1 WHERE logged_in = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement la NRdepuneri: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, descriptor);

    // Execute the update query
    rc = sqlite3_step(stmt);
}


void cresteNRretrageri(int descriptor)
{
  int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }

  const char *sql_query = "UPDATE utilizatori SET NRretrageri = NRretrageri + 1 WHERE logged_in = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement la NRretrageri: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, descriptor);

    // Execute the update query
    rc = sqlite3_step(stmt);
}

int cauta_cont(char nume[])
{
  int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }

    char *search_string = nume;  
    const char *sql_query = "SELECT COUNT(*) FROM utilizatori WHERE name LIKE ?;";
    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    sqlite3_bind_text(stmt, 1, search_string, -1, SQLITE_STATIC);

    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

   // printf("Number of rows where the string '%s' is found: %d\n", search_string, count);

    sqlite3_finalize(stmt);
    
    if(count!=0){return 1;}
    else return 0;
    
}

int verifica_parola(char nume[], char parola[])
{
  int found = 0; // Flag to indicate if vector is found (initially set to 0)

    int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        fflush(stdout);
        return 0;
    }

    const char *numele_gasit = nume; 
    const char *parola_cautata = parola; 

    char sql[1000];
    int result = snprintf(sql, sizeof(sql), "SELECT * FROM utilizatori WHERE name = '%s' AND "
                                "password = '%s';", numele_gasit, parola_cautata);

  
    rc = sqlite3_exec(db, sql, callback, &found, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        //found = 1; // If rows are found, set found flag to 1
    }

    return found;
}
void logg_in(char nume[])
{
  int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }

    const char *knownValue = nume;
    int newValue = fd; 

    char sql[1000];
    snprintf(sql, sizeof(sql), "UPDATE utilizatori SET logged_in = %d WHERE name = '%s';",
             newValue, knownValue);

    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
    } else {
       // printf("Column updated successfully.\n");
    }

}

int check_logged(int number_to_search)
{
    int rc = sqlite3_open("database.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
    }

    const char *query = "SELECT COUNT(*) FROM utilizatori WHERE logged_in = ?";
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, number_to_search);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            return (count > 0) ? 1 : 0; // Return 1 if number is found, 0 otherwise
        } else {
            fprintf(stderr, "No data found\n");
            fflush(stdout);
        }
    } else {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        fflush(stdout);
    }

    sqlite3_finalize(stmt);
    return 0; // Return 0 if search encounters an error or doesn't find the number
}

char * analiza_financiara(int descriptor)
{
  if(check_logged(descriptor)==0) return "Must be logged in for this operation";
  else {
    int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }
    char query[1000];
    snprintf(query, sizeof(query), "SELECT NRdepuneri, NRretrageri, card1, card2, savings FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        char* result = (char*)malloc(1000 * sizeof(char)); // Adjust the size as per your data
        if (result == NULL) {
            fprintf(stderr, "Memory allocation error.\n");
            sqlite3_finalize(stmt);
            return NULL;
        }

        result[0] = '\0'; 
        int total = 0;
            
            char intStr[50]; 
            strcat(result,"\nNumar depuneri: ");
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 0));
            strcat(result, intStr);
            //strcat(result, "\n"); 
            memset(intStr, 0, sizeof(intStr));
            strcat(result,"\nNumar transferuri: ");
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 1));
            strcat(result, intStr);
            strcat(result, "\n"); 
            memset(intStr, 0, sizeof(intStr));
            float savings = sqlite3_column_int(stmt,4);
            total = sqlite3_column_int(stmt,2) + sqlite3_column_int(stmt,3) + savings;
            float procent = ((float)savings / total ) * 100;
            strcat(result, "Savings contine ");
            sprintf(intStr, "%.2f", procent);
            strcat(result, intStr);
            strcat(result,"% din total.\n");
            //strcat(result,"\n");
            memset(intStr, 0, sizeof(intStr));
           sprintf(intStr, "%s", "Recomandare?\n");
                             strcat(result, intStr);
                             memset(intStr, 0, sizeof(intStr));
            strcat(result,"\n");
        sqlite3_finalize(stmt);
       

        return result;
    } else {
        fprintf(stderr, "No rows found for the given value.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }
  
}}

char * recomandare(int descriptor)
{
  if(check_logged(descriptor)==0) return "Must be logged in for this operation";
  else {
    int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }
    char query[1000];
    snprintf(query, sizeof(query), "SELECT NRdepuneri, NRretrageri, card1, card2, savings FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        char* result = (char*)malloc(1000 * sizeof(char)); // Adjust the size as per your data
        if (result == NULL) {
            fprintf(stderr, "Memory allocation error.\n");
            sqlite3_finalize(stmt);
            return NULL;
        }

        result[0] = '\0'; 
        int total = 0;
            
            char intStr[50]; 
            memset(intStr, 0, sizeof(intStr));
            float savings = sqlite3_column_int(stmt,4);
            total = sqlite3_column_int(stmt,2) + sqlite3_column_int(stmt,3) + savings;
            float procent = ((float)savings / total ) * 100;
            strcat(result, "\n Savings contine ");
            sprintf(intStr, "%.2f", procent);
            strcat(result, intStr);
            strcat(result,"% din total.\n");
            //strcat(result,"\n");
            memset(intStr, 0, sizeof(intStr));
            if(procent>=20) {sprintf(intStr, "%s", " Felicitari! Recomandare savings: minim 20%.\n");
                             strcat(result, intStr);
                             memset(intStr, 0, sizeof(intStr));}
            else {sprintf(intStr, "%s", " Recomandare: minim 20% in savings.\n");
                  strcat(result, intStr);
                  memset(intStr, 0, sizeof(intStr));}
            strcat(result,"\n");
        sqlite3_finalize(stmt);
       

        return result;
    } else {
        fprintf(stderr, "No rows found for the given value.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }
  
}
}

int ok=0;
char * login_command(char nume[], char parola[])
{
  if(nu_te_poti_loga_de_doua_ori(nume)!=0) return "Account is already in use.";
  else if(check_logged(fd)==1) return "Someone is already logged in.";
  else {
    char *raspuns = (char *)malloc(100 * sizeof(char));
    if (raspuns == NULL) {
        // Handle memory allocation failure
        return NULL;
    }
  raspuns[0]='\0';
  

  if(cauta_cont(nume)!=0)
  {
  //a gasit contul

  if (verifica_parola(nume, parola)!=0){
    strcat(raspuns,"Login succesfull for ");
    strcat(raspuns, nume);
    logg_in(nume);
  }
  else strcat(raspuns,"Parola introdusa este incorecta.");
  }
  else
  {
  //contul nu a fost gasit
  strcat(raspuns, "Contul nu a fost gasit.");
  }

  return raspuns;
  }
}

void adauga_suma_pentru_destinatar(char nume[], int suma)
{
    int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }
  const char *sql_query = "UPDATE utilizatori SET card1 = card1 + ? WHERE name = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, suma);
    //sqlite3_bind_text(stmt, 2, nume);
    sqlite3_bind_text(stmt, 2, nume, -1, SQLITE_TRANSIENT);

    // Execute the update query
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update failed la adaugarea sumei pt destinatar: %s\n", sqlite3_errmsg(db));
    } 
    // Finalize and close the statement
    //sqlite3_finalize(stmt);

}

int e_posibil_transferul(int n, int descriptor, int toTransfer)
{ 
   int rc = sqlite3_open("database.db", &db);
     if (rc) {
         fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
         fflush(stderr);
     }

    if(n==1){
    char query[1000];
    snprintf(query, sizeof(query), "SELECT card1 FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {

            char intStr[20]; 
            
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 0));
            int nr = char_in_int(intStr);
            if(toTransfer > nr) { sqlite3_finalize(stmt); return 0; } //nu se poate efectua transferul
            else {sqlite3_finalize(stmt);return 1;}
    }
    }
    else if(n==2){
    char query[1000];
    snprintf(query, sizeof(query), "SELECT card2 FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {

            char intStr[20]; 
            
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 0));
            int nr = char_in_int(intStr);
            if(toTransfer > nr) {sqlite3_finalize(stmt);return 0; } //nu se poate efectua transferul
            else {sqlite3_finalize(stmt);return 1;}
    }
    }
    else if(n==3){
    char query[1000];
    snprintf(query, sizeof(query), "SELECT savings FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {

            char intStr[20]; 
            
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 0));
            int nr = char_in_int(intStr);
            if(toTransfer > nr) {sqlite3_finalize(stmt);return 0; } //nu se poate efectua transferul
            else {sqlite3_finalize(stmt);return 1;}
      
    }
    }

}

char * transfer_command(int card, char destinatar[], int suma, int descriptor)
{ 
  char *raspuns = (char *)malloc(100 * sizeof(char));
    if (raspuns == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

  int rc = sqlite3_open("database.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        free(raspuns);
        return NULL;
    }
  
  raspuns[0]='\0';
  //printf("Am primit: card: %d, destinatar: %s, suma: %d, descriptor: %d", card, destinatar, suma, descriptor);
  //fflush(stdout);

  if(check_logged(fd)==0) strcat(raspuns,"Must be logged in for this command");
  else if (cauta_cont(destinatar)==0) strcat(raspuns, "Contul pentru transfer nu exista.");
  else if (card!=1 && card!=2 && card!= 3) strcat(raspuns, "Cardul mentionat nu exista.");
  else if(e_posibil_transferul(card, descriptor, suma)==0) strcat(raspuns,"Fonduri insuficiente pentru transfer.");
  else {   
     if(card==1){
    // Define the SQL query to update the int column based on conditions
    const char *sql_query = "UPDATE utilizatori SET card1 = card1 - ? WHERE logged_in = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "[here]Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, suma);
    sqlite3_bind_int(stmt, 2, descriptor);

    // Execute the update query
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
    } else {
      cresteNRretrageri(descriptor);
      adauga_suma_pentru_destinatar(destinatar, suma);
    }
   
    // Finalize and close the statement
    sqlite3_finalize(stmt);

    return "Transfer efectuat cu succes.";

    }
    else if(card==2){
    // Define the SQL query to update the int column based on conditions
    const char *sql_query = "UPDATE utilizatori SET card2 = card2 - ? WHERE logged_in = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, suma);
    sqlite3_bind_int(stmt, 2, descriptor);

    // Execute the update query
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
    } else {
      strcat(raspuns,"Transfer efectuat cu succes.\n");
      cresteNRretrageri(descriptor);
      adauga_suma_pentru_destinatar(destinatar, suma);
    }
   
    // Finalize and close the statement
    sqlite3_finalize(stmt);

    return raspuns;
    }
    else if(card==3){
    // Define the SQL query to update the int column based on conditions
    const char *sql_query = "UPDATE utilizatori SET savings = savings - ? WHERE logged_in = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, suma);
    sqlite3_bind_int(stmt, 2, descriptor);

    // Execute the update query
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
    } else {
      strcat(raspuns,"Transfer efectuat cu succes.\n");
      cresteNRretrageri(descriptor);
      adauga_suma_pentru_destinatar(destinatar, suma);
    }
   
    // Finalize and close the statement
    sqlite3_finalize(stmt);

    return raspuns;
    }
    
    else return "Card number does not exist.";
  }
 }


char * depunere_numerar(int n, int suma, int descriptor)
{
  if(check_logged(descriptor)==0){ return "Must be logged in for this comand";}
  else{
    int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }

    char* result = (char*)malloc(1000 * sizeof(char)); // Adjust the size as per your data
    if (result == NULL) {
            fprintf(stderr, "Memory allocation error.\n");
            sqlite3_finalize(stmt);}
   result[0] = '\0'; 


    if(n==1){
    // Define the SQL query to update the int column based on conditions
    const char *sql_query = "UPDATE utilizatori SET card1 = card1 + ? WHERE logged_in = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, suma);
    sqlite3_bind_int(stmt, 2, descriptor);

    // Execute the update query
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
    } else {
      strcat(result,"Depunere efectuata cu succes.");
      printf("Update successful!\n");
      cresteNRdepuneri(descriptor);
      fflush(stdout);
    }

    // Finalize and close the statement
    sqlite3_finalize(stmt);

    return result;

    }
    else if(n==2){
    // Define the SQL query to update the int column based on conditions
    const char *sql_query = "UPDATE utilizatori SET card2 = card2 + ? WHERE logged_in = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, suma);
    sqlite3_bind_int(stmt, 2, descriptor);

    // Execute the update query
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
    } else {
      strcat(result,"Depunere efectuata cu succes.");
      printf("Update successful!\n");
      cresteNRdepuneri(descriptor);
      fflush(stdout);
    }

    // Finalize and close the statement
    sqlite3_finalize(stmt);

      return result;
    }
    else if(n==3){
    // Define the SQL query to update the int column based on conditions
    const char *sql_query = "UPDATE utilizatori SET savings = savings + ? WHERE logged_in = ?;";

    rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
    }

    // Bind values to the prepared statement
    sqlite3_bind_int(stmt, 1, suma);
    sqlite3_bind_int(stmt, 2, descriptor);

    // Execute the update query
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Update failed: %s\n", sqlite3_errmsg(db));
    } else {
      strcat(result,"Depunere efectuata cu succes.");
      printf("Update successful!\n");
      cresteNRdepuneri(descriptor);
      fflush(stdout);
    }

    // Finalize and close the statement
    sqlite3_finalize(stmt);

      return result;
    }
    
    else return "Card number does not exist.";
  }
}

char * check_card(int n, int descriptor)
{ 
  if(check_logged(descriptor)==0) return "Must be logged in for this command.";
  else{
  int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }

    if(n==1){
    char query[1000];
    snprintf(query, sizeof(query), "SELECT card1 FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        char* result = (char*)malloc(1000 * sizeof(char)); // Adjust the size as per your data
        if (result == NULL) {
            fprintf(stderr, "Memory allocation error.\n");
            sqlite3_finalize(stmt);
            return NULL;
        }

        result[0] = '\0'; 

            char intStr[20]; 
            strcat(result,"Status card1: ");
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 0));
            strcat(result, intStr);
            strcat(result, "\n");
        

        sqlite3_finalize(stmt);
    
        return result;
    } else {
        fprintf(stderr, "No rows found for the given value.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }
    }
    else if(n==2){
    char query[1000];
    snprintf(query, sizeof(query), "SELECT card2 FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        char* result = (char*)malloc(1000 * sizeof(char)); // Adjust the size as per your data
        if (result == NULL) {
            fprintf(stderr, "Memory allocation error.\n");
            sqlite3_finalize(stmt);
            return NULL;
        }

        result[0] = '\0'; 

            char intStr[20]; 
            strcat(result,"Status card2: ");
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 0));
            strcat(result, intStr);
            strcat(result, "\n");
        

        sqlite3_finalize(stmt);
    
        return result;
    } else {
        fprintf(stderr, "No rows found for the given value.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }
    }
    else if(n==3){
    char query[1000];
    snprintf(query, sizeof(query), "SELECT savings FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        char* result = (char*)malloc(1000 * sizeof(char)); // Adjust the size as per your data
        if (result == NULL) {
            fprintf(stderr, "Memory allocation error.\n");
            sqlite3_finalize(stmt);
            return NULL;
        }

        result[0] = '\0'; 

            char intStr[20]; 
            strcat(result,"Status savings: ");
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 0));
            strcat(result, intStr);
            strcat(result, "\n");
        

        sqlite3_finalize(stmt);
    
        return result;
    } else {
        fprintf(stderr, "No rows found for the given value.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }}
    else return "Card number does not exist.";
}
}

char * check_account(int descriptor)
{
  if(check_logged(descriptor)==0) return "Must be logged in for this operation";
  else {
    int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        fflush(stderr);
    }
    char query[1000];
    snprintf(query, sizeof(query), "SELECT ID, name, logged_in, card1, card2, savings, NRdepuneri, NRretrageri FROM utilizatori WHERE logged_in = %d", descriptor);

    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare query: %s\n", sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        char* result = (char*)malloc(1000 * sizeof(char)); // Adjust the size as per your data
        if (result == NULL) {
            fprintf(stderr, "Memory allocation error.\n");
            sqlite3_finalize(stmt);
            return NULL;
        }

        result[0] = '\0'; // Ensure the string is initially empty

            char intStr[20]; // Assuming integers can fit within 20 characters
            strcat(result,"\nSuma card1: ");
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 3));
            strcat(result, intStr);
            strcat(result, "\n");
            strcat(result,"Suma card2: "); 
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 4));
            strcat(result, intStr);
            strcat(result, "\n");
            strcat(result,"Suma savings: "); 
            memset(intStr, 0, sizeof(intStr));
            sprintf(intStr, "%d", sqlite3_column_int(stmt, 5));
            strcat(result, intStr);
            strcat(result, "\n"); 
            memset(intStr, 0, sizeof(intStr));
        

        sqlite3_finalize(stmt);
       

        return result;
    } else {
        fprintf(stderr, "No rows found for the given value.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }
}}

/* functie de convertire a adresei IP a clientului in sir de caractere */
char * conv_addr (struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy (str, inet_ntoa (address.sin_addr));	
  /* portul utilizat de client */
  bzero (port, 7);
  sprintf (port, ":%d", ntohs (address.sin_port));	
  strcat (str, port);
  return (str);
}

void database_logout(int fd)
{
  int rc = sqlite3_open("database.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
    }
    int knownValue = fd;
    int newValue = 0; 

    char sql[1000];
    snprintf(sql, sizeof(sql), "UPDATE utilizatori SET logged_in = %d WHERE logged_in = '%d';",
             newValue, knownValue);

    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
    } 
}

/* realizeaza primirea si retrimiterea unui mesaj unui client */
int comunicareClient(int fd, fd_set actfds)
{
  char buffer[100];		/* mesajul */
  int bytes;			/* numarul de octeti cititi/scrisi */
  char msg[100];		/*mesajul primit de la client*/ 
  char msgrasp[100]=" ";        /*mesaj de raspuns pentru client*/

  /*recv*///------------------------------------------------------------------
  bytes = read (fd, msg, sizeof (buffer));
  if (bytes < 0)
    {
      perror ("Eroare la read() de la client.\n");
      return 0;
    }
   
  //printf ("[server]Am receptionat comanda:%s\n", msg);
  //fflush(stdout);

  msg[strlen(msg)-1]='\0';
  
  /*pregatim mesajul de raspuns */
  bzero(msgrasp,100);

/*cazul comenzii Login*/
   if(strncmp(msg,"Login", 5)==0) 
        {char message[100];
          strcpy(message, msg);
          char *cuvant;
          char sir[100][100];
          int k=0;
          cuvant = strtok(message," ");
          while(cuvant != NULL)
          {
              strcpy(sir[k], cuvant);
              cuvant = strtok(NULL," ");
              k++;
          }
           if(k!=3){strcpy(msgrasp,"Login format: Login [nume utilizator] [parola]");}
          else{strcpy(msgrasp,login_command(sir[1],sir[2]));}
         }
    
/*cazul comenzii Logout*/
    else if(strncmp(msg,"Logout", 6)==0)
         {if(check_logged(fd)==0) strcpy(msgrasp,"Must be logged in for this command.");
         else{
          database_logout(fd);
          strcpy(msgrasp,"Logged out.");
          printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
		      fflush (stdout);
          //sqlite3_close(db); /*inchidem baza de date*/
          close (fd);		/* inchidem conexiunea cu clientul */ 
		      FD_CLR (fd, &actfds);/* scoatem si din multime */
          exit(1);}
          }

/*cazul comenzii Depunere numerar*/
    else if(strncmp(msg,"Depunere numerar",16)==0)
    {    char message[100];
          strcpy(message, msg);
          char *cuvant;
          char sir[100][100];
          int k=0;
          cuvant = strtok(message," ");
          while(cuvant != NULL)
          {
              strcpy(sir[k], cuvant);
              cuvant = strtok(NULL," ");
              k++;
          }
          if(k!=4) strcpy(msgrasp,"Format: Depunere numerar [nr_card] [suma]");
          else{
           int a = sir[2][0]-'0'; // nr card
           int b = char_in_int(sir[3]); // suma
            strcpy(msgrasp, depunere_numerar(a, b, fd));
          }}

/*cazul comenzii Transfer*/          
else if(strncmp(msg,"Transfer", 8) == 0)
         {
          char message[100];
          strcpy(message, msg);
          char *cuvant;
          char sir[100][100];
          int k=0;
          cuvant = strtok(message," ");
          while(cuvant != NULL)
          {
              strcpy(sir[k], cuvant);
              cuvant = strtok(NULL," ");
              k++;
          }
          if(k!=4) strcpy(msgrasp,"Transfer format: Transfer <nrcard> <user_destinatar> <suma>");
          else{int a = sir[1][0]-'0';
               int b = char_in_int(sir[3]);
               strcpy(msgrasp,transfer_command(a,sir[2],b, fd));}
         }

/*cazul comenzii Check card*/
      else if(strncmp(msg,"Check card",10)==0)
          {char message[100];
          strcpy(message, msg);
          char *cuvant;
          char sir[100][100];
          int k=0;
          cuvant = strtok(message," ");
          while(cuvant != NULL)
          {
              strcpy(sir[k], cuvant);
              cuvant = strtok(NULL," ");
              k++;
          }
          if(k!=3) strcpy(msgrasp,"\n Check card format: Check card <nrcard>. \n Options: \n 1 for card1 \n 2 for card2 \n 3 for savings. \n");
          else{int nr = msg[11]-'0';
           strcpy(msgrasp,check_card(nr, fd));}
          }

/*cazul comenzii Check account*/
      else if(strncmp(msg,"Check account",13)==0)
          {strcpy(msgrasp,check_account(fd));}


/*cazul comenzii Analiza financiara*/
    else if(strcmp(msg,"Analiza financiara") == 0)
    {strcpy(msgrasp, analiza_financiara(fd));}

/*cazul comenzii Recomandare*/
    else if(strcmp(msg,"Recomandare")==0)
    {strcpy(msgrasp, recomandare(fd));}

/*cazul apelarii unei comenzi inexistente*/
      else {strncpy(msgrasp,"Comanda nu exista.\n",20);}


  /*send*///-------------------------------------------------------------------    
  if (bytes && write (fd, msgrasp, bytes) < 0)
    {
      perror ("[server] Eroare la write() catre client.\n");
      return 0;
    }
  
  return bytes;
}

int main ()
{
  struct sockaddr_in server;	   /* structura pentru server */
  struct sockaddr_in from; /*structura pentru clienti*/
  fd_set readfds;		            /* multimea descriptorilor de citire */
  fd_set actfds;	       	/* multimea descriptorilor activi */
  struct timeval tv;		       /* structura de timp pentru select() */
  int sd, client;	     	/* descriptori de socket */
  int optval=1; 		         	/* optiune folosita pentru setsockopt()*/ 

  int nfds;			              /* numarul maxim de descriptori */
  int len;		     	/* lungimea structurii sockaddr_in */

  /* creare socket *///----------------------------------------------------
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server] Eroare la socket().\n");
      return errno;
    }

  /*setam pentru socket optiunea SO_REUSEADDR *///////////// 
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

  /* pregatim structurile de date */
  bzero (&server, sizeof (server));

  /* umplem structura folosita de server */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  server.sin_port = htons (PORT);

  /* atasam socketul *///--------------------------------------------------------
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server] Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze *///--------------
   if (listen (sd, 5) == -1)
    {
      perror ("[server] Eroare la listen().\n");
      return errno;
    }

    /* completam multimea de descriptori de citire */
  FD_ZERO (&actfds);		/* initial, multimea este vida */
  FD_SET (sd, &actfds);		/* includem in multime socketul creat */

  tv.tv_sec = 1;		/* se va astepta un timp de 1 sec. */
  tv.tv_usec = 0;
  
  /* valoarea maxima a descriptorilor folositi */
  nfds = sd;

  printf ("[server] Serverul e pregatit la portul %d...\n", PORT);
  fflush (stdout);


  /* servim in mod concurent clientii*/
  while (1)
    { 
      /* ajustam multimea descriptorilor activi (efectiv utilizati) */
      bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));

  /* apelul select() *///---------------------------------------------------------
      if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
	{
	  perror ("[server] Eroare la select().\n");
	  return errno;
	}
      /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
      if (FD_ISSET (sd, &readfds))
	{
	  /* pregatirea structurii client */
	  len = sizeof (from);
	  bzero (&from, sizeof (from));

	/* a venit un client, acceptam conexiunea *///---------------------------
	  client = accept (sd, (struct sockaddr *) &from, &len);

	  /* eroare la acceptarea conexiunii de la un client */
	  if (client < 0)
	    {
	      perror ("[server] Eroare la accept().\n");
	      continue;
	    }

    if (nfds < client) /* ajusteaza valoarea maximului */
            nfds = client;
            
	  /* includem in lista de descriptori activi si acest socket */
	  FD_SET (client, &actfds);

    BazaDeDate();

	  printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
	  fflush (stdout);
	}
     /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
      for (fd = 0; fd <= nfds; fd++)	/* parcurgem multimea de descriptori */
	{

	  /* este un socket de citire pregatit? */
	  if (fd != sd && FD_ISSET (fd, &readfds))
	    {
		  if(fork()==0)
		  { // proces copil
	      comunicareClient(fd, actfds);
		  }}
		else { // proces parinte
			continue;
		}

	}			/* for */
    }				/* while */
}				/* main */

