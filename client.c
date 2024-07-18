#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sqlite3.h>


/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port = 2728;
const char *ipAddress = "127.0.0.1";
//definite pentru usurinta rularii progragmului in timpul lucrarii la acesta

int main(int argc, char *argv[])
{
    int okwhile = 1;
    int sd; //descriptor socket
    struct sockaddr_in server; //structura conectare
    char msg[100];
    
    if(argc != 3)
    {
      printf("[client] Sintaxa : %s <adresa_server> <port>\n",argv[0]);
      return -1;
    }

    port = atoi(argv[2]);

    if((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[client] Eroare la socket().\n");
        return errno;
    }

    /* umplem structura folosita pentru realizarea conexiunii */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);


  /* ne conectam la server *///--------------------------------------------------
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
char utilizator[50];
int k=0;
while(okwhile)
{
  printf("[client]Executati comanda: ");
  fflush(stdout);
  
  /* citirea mesajului */
  bzero (msg, 100);
  read (0, msg, 100);


  /* trimiterea mesajului la server *///-----------------------------------------
  if (write (sd, msg, 100) <= 0)
    {
      perror ("[client]Eroare la write() spre server.\n");
      return errno;
    }

  /* citirea raspunsului dat de server 
     (apel blocant pana cand serverul raspunde) *///------------------------------
  if (read (sd, msg, 100) < 0)
    {
      perror ("[client]Eroare la read() de la server.\n");
      return errno;
    }

  //check for code for file opening? else --

  /* afisam mesajul primit */
    printf ("[client]Mesajul primit: %s\n", msg);
    fflush(stdout);

  if(strncmp(msg,"Login succesfull for ",21)==0)
  {
    for(int i=21; i<strlen(msg); i++)
     {
      utilizator[k]=msg[i];
      k++;
     }
     utilizator[k]='\0';
  }
  if(strcmp(msg, "Logged out.") == 0)
  {
    /* inchidem conexiunea, am terminat */
    okwhile = 0;
    close(sd);
    exit(0);
  }

      } //while
  return 0;
} //main