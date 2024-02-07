#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "sqlite3.h"
#include <stdbool.h>

#define PORT 2908
#define MAX_CLIENTS 2

extern int errno;

struct cLienti {
    int id_client;
    char nume_client[30];   
}vector_clienti[100];

int nr_clienti = 0;

sqlite3 *dataBase;

typedef struct thData {
    int idThread; 
    int cl; 
}thData;

void create_dataBase();

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clienti */

void raspunde(void *);

int is_command(char *msg);

void registerr(void *arg);

void connectt(void *arg);

bool exists(char *username);

void adminn(void *arg, char *username, char *password);

void delete(void *arg, char *username, char *password);

void quit(void *arg, int ok, char *username, char *password);

void set_profile(void *arg, char *username, char *password);

int exists_fri(char *username, char *user);

void add_friend(void *arg, char *username, char *password);

void sendd(void *arg ,char *username, char *password);

void make_post(void *arg, char *username, char *password);

void see_posts(void *arg, char *username, char *password);

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;	
    int msg[100];		//mesajul primit de trimis la client 
    int sd;		//descriptorul de socket 
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;

    sqlite3_stmt *stmt;

    create_dataBase();

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        vector_clienti[i].id_client = -1;
    }
    
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1) {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }
    
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
    
    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));
    
    server.sin_family = AF_INET;	
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons (PORT);
    
    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1) {
        perror ("[server]Eroare la bind().\n");
        return errno;
    }

    if (listen (sd, 5) == -1) {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }

    while (1) {
        
        int client;
        thData * td; //parametru functia executata de thread     
        int length = sizeof (from);

        printf ("[server]Asteptam la portul %d...\n",PORT);
        fflush (stdout);

        // client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0) {
            perror ("[server]Eroare la accept().\n");
            continue;
        }

        td=(struct thData*)malloc(sizeof(struct thData));	
        td->idThread=i++;
        td->cl=client;

        pthread_create(&th[i], NULL, &treat, td);	      
    }

    return 0;    
}	

void create_dataBase() {
    char *err;
    int rc;

    rc = sqlite3_open("myDataBase", &dataBase);
    if (rc != SQLITE_OK) {
        perror("[server] eroare la deschidere dataBase...\n");
        exit(1);
    }

    rc = sqlite3_exec(dataBase, "CREATE TABLE IF NOT EXISTS Users(username VARCHAR2, password VARCHAR2, type VARCHAR2, profile VARCHAR2, conct INTEGER)", NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        perror("[server] eroare tabela Users...\n");
        exit(1);
    }

    rc = sqlite3_exec(dataBase, "CREATE TABLE IF NOT EXISTS Messages(user1 VARCHAR2, user2 VARCHAR2, message VARCHAR2)", NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        perror("[server] eroare tabela Messages...\n");
        exit(1);
    }

    rc = sqlite3_exec(dataBase, "CREATE TABLE IF NOT EXISTS Posts(user VARCHAR2, post VARCHAR2)", NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        perror("[server] eroare tabela Posts...\n");
        exit(1);
    }

    rc = sqlite3_exec(dataBase, "CREATE TABLE IF NOT EXISTS Friends(user1 VARCHAR2, user2 VARCHAR2)", NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        perror("[server] eroare tabela Friends...\n");
        exit(1);
    }
}

static void *treat(void * arg)
{		
	struct thData tdL; 
	tdL= *((struct thData*)arg);	
	printf ("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
	fflush (stdout);		 
	pthread_detach(pthread_self());		
	raspunde((struct thData*)arg);
	close(((struct thData*)arg)->cl);
	return(NULL);	 		
}

void raspunde(void *arg)
{
    char mesaj[100];
    int nr;
	struct thData tdL; 
	tdL= *((struct thData*)arg);
    
	int bytesRead = read(tdL.cl, &mesaj, sizeof(mesaj));
    if (bytesRead <= 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
    } 
	      
	/*pregatim mesajul de raspuns */
	nr = is_command(mesaj);      
	printf("[Thread %d]Trimitem mesajul inapoi...%d\n",tdL.idThread, nr);
    //fflush(stdout);
		      
	/* returnam mesajul clientului */
	if (write (tdL.cl, &nr, sizeof(int)) < 0) {
		printf("[Thread %d] ",tdL.idThread);
	    perror ("[Thread]Eroare la write() catre client.\n");
	}
    
    int ok = 0;
    char username[100], password[100];

    if (nr == 1) 
        registerr((struct thData*)arg);
    else
        if (nr == 2)
            connectt((struct thData*)arg);
        else    
            quit((struct thData*)arg, ok, username, password);
        

}

int is_command(char *msg) {

    if (strcmp(msg, "register") == 0)
        return 1;

    if (strcmp(msg, "connect") == 0)
        return 2;

    if (strcmp(msg, "quit") == 0)
        return 3;

    return 0;
}

void registerr(void *arg) {

    char mesaj[100];
    struct thData tdL; 
	tdL= *((struct thData*)arg);
    int rc;

    if (read(tdL.cl, &mesaj, sizeof(mesaj)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in register.\n");
    }

    char username[30], password[30];
    char *str = strtok(mesaj, " ");
    strcpy(username, str);
    str = strtok(NULL, " ");
    strcpy(password, str);

    printf("%s %s", username, password);
    //fflush(stdout);

    sqlite3_exec(dataBase, "BEGIN TRANSACTION", NULL, NULL, NULL);

    char insert[200];
    sprintf(insert, "INSERT INTO Users (username, password, type, profile, conct) VALUES ('%s', '%s', 'user', 'public', 0)", username, password);
    
    rc = sqlite3_exec(dataBase, insert, NULL, NULL, NULL);
    int verificareRegister = 0;

    if (rc != SQLITE_OK) {
        sqlite3_exec(dataBase, "ROLLBACK", NULL, NULL, NULL);
        perror("[server] Eroare la inserarea utilizatorului in baza de date.\n");
    }
    else {
        sqlite3_exec(dataBase, "COMMIT", NULL, NULL, NULL);
        verificareRegister = 1;
    }

    if (write(tdL.cl, &verificareRegister, sizeof(verificareRegister)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in register.\n");
    }

    //memset(mesaj, 0, sizeof(mesaj));
    char mesaj2[100];
    int bytesRead = read(tdL.cl, &mesaj2, sizeof(mesaj2));
    if (bytesRead <= 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in register.\n");
    }

    printf("%s", mesaj2);
    int ok = 0;

    if (strcmp(mesaj2, "connect") == 0) {
        connectt(arg);
    } else if (strcmp(mesaj2, "quit") == 0) {
        quit((struct thData*)arg, ok, username, password);
    } else {
        printf("[client] comanda gresita\n");
    }
}

void connectt(void *arg) {
    
    char mesaj[100];
    struct thData tdL; 
	tdL= *((struct thData*)arg);
    int rc;
    int ok = 0; // pentru a vedea daca s-a logat vreun user

    if (read(tdL.cl,mesaj,sizeof(mesaj)) < 0 ){
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in register0.\n");
    }

    char username[30], password[30];
    char *str = strtok(mesaj, " ");
    strcpy(username, str);
    str = strtok(NULL, " ");
    strcpy(password, str);

    char verif[200];
    sprintf(verif, "SELECT * FROM Users WHERE username='%s' AND password='%s'", username, password);

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(dataBase, verif, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        perror("[server] eroare la connect pt verificare de user.\n");
    }

    rc = sqlite3_step(stmt);
    int nr;

    if (rc == SQLITE_ROW) {
        ok = 1;
        nr = 1;
        if (write(tdL.cl,&nr,sizeof(nr)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la read() in connect1.\n");
        }
    }
    else {
        nr = 0;
        if (write(tdL.cl,&nr,sizeof(nr)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la read() in connect2.\n");
        }
    }
    
    memset(verif, 0, sizeof(verif));

    if (nr == 1 && rc == SQLITE_ROW) {
        
        sprintf(verif, "UPDATE Users SET conct = 1 WHERE username='%s'", username);

        rc = sqlite3_exec(dataBase, verif, NULL, NULL, NULL);

        if (rc != SQLITE_OK) 
            perror("[server] eroare la actualizarea câmpului conct.\n");
        
    }

    int admin;
    if (strcmp(username, "admin") == 0) {
        admin = 1;
        if (write(tdL.cl,&admin,sizeof(admin)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in admin.\n");
        }
    }
    else {
        admin = 0;
        if (write(tdL.cl,&admin,sizeof(admin)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in admin`.\n");
        }
    }

    if (admin == 1) {
        adminn((struct thData*)arg, username, password);
    }
    else {
        vector_clienti[nr_clienti++].id_client = tdL.cl;
        strcpy(vector_clienti[nr_clienti - 1].nume_client, username);
        
        char new_command[100];
        if (read(tdL.cl,&new_command,sizeof(new_command)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la read() in connect2.\n");
        }

        if (strcmp(new_command, "quit") == 0) 
            quit((struct thData*)arg, ok, username, password);
        else
            if (strcmp(new_command, "set profile") == 0)
                set_profile((struct thData*)arg, username, password);
            else
                if (strcmp(new_command, "add") == 0)
                    add_friend((struct thData*)arg, username, password);
                else    
                    if (strcmp(new_command, "send") == 0)
                        sendd((struct thData*)arg, username, password);
                    else
                        if (strcmp(new_command, "makePost") ==0)
                            make_post((struct thData*)arg, username, password);
                        else
                            if (strcmp(new_command, "seePosts") ==0)
                                see_posts((struct thData*)arg, username, password);
    }

}

bool exists(char *username) {

    char search[201];
    snprintf(search, sizeof(search), "SELECT COUNT(*) FROM Users WHERE username = '%s'", username);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(dataBase, search, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        perror("[server] eroare la pregatirea interogarii in exists()\n");
        return false;
    }

    rc = sqlite3_step(stmt);
    int ok = 0;

    if (rc == SQLITE_ROW) 
        ok = sqlite3_column_int(stmt, 0);

    return ok > 0;

}

void adminn(void *arg, char *username, char *password) {

    struct thData tdL; 
	tdL= *((struct thData*)arg);

    char new_command[100];
    if (read(tdL.cl,&new_command,sizeof(new_command)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in connect2.\n");
    }

    if (strcmp(new_command, "quit") == 0) {
        int ok = 1;
        quit((struct thData*)arg, ok, username, password);
    }
    else
        if (strcmp(new_command, "delete") == 0) {
            delete((struct thData*)arg, username, password);
        }
        else
            adminn((struct thData*)arg, username, password);

}

void delete(void *arg, char *username, char *password) {

    struct thData tdL; 
	tdL= *((struct thData*)arg);

    char user[30];
    if (read(tdL.cl,&user,sizeof(user)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in delete().\n");
    }

    int exista;
    if (!exists(user)) {
        exista = 0;
        if (write(tdL.cl,&exista,sizeof(exista)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in delete().\n");
        }
    }
    else {
        exista = 1;
        if (write(tdL.cl,&exista,sizeof(exista)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in delete().\n");
        }
    }

    if (exista == 0) {
        adminn((struct thData*)arg, username, password);
    }
    else {

        char query[256];

        snprintf(query, sizeof(query), "DELETE FROM Users WHERE username = '%s'", user);
        if (sqlite3_exec(dataBase, query, NULL, NULL, NULL) != SQLITE_OK) {
            perror("[server] Eroare la stergerea din tabela Users.\n");
        }

        snprintf(query, sizeof(query), "DELETE FROM Friends WHERE user1 = '%s' OR user2 = '%s'", user, user);
        if (sqlite3_exec(dataBase, query, NULL, NULL, NULL) != SQLITE_OK) {
            perror("[server] Eroare la stergerea din tabela Friends.\n");
        }

        snprintf(query, sizeof(query), "DELETE FROM Posts WHERE user = '%s'", user);
        if (sqlite3_exec(dataBase, query, NULL, NULL, NULL) != SQLITE_OK) {
            perror("[server] Eroare la stergerea din tabela Posts.\n");
        }

        snprintf(query, sizeof(query), "DELETE FROM Messages WHERE user1 = '%s' OR user2 = '%s'", user, user);
        if (sqlite3_exec(dataBase, query, NULL, NULL, NULL) != SQLITE_OK) {
            perror("[server] Eroare la stergerea din tabela Messages.\n");
        }

        int ok = 1;
        if (write(tdL.cl,&ok,sizeof(ok)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in delete().\n");
        }

        adminn((struct thData*)arg, username, password);
    }

}

void quit(void *arg, int ok, char *username, char *password) {

    struct thData tdL; 
	tdL= *((struct thData*)arg);
    int rc;

    if (write(tdL.cl,&ok,sizeof(ok)) < 0 ){
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la write() in quit.\n");
    }

    if (ok == 0) { // nu s-a conectat niciun client
        sqlite3_close(dataBase);
    }
    else { // trebuie sa schimb starea de connect

        char verif[200];
        sprintf(verif, "SELECT * FROM Users WHERE username='%s' AND password='%s'", username, password);

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(dataBase, verif, -1, &stmt, 0);

        if (rc != SQLITE_OK) {
            perror("[server] eroare la connect pt verificare de user.\n");
        }

        rc = sqlite3_step(stmt);

        if (rc == SQLITE_ROW) {
        
            sprintf(verif, "UPDATE Users SET conct = 0 WHERE username='%s'", username);

            rc = sqlite3_exec(dataBase, verif, NULL, NULL, NULL);

            if (rc != SQLITE_OK) 
                perror("[server] eroare la actualizarea câmpului conct.\n");
        }
        
        if (rc != SQLITE_OK) {
            perror("[server] eroare la quit()\n");
        }

        sqlite3_close(dataBase);
    }

}

void set_profile(void *arg, char *username, char *password) {

    struct thData tdL; 
	tdL= *((struct thData*)arg);
    char profile[21];
    int rc;

    if (read(tdL.cl,&profile,sizeof(profile)) < 0 ){
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in set_profile().\n");
    }

    int nr;

    if (strcmp(profile, "public") == 0) 
        nr = 1;
    else
        if (strcmp(profile, "private") == 0) 
            nr = 2;   
    else 
        nr = 3;

    char set[200];
    sqlite3_stmt *stmt;

    snprintf(set, sizeof(set), "UPDATE Users SET profile='%s' WHERE username='%s'", profile, username);

    rc = sqlite3_exec(dataBase, set, NULL, NULL, NULL);

    int ok = 1;

    if (rc != SQLITE_OK) {
        perror("[server] eroare la actualizarea campului de profil set_profile()\n");
    }
    else {
        if (write(tdL.cl,&ok,sizeof(ok)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in set_profile().\n");
        }
    }

    char new_command[21];
    if (read(tdL.cl, new_command, sizeof(new_command)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in set_profile().\n");
    }

    size_t length = strcspn(new_command, "\n");
    new_command[length] = '\0';

    if (strcmp(new_command, "quit") == 0) 
        quit((struct thData*)arg, ok, username, password);
    else
        if (strcmp(new_command, "add") == 0)
            add_friend((struct thData*)arg, username, password);
        else    
            if (strcmp(new_command, "send") == 0)
                sendd((struct thData*)arg, username, password);
            else
                if (strcmp(new_command, "makePost") ==0)
                    make_post((struct thData*)arg, username, password);
                else
                    if (strcmp(new_command, "seePosts") ==0)
                        see_posts((struct thData*)arg, username, password);
                    else
                        if (strcmp(new_command, "set profile") ==0)
                            set_profile((struct thData*)arg, username, password);
 
}

int exists_fri(char *username, char *user) {

    //functia returneaza 1 daca cei doi sunt prieteni si 0 in caz contrar
    char search[201];
    snprintf(search, sizeof(search), "SELECT * FROM Friends WHERE (user1 = '%s' AND user2 = '%s') OR (user1 = '%s' AND user2 = '%s')", username, user, user, username);

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(dataBase, search, -1, &stmt, NULL);

    if (rc != SQLITE_OK) {
        perror("[server] eroare la pregatirea interogarii in exists()\n");
        return -1;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) 
        return 1;

    return 0;


}

void add_friend(void *arg, char *username, char *password) {

    struct thData tdL; 
    tdL= *((struct thData*)arg);
    char user[21];
    int rc;

    if (read(tdL.cl,&user,sizeof(user)) < 0 ){
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in add().\n");
    }

    int ok = 1;

    if (!exists(user)) {
        ok = 0;
        if (write(tdL.cl,&ok,sizeof(ok)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la read() in add().\n");
        }

        char user1[21];
        if (read(tdL.cl,&user1,sizeof(user1)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la read() in add() a doua incercare.\n");
        }

        sqlite3_exec(dataBase, "BEGIN TRANSACTION", NULL, NULL, NULL);

        char command[201];
        snprintf(command, sizeof(command), "INSERT INTO Friends (user1, user2) VALUES ('%s', '%s')", username, user1);

        rc = sqlite3_exec(dataBase, command, NULL, NULL, NULL);
        if (rc != SQLITE_OK) {
            perror("[server] eroare la executare db la add_friends()");
        }
        else {
            sqlite3_exec(dataBase, "COMMIT", NULL, NULL, NULL);
        }

    }
    else {
        if (write(tdL.cl,&ok,sizeof(ok)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in add().\n");
        }
        //verific daca sunt deja prieteni

        int ok_prietenie = exists_fri(username, user);
        if (write(tdL.cl,&ok_prietenie,sizeof(ok_prietenie)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in add().\n");
        }

        //daca nu sunt deja prieteni
        if (ok_prietenie == 0) {

            sqlite3_exec(dataBase, "BEGIN TRANSACTION", NULL, NULL, NULL);

            char command[201];
            snprintf(command, sizeof(command), "INSERT INTO Friends (user1, user2) VALUES ('%s', '%s')", username, user);

            rc = sqlite3_exec(dataBase, command, NULL, NULL, NULL);
            if (rc != SQLITE_OK) {
                perror("[server] eroare la executare db la add_friends()");
            }
            else {
                sqlite3_exec(dataBase, "COMMIT", NULL, NULL, NULL);
            }
        }

    }

    char new_command[21];
    if (read(tdL.cl, new_command, sizeof(new_command)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in set_profile().\n");
    }

    size_t length = strcspn(new_command, "\n");
    new_command[length] = '\0';
    ok = 1;

    if (strcmp(new_command, "quit") == 0) 
        quit((struct thData*)arg, ok, username, password);
    else    
        if (strcmp(new_command, "send") == 0)
            sendd((struct thData*)arg, username, password);
        else    
            if (strcmp(new_command, "set profile") == 0)
                set_profile((struct thData*)arg, username, password);
            else
                if (strcmp(new_command, "makePost") ==0)
                    make_post((struct thData*)arg, username, password);
                else
                    if (strcmp(new_command, "seePosts") ==0)
                        see_posts((struct thData*)arg, username, password);
                    else
                        if (strcmp(new_command, "add") ==0)
                            add_friend((struct thData*)arg, username, password);
}

void sendd(void *arg, char *username, char *password) {
    struct thData tdL; 
    tdL = *((struct thData*)arg);

    char prieten[16];
    int prietenie;
    int client_conectat;

    while (1) { //verific daca exista prietenie intre cei doi
        if (read(tdL.cl, &prieten, sizeof(prieten)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la read() in sendd()1.\n");
        }

        char query[256];
        sprintf(query, "SELECT 1 FROM Friends WHERE (user1='%s' AND user2='%s') OR (user1='%s' AND user2='%s')",
                username, prieten, prieten, username);

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(dataBase, query, -1, &stmt, 0);

        if (rc != SQLITE_OK) {
            perror("[server] eroare in la baza de date in sendd()1.\n");
        }

        rc = sqlite3_step(stmt);

        if (rc == SQLITE_ROW) {
            prietenie = 1;
            if (write(tdL.cl, &prietenie, sizeof(prietenie)) < 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("[server] Eroare la write() in sendd()2.\n");
            }
            break;
        }
        else {
            prietenie = 0;
            if (write(tdL.cl, &prietenie, sizeof(prietenie)) < 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("[server] Eroare la write() in sendd()3.\n");
            }
        }
    }

    //verificare daca este conectat
    char query_conectare[256];
    sprintf(query_conectare, "SELECT conct FROM Users WHERE username='%s'", prieten);

    sqlite3_stmt *stmt_conectare;
    int rc = sqlite3_prepare_v2(dataBase, query_conectare, -1, &stmt_conectare, 0);

    if (rc != SQLITE_OK) {
        perror("[server] eroare in la baza de date in sendd()4.\n");
    }

    rc = sqlite3_step(stmt_conectare);

    if (rc == SQLITE_ROW) {
        client_conectat = sqlite3_column_int(stmt_conectare, 0);
        if (write(tdL.cl, &client_conectat, sizeof(client_conectat)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in sendd()5.\n");
        }
    }

    //trimitere mesaje din cei doi utilizatori

    char sendQuery[512];
    sprintf(sendQuery, "SELECT message FROM Messages WHERE (user1='%s' and user2='%s') or (user1='%s' and user2='%s')", username, prieten, prieten, username);

    sqlite3_stmt *sendSTMT;
    rc = sqlite3_prepare_v2(dataBase, sendQuery, -1, &sendSTMT, 0);

    if (rc != SQLITE_OK) {
        perror("[server] eroare la baza de date in seePosts()1.\n");
    }

    while (sqlite3_step(sendSTMT) == SQLITE_ROW) {
        char postarea[201];
        strcpy(postarea, sqlite3_column_text(sendSTMT, 0));

        if (write(tdL.cl, &postarea, sizeof(postarea)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in sendd()4.\n");
        }
    }

    char end_while[] = "END";
    if (write(tdL.cl, &end_while, sizeof(end_while)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la write() in sendd()4.\n");
    }

    //////////////////////////////////////////////////////
    while (1) {
        char message[500];
        ssize_t bytes_received;

        while ((bytes_received = read(tdL.cl, message, sizeof(message))) >= 0) {
            message[bytes_received] = '\0';

            if (strcmp(message, "quit\n") == 0) 
                break;

            //adaugare mesaj in baza de date

            char command[601];
            snprintf(command, sizeof(command), "INSERT INTO Messages (user1, user2, message) VALUES ('%s', '%s', '%s')", username, prieten, message);

            int rc = sqlite3_exec(dataBase, command, NULL, NULL, NULL);
            if (rc != SQLITE_OK) {
                perror("[server] eroare la executare db la sendd()");
            }
            else {
                sqlite3_exec(dataBase, "COMMIT", NULL, NULL, NULL);
            }

            //trimitere mesaj la user

            for (int i = 0; i < nr_clienti; ++i) {
                if (vector_clienti[i].id_client != -1 && vector_clienti[i].id_client != tdL.cl && strcmp(prieten, vector_clienti[i].nume_client) == 0) {
                    if (write(vector_clienti[i].id_client, message, sizeof(message)) < 0) {
                        perror("[server] Eroare la write() catre client.\n");
                    }
                }
            }

        }
        break;
    }

    //
    close(tdL.cl);
    free(arg);
    pthread_exit(NULL);
    //

    //comanda noua

    char new_command[21];
    if (read(tdL.cl, new_command, sizeof(new_command)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in set_profile().\n");
    }

    size_t length = strcspn(new_command, "\n");
    new_command[length] = '\0';
    int ok = 1;

    if (strcmp(new_command, "quit") == 0) 
        quit((struct thData*)arg, ok, username, password);
    else    
        if (strcmp(new_command, "add") == 0)
            add_friend((struct thData*)arg, username, password);
        else    
            if (strcmp(new_command, "set profile") == 0)
                set_profile((struct thData*)arg, username, password);
            else
                if (strcmp(new_command, "makePost") == 0)
                    make_post((struct thData*)arg, username, password);
                else
                    if (strcmp(new_command, "seePosts") == 0)
                        see_posts((struct thData*)arg, username, password);
                    else    
                        if (strcmp(new_command, "send") == 0)
                            sendd((struct thData*)arg, username, password);

    // Închideți conexiunea și eliberați resursele alocate.
    //close(tdL.cl);
    //free(arg);

    //pthread_exit(NULL);

}

void make_post(void *arg, char *username, char *password) {

    struct thData tdL; 
	tdL= *((struct thData*)arg);
    char post[201];
    int rc;

    if (read(tdL.cl,&post,sizeof(post)) < 0 ){
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in set_profile().\n");
    }

    int postare_reusita;

    char post_db[256];
    sprintf(post_db, "INSERT INTO Posts(user, post) VALUES ('%s', '%s')", username, post);

    rc = sqlite3_exec(dataBase, post_db, NULL, NULL, NULL);

    if (rc != SQLITE_OK) {
        perror("[server] eroare la db make_post()\n");
        postare_reusita = 0;
        if (write(tdL.cl,&postare_reusita,sizeof(postare_reusita)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in make_post().\n");
        }
    }
    else {
        postare_reusita = 1;
        if (write(tdL.cl,&postare_reusita,sizeof(postare_reusita)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in make_post().\n");
        }
    }

    char new_command[21];
    if (read(tdL.cl, new_command, sizeof(new_command)) < 0) {
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in set_profile().\n");
    }

    size_t length = strcspn(new_command, "\n");
    new_command[length] = '\0';
    int ok = 1;

    if (strcmp(new_command, "quit") == 0) 
        quit((struct thData*)arg, ok, username, password);
    else    
        if (strcmp(new_command, "send") == 0)
            sendd((struct thData*)arg, username, password);
        else    
            if (strcmp(new_command, "set profile") == 0)
                set_profile((struct thData*)arg, username, password);
            else
                if (strcmp(new_command, "makePost") ==0)
                    make_post((struct thData*)arg, username, password);
                else
                    if (strcmp(new_command, "add") ==0)
                        make_post((struct thData*)arg, username, password);
                    else
                        if (strcmp(new_command, "seePosts") ==0)
                            see_posts((struct thData*)arg, username, password);

}

void see_posts(void *arg, char *username, char *password) {

    struct thData tdL; 
	tdL= *((struct thData*)arg);
    char user[201];
    int rc;
    int prietenie;
    int profil;

    if (read(tdL.cl,&user,sizeof(user)) <0 ){
        printf("[Thread %d]\n", tdL.idThread);
        perror("[server] Eroare la read() in set_profile().\n");
    }

    int ok = 0;
    if (!exists(user)) { //daca nu exista utilizatorul
        if (write(tdL.cl,&ok,sizeof(ok)) <0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in see_posts().\n");
        }
    }
    else {
        ok = 1;
        if (write(tdL.cl,&ok,sizeof(ok)) < 0 ){
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la write() in see_posts().\n");
        }
    }

    if (ok == 0) { //utilizatorul nu exista
        
        char new_command[21];
        if (read(tdL.cl, new_command, sizeof(new_command)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la read() in set_profile().\n");
        }

        size_t length = strcspn(new_command, "\n");
        new_command[length] = '\0';
        int ok1 = 1;

        if (strcmp(new_command, "quit") == 0) 
            quit((struct thData*)arg, ok1, username, password);
        else    
            if (strcmp(new_command, "send") == 0)
                sendd((struct thData*)arg, username, password);
            else    
                if (strcmp(new_command, "set profile") == 0)
                    set_profile((struct thData*)arg, username, password);
                else
                    if (strcmp(new_command, "makePost") ==0)
                        make_post((struct thData*)arg, username, password);
                    else
                        if (strcmp(new_command, "add") ==0)
                            make_post((struct thData*)arg, username, password);
                        else
                            if (strcmp(new_command, "seePosts") ==0)
                                see_posts((struct thData*)arg, username, password);
    }
    else { // exista utilizator si verific prietenia
        
        char query[556];
        sprintf(query, "SELECT 1 FROM Friends WHERE (user1='%s' AND user2='%s') OR (user1='%s' AND user2='%s')",
                username, user, user, username);

        sqlite3_stmt *stmt;
        int rc = sqlite3_prepare_v2(dataBase, query, -1, &stmt, 0);

        if (rc != SQLITE_OK) {
            perror("[server] eroare in la baza de date in seePosts()1.\n");
        }

        rc = sqlite3_step(stmt);

        if (rc == SQLITE_ROW) {
            prietenie = 1;
            if (write(tdL.cl, &prietenie, sizeof(prietenie)) < 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("[server] Eroare la write() in sendd()2.\n");
            }
        }
        else {
            prietenie = 0;
            if (write(tdL.cl, &prietenie, sizeof(prietenie)) < 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("[server] Eroare la write() in sendd()3.\n");
            }
        }
    }

    //daca sunt prieteni afisez postarile
    if (prietenie == 1) {

        char postQuery[512];
        sprintf(postQuery, "SELECT post FROM Posts WHERE user='%s'", user);

        sqlite3_stmt *postSTMT;
        int rc = sqlite3_prepare_v2(dataBase, postQuery, -1, &postSTMT, 0);

        if (rc != SQLITE_OK) {
            perror("[server] eroare la baza de date in seePosts()1.\n");
        }

        while (sqlite3_step(postSTMT) == SQLITE_ROW) {
            char postarea[201];
            strcpy(postarea, sqlite3_column_text(postSTMT, 0));

            if (write(tdL.cl, &postarea, sizeof(postarea)) < 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("[server] Eroare la write() in sendd()4.\n");
            }
        }

        char end_while[] = "END";
        write(tdL.cl, &end_while, sizeof(end_while));

        sqlite3_finalize(postSTMT);

        char new_command[21];
        if (read(tdL.cl, new_command, sizeof(new_command)) < 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("[server] Eroare la read() in set_profile().\n");
        }

        size_t length = strcspn(new_command, "\n");
        new_command[length] = '\0';
        int ok1 = 1;

        if (strcmp(new_command, "quit") == 0) 
            quit((struct thData*)arg, ok1, username, password);
        else    
            if (strcmp(new_command, "send") == 0)
                sendd((struct thData*)arg, username, password);
            else    
                if (strcmp(new_command, "set profile") == 0)
                    set_profile((struct thData*)arg, username, password);
                else
                    if (strcmp(new_command, "makePost") ==0)
                        make_post((struct thData*)arg, username, password);
                    else
                        if (strcmp(new_command, "add") ==0)
                            make_post((struct thData*)arg, username, password);
                        else
                            if (strcmp(new_command, "seePosts") ==0)
                                see_posts((struct thData*)arg, username, password);
    }
    else { //daca nu sunt prieteni verific daca profilul este public sau privat

        char profileQuery[512];
        sprintf(profileQuery, "SELECT profile FROM Users WHERE username='%s'", user);

        sqlite3_stmt *profileStmt;
        rc = sqlite3_prepare_v2(dataBase, profileQuery, -1, &profileStmt, 0);

        if (rc != SQLITE_OK) {
            perror("[server] eroare la baza de date in seePosts()1.\n");
        }

        rc = sqlite3_step(profileStmt);

        if (rc == SQLITE_ROW) {
            char PROFILE[9];
            strcpy(PROFILE, sqlite3_column_text(profileStmt, 0));

            if (strcmp(PROFILE, "public") == 0)
                profil = 1;
            else
                profil = 0;

            if (write(tdL.cl, &profil, sizeof(profil)) < 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("[server] Eroare la write() in seePosts()10.\n");
            }
        }

        //daca profilul este public
        if (profil == 1) {

            char postQuery[512];
            sprintf(postQuery, "SELECT post FROM Posts WHERE user='%s'", user);

            sqlite3_stmt *postSTMT;
            int rc = sqlite3_prepare_v2(dataBase, postQuery, -1, &postSTMT, 0);

            if (rc != SQLITE_OK) {
                perror("[server] eroare la baza de date in seePosts()1.\n");
            }

            while (sqlite3_step(postSTMT) == SQLITE_ROW) {
                char postarea[201];
                strcpy(postarea, sqlite3_column_text(postSTMT, 0));

                if (write(tdL.cl, &postarea, sizeof(postarea)) < 0) {
                    printf("[Thread %d]\n", tdL.idThread);
                    perror("[server] Eroare la write() in sendd()4.\n");
                }
            }

            char end_while[] = "END";
            write(tdL.cl, &end_while, sizeof(end_while));

            sqlite3_finalize(postSTMT);

            char new_command[21];
            if (read(tdL.cl, new_command, sizeof(new_command)) < 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("[server] Eroare la read() in set_profile().\n");
            }

            size_t length = strcspn(new_command, "\n");
            new_command[length] = '\0';
            int ok1 = 1;

            if (strcmp(new_command, "quit") == 0) 
                quit((struct thData*)arg, ok1, username, password);
            else    
                if (strcmp(new_command, "send") == 0)
                    sendd((struct thData*)arg, username, password);
                else    
                    if (strcmp(new_command, "set profile") == 0)
                        set_profile((struct thData*)arg, username, password);
                    else
                        if (strcmp(new_command, "makePost") ==0)
                            make_post((struct thData*)arg, username, password);
                        else
                            if (strcmp(new_command, "add") ==0)
                                make_post((struct thData*)arg, username, password);
                            else
                                if (strcmp(new_command, "seePosts") ==0)
                                    see_posts((struct thData*)arg, username, password);
            
        }
        else {//daca profilul este privat

            char new_command[21];
            if (read(tdL.cl, new_command, sizeof(new_command)) < 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("[server] Eroare la read() in set_profile().\n");
            }

            size_t length = strcspn(new_command, "\n");
            new_command[length] = '\0';
            int ok1 = 1;

            if (strcmp(new_command, "quit") == 0) 
                quit((struct thData*)arg, ok1, username, password);
            else    
                if (strcmp(new_command, "send") == 0)
                    sendd((struct thData*)arg, username, password);
                else    
                    if (strcmp(new_command, "set profile") == 0)
                        set_profile((struct thData*)arg, username, password);
                    else
                        if (strcmp(new_command, "makePost") ==0)
                            make_post((struct thData*)arg, username, password);
                        else
                            if (strcmp(new_command, "add") ==0)
                                make_post((struct thData*)arg, username, password);
                            else
                                if (strcmp(new_command, "seePosts") ==0)
                                    see_posts((struct thData*)arg, username, password);

        }


    }

}