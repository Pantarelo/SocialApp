#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>

extern int errno;
int port;
int quit_flag;

void registerr(int sd) {

    printf("Scrie un username si o parola: ");
    //fflush(stdout);

    char mesaj[100];
    fgets(mesaj, sizeof(mesaj), stdin);

    if (write (sd,&mesaj,sizeof(mesaj)) < 0) {
        perror ("[client]Eroare la write() pentru register.\n");
        return errno;
    }

    int verificareRegister;

    if (read (sd,&verificareRegister,sizeof(verificareRegister)) < 0) {
        perror ("[client]Eroare la write() pentru register.\n");
        return;
    }

    if (verificareRegister == 0) {
        printf("Inregistrare esuata\n");
        //fflush(stdout);
    }
    else {
        printf("Inregistrare reusita\n");
        //fflush(stdout);
    }

    memset(mesaj, 0, sizeof(mesaj));

    printf("[client] Introduceti urmatoarea comanda(connect, quit): ");
    //fflush(stdout);

    fgets(mesaj, sizeof(mesaj), stdin);
    
    size_t length = strcspn(mesaj, "\n");
    mesaj[length] = '\0';

    if (write (sd,&mesaj,sizeof(mesaj)) < 0) {
        perror ("[client]Eroare la write() pentru register.\n");
        return 0;
    }

    if (strcmp(mesaj, "connect") == 0)
        connectt(sd);
    else
        if (strcmp(mesaj, "quit") == 0)
            quit(sd);
    else
        printf("[client] comanda gresita");

}

void connectt(int sd) {

    char mesaj[100];

    printf("Introduceti username-ul si parola de la cont: ");
    //fflush(stdout);
    fgets(mesaj, sizeof(mesaj), stdin);

    if (write (sd,&mesaj,sizeof(mesaj)) < 0) {
        perror ("[client]Eroare la write() pentru connect.\n");
        return;
    }

    int nr;
    if (read (sd,&nr,sizeof(nr)) < 0) {
        perror ("[client]Eroare la read() pentru connect.\n");
        return;
    }

    if (nr == 1)
        printf("Autentificare reusita.\n");
    else
        printf("Autentificare nereusita.\n");

    int admin;
    if (read (sd,&admin,sizeof(admin)) < 0) {
        perror ("[client]Eroare la read() pentru connect_admin.\n");
        return;
    }

    if (admin == 1) {
        adminn(sd);
    }
    else {
        printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");
        memset(mesaj, 0, sizeof(mesaj));

        fgets(mesaj, sizeof(mesaj), stdin);
        
        size_t length = strcspn(mesaj, "\n");
        mesaj[length] = '\0';

        if (write (sd,&mesaj,sizeof(mesaj)) < 0) { //trimit la server quit sau alta comanda.
            perror ("[client]Eroare la write() pentru connect - urmatoarea comanda.\n");
            return;
        }

        if (strcmp(mesaj, "set profile") == 0) 
            set_profile(sd);
        else
            if (strcmp(mesaj, "quit") == 0)
                quit(sd);
            else
                if (strcmp(mesaj, "add") == 0)
                    add_friend(sd);
                else
                    if (strcmp(mesaj, "send") == 0)
                        sendd(sd);
                    else
                        if (strcmp(mesaj, "makePost") == 0)
                            make_post(sd);
                        else
                            if (strcmp(mesaj, "seePosts") == 0)
                                see_posts(sd);
    }

    
}

void adminn(int sd) {

    char mesaj[9];

    printf("[admin]Introduceti comanda(delete/quit): ");

    fgets(mesaj, sizeof(mesaj), stdin);
        
    size_t length = strcspn(mesaj, "\n");
    mesaj[length] = '\0';

    if (write (sd,&mesaj,sizeof(mesaj)) < 0) { 
        perror ("[client]Eroare la write() pentru admin.\n");
        return;
    }

    if (strcmp(mesaj, "quit") == 0) 
        quit(sd);
    else 
        if (strcmp(mesaj, "delete") == 0)
            delete(sd);
        else {
            printf("[admin] comanda gresita.\n");
            adminn(sd);
        }

}

void delete(int sd) {

    printf("[admin] Introdu utilizatorul pe care doresti sa l stergi: ");

    char user[30];

    fgets(user, sizeof(user), stdin);
        
    size_t length = strcspn(user, "\n");
    user[length] = '\0';

    if (write (sd,&user,sizeof(user)) < 0) { 
        perror ("[client]Eroare la write() pentru delete().\n");
        return;
    }

    int exista;
    if (read (sd,&exista,sizeof(exista)) < 0) { 
        perror ("[client]Eroare la write() pentru delete().\n");
        return;
    }

    if (exista == 0) {
        printf("[admin] utilizatorul nu exista.\n");
        adminn(sd);
    }
    else {
        int ok;
        if (read (sd,&ok,sizeof(ok)) < 0) { 
            perror ("[client]Eroare la write() pentru delete().\n");
            return;
        }

        if (ok == 1)
            printf("[admin] %s a fost sters cu succes\n");
            adminn(sd);

    }
    
}

void quit(int sd) {
    //printf("assssssssssssssssss");

    int ok;

    if (read (sd,&ok,sizeof(ok)) < 0) {
        perror ("[client]Eroare la read() pentru quit.\n");
        //return errno;
    }

    if (ok == 1) {
        printf("[client] S-a deconectat clientul...\n");
        close(sd);
        exit(0);
    }
    else {
        printf("[client] S-a deconectat clientul ...\n");
        close(sd);
        return;
    }
}

void set_profile(int sd) {

    printf("[client]Introdu tipul de profil(public/private): ");

    char profile[21];
    fgets(profile, sizeof(profile), stdin);

    size_t length = strcspn(profile, "\n");
    profile[length] = '\0';

    if (write (sd,&profile,sizeof(profile)) < 0) { 
        perror ("[client]Eroare la write() pentru set_profile().\n");
        return;
    }

    int ok;

    if (read (sd,&ok,sizeof(ok)) < 0) { 
        perror ("[client]Eroare la read() pentru set_profile().\n");
        return;
    }

    if (ok == 1) 
        printf("[client]Actualizare profil realizata cu succes.\n");

    printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");

    char new_command[21];
    fgets(new_command, sizeof(new_command), stdin);

    size_t len = strcspn(new_command, "\n");
    new_command[len] = '\0';

    if (write(sd, &new_command, sizeof(new_command)) < 0) {
        perror ("[client]Eroare la write() pentru set_profile()2.\n");
        return;
    }

    if (strcmp(new_command, "quit") == 0)
        quit(sd);
    else
        if (strcmp(new_command, "add") == 0)
            add_friend(sd);
        else 
            if (strcmp(new_command, "send") == 0)
                sendd(sd);
            else
                if (strcmp(new_command, "makePost") == 0)
                    make_post(sd);
                else
                    if (strcmp(new_command, "seePosts") == 0)
                        see_posts(sd);
                    else
                        if (strcmp(new_command, "set profile") == 0)
                            set_profile(sd);

}

void add_friend(int sd) {

    printf("[client]Introdu utilizatorul pe care doresti sa-l adaugi: ");

    char user[21];
    fgets(user, sizeof(user), stdin);

    size_t length = strcspn(user, "\n");
    user[length] = '\0';

    if (write (sd,&user,sizeof(user)) < 0) { 
        perror ("[client]Eroare la write() pentru add().\n");
        return;
    }

    int ok;

    if (read (sd,&ok,sizeof(ok)) < 0) { 
        perror ("[client]Eroare la read() pentru add().\n");
        return;
    }

    if (ok == 0) {

        printf("[client] Utilizatorul nu exista, incercati din nou: ");
        char user1[21];
        fgets(user1, sizeof(user1), stdin);

        size_t len = strcspn(user1, "\n");
        user1[len] = '\0';

        if (write (sd,&user1,sizeof(user1)) < 0) { 
            perror ("[client]Eroare la write() pentru add() pentru a doua incercare.\n");
            return;
        }
        printf("[client] Utilizator adaugat cu succes.\n");
    }
    else {

        //verific daca sunt prieteni
        int ok_prietenie;
        if (read (sd,&ok_prietenie,sizeof(ok_prietenie)) < 0) { 
            perror ("[client]Eroare la read() pentru add().\n");
            return;
        }

        if (ok_prietenie == 1) {
            printf("[client] Acesti 2 utilizatori sunt deja prieteni.\n");
        }
        else 
            printf("[client] Utilizator adaugat cu succes.\n");
    }

    printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");

    char new_command[21];
    fgets(new_command, sizeof(new_command), stdin);

    size_t len = strcspn(new_command, "\n");
    new_command[len] = '\0';

    if (write(sd, &new_command, sizeof(new_command)) < 0) {
        perror ("[client]Eroare la write() pentru set_profile()2.\n");
        return;
    }

    if (strcmp(new_command, "quit") == 0)
        quit(sd);
    else
        if (strcmp(new_command, "send") == 0)
            sendd(sd);
        else
            if (strcmp(new_command, "set profile") == 0) 
                set_profile(sd);
            else
                if (strcmp(new_command, "makePost") == 0)
                    make_post(sd);
                else
                    if (strcmp(new_command, "seePosts") == 0)
                        see_posts(sd);
                    else
                        if (strcmp(new_command, "add") == 0)
                            add_friend(sd);

}

void *receive_messages(void *arg) {

    int client_socket = *((int *)arg);
    char buffer[500];
    ssize_t bytes_received;

    while ((bytes_received = read(client_socket, buffer, sizeof(buffer))) > 0 && !quit_flag) {

        buffer[bytes_received] = '\0';
        printf("\nPrimit: %s", buffer);

    }

    //pthread_exit(NULL);

    //pthread_exit(NULL);
    //
    // pthread_t receive_thread = pthread_self();
    // pthread_join(receive_thread, NULL);
    // return NULL;

}

void sendd(int sd) {

    quit_flag = 0;

    int client_conectat;
    int exista_prieten = 0;
    char prieten[16];

    while (!exista_prieten) { ////verific daca exista prietenie intre cei doi
        printf("[client] Introdu prietenul caruia doresti sa i trimiti mesaj: ");
        fgets(prieten, sizeof(prieten), stdin);
        
        size_t len = strcspn(prieten, "\n");
        prieten[len] = '\0';

        if (write(sd, &prieten, sizeof(prieten)) < 0) {
            perror ("[client]Eroare la write1() pentru sendd().\n");
            return;
        }

        if (read(sd, &exista_prieten, sizeof(exista_prieten)) < 0) {
            perror ("[client]Eroare la read1() pentru sendd().\n");
            return;
        }
    }

    if (read(sd, &client_conectat, sizeof(client_conectat)) < 0) {
        perror ("[client]Eroare la read1() pentru sendd().\n");
        return;
    }

    // afisare mesaje intre prieteni
    
    char postarea[201];
    ssize_t bb;

    while ((bb = read(sd, &postarea, sizeof(postarea))) > 0) {
        if (strcmp(postarea, "END") == 0)
            break;
        printf("[client] Mesaj: %s\n", postarea);
    }

    if (bb < 0) {
        perror ("[client]Eroare la read() pentru seePosts()4.\n");
        return;
    }
    
    /////////////////////////////////////
    pthread_t receive_thread;
    pthread_create(&receive_thread, NULL, receive_messages, (void *)&sd);

    char message[500];
    
    while (1) {
        printf("Enter message: ");
        fgets(message, sizeof(message), stdin);

        // Trimitere mesaj la server
        if (write(sd, &message, sizeof(message)) < 0) {
            perror ("[client]Eroare la write() pentru sendd().\n");
            return;
        }

        if (strcmp(message, "quit\n") == 0) {
            quit_flag = 1;
            break;
        }
    }

    //comanda noua
    printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");

    char new_command[21];
    fgets(new_command, sizeof(new_command), stdin);

    size_t len = strcspn(new_command, "\n");
    new_command[len] = '\0';

    if (write(sd, &new_command, sizeof(new_command)) < 0) {
        perror ("[client]Eroare la write() pentru set_profile()2.\n");
        return;
    }

    if (strcmp(new_command, "quit") == 0)
        quit(sd);
    else
        if (strcmp(new_command, "add") == 0)
            add_friend(sd);
        else
            if (strcmp(new_command, "set profile") == 0) 
                set_profile(sd);
            else
                if (strcmp(new_command, "makePost") == 0)
                    make_post(sd);
                else
                    if (strcmp(new_command, "seePosts") == 0)
                        see_posts(sd);
                    else
                        if (strcmp(new_command, "send") == 0)
                            sendd(sd);

}

void make_post(int sd) {

    printf("[client] Introdu postarea pe care doresti sa o faci: \n");

    char post[201];
    fgets(post, sizeof(post), stdin);

    size_t length = strcspn(post, "\n");
    post[length] = '\0';

    if (write (sd,&post,sizeof(post)) < 0) { 
        perror ("[client]Eroare la write() pentru make_post().\n");
        return;
    }

    int postare_reusita;

    if (read (sd,&postare_reusita,sizeof(postare_reusita)) < 0) { 
        perror ("[client]Eroare la read() pentru make_post().\n");
        return;
    }

    if (postare_reusita == 0)
        printf("[client] Postarea nu a fost publicata\n");
    else
        printf("[client] Postarea a fost publicata\n");


    printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");

    char new_command[21];
    fgets(new_command, sizeof(new_command), stdin);

    size_t len = strcspn(new_command, "\n");
    new_command[len] = '\0';

    if (write(sd, &new_command, sizeof(new_command)) < 0) {
        perror ("[client]Eroare la write() pentru set_profile()2.\n");
        return;
    }

    if (strcmp(new_command, "quit") == 0)
        quit(sd);
    else
        if (strcmp(new_command, "add") == 0)
            add_friend(sd);
        else 
            if (strcmp(new_command, "send") == 0)
                sendd(sd);
            else
                if (strcmp(new_command, "makePost") == 0)
                    make_post(sd);
                else
                    if (strcmp(new_command, "set profile") == 0)
                        set_profile(sd);
                    else
                        if (strcmp(new_command, "seePosts") == 0)
                            see_posts(sd);
}

void see_posts(int sd) {

    int prietenie;
    int profil;

    printf("[client] Introdu uitilizatorul caruia doresti sa i vezi postarile: ");

    char user[201];
    fgets(user, sizeof(user), stdin);

    size_t length = strcspn(user, "\n");
    user[length] = '\0';

    if (write (sd,&user,sizeof(user)) < 0) { 
        perror ("[client]Eroare la write() pentru see_posts().\n");
        return;
    }

    int ok;
    if (read (sd,&ok,sizeof(ok)) < 0) { 
        perror ("[client]Eroare la read() pentru see_posts().\n");
        return;
    }

    if (ok == 0) { //utilizatorul nu exista
        printf("[client] Utilizatorul nu exista\n");
        printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");

        char new_command[21];
        fgets(new_command, sizeof(new_command), stdin);

        size_t len = strcspn(new_command, "\n");
        new_command[len] = '\0';

        if (write(sd, &new_command, sizeof(new_command)) < 0) {
            perror ("[client]Eroare la write() pentru seePosts()1.\n");
            return;
        }

        if (strcmp(new_command, "quit") == 0)
            quit(sd);
        else
            if (strcmp(new_command, "add") == 0)
                add_friend(sd);
            else 
                if (strcmp(new_command, "send") == 0)
                    sendd(sd);
                else
                    if (strcmp(new_command, "makePost") == 0)
                        make_post(sd);
                    else
                        if (strcmp(new_command, "set profile") == 0)
                            set_profile(sd);
                        else
                            if (strcmp(new_command, "seePosts") == 0)
                                see_posts(sd);
    }
    else { //exista utilizator si verific prietenia
        
        if (read(sd, &prietenie, sizeof(prietenie)) < 0) {
            perror ("[client]Eroare la read() pentru seePosts().\n");
            return;
        }

    }

    //daca sunt prieteni afisez postarile
    if (prietenie == 1) {

        char postarea[201];
        ssize_t bb;

        while ((bb = read(sd, &postarea, sizeof(postarea))) > 0) {
            if (strcmp(postarea, "END") == 0)
                break;
            printf("[client] Post: %s\n", postarea);
        }

        if (bb < 0) {
            perror ("[client]Eroare la read() pentru seePosts()4.\n");
            return;
        }

        printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");

        char new_command[21];
        fgets(new_command, sizeof(new_command), stdin);

        size_t len = strcspn(new_command, "\n");
        new_command[len] = '\0';

        if (write(sd, &new_command, sizeof(new_command)) < 0) {
            perror ("[client]Eroare la write() pentru seePosts()1.\n");
            return;
        }

        if (strcmp(new_command, "quit") == 0)
            quit(sd);
        else
            if (strcmp(new_command, "add") == 0)
                add_friend(sd);
            else 
                if (strcmp(new_command, "send") == 0)
                    sendd(sd);
                else
                    if (strcmp(new_command, "makePost") == 0)
                        make_post(sd);
                    else
                        if (strcmp(new_command, "set profile") == 0)
                            set_profile(sd);
                        else
                            if (strcmp(new_command, "seePosts") == 0)
                                see_posts(sd);

    }
    else { //daca nu sunt prieteni verific daca profilul este public sau privat
        
        if (read(sd, &profil, sizeof(profil)) < 0) {
            perror ("[client]Eroare la read() pentru seePosts()1.\n");
            return;
        }

        //daca profilul este public

        if (profil == 1) {

            char postarea[201];
            ssize_t bb;

            while ((bb = read(sd, &postarea, sizeof(postarea))) > 0) {
                if (strcmp(postarea, "END") == 0)
                    break;
                printf("[client] Post: %s\n", postarea);
            }

            if (bb < 0) {
                perror ("[client]Eroare la read() pentru seePosts()4.\n");
                return;
            }

            printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");

            char new_command[21];
            fgets(new_command, sizeof(new_command), stdin);

            size_t len = strcspn(new_command, "\n");
            new_command[len] = '\0';

            if (write(sd, &new_command, sizeof(new_command)) < 0) {
                perror ("[client]Eroare la write() pentru seePosts()1.\n");
                return;
            }

            if (strcmp(new_command, "quit") == 0)
                quit(sd);
            else
                if (strcmp(new_command, "add") == 0)
                    add_friend(sd);
                else 
                    if (strcmp(new_command, "send") == 0)
                        sendd(sd);
                    else
                        if (strcmp(new_command, "makePost") == 0)
                            make_post(sd);
                        else
                            if (strcmp(new_command, "set profile") == 0)
                                set_profile(sd);
                            else
                                if (strcmp(new_command, "seePosts") == 0)
                                    see_posts(sd);

        }
        else { //daca profilul este privat

            printf("[client] Nu ai acces la postarile acestui utilizator.\n");
            printf("Introduceti urmatoarea comanda(quit, set profile, add, seePosts, makePost, send): ");

            char new_command[21];
            fgets(new_command, sizeof(new_command), stdin);

            size_t len = strcspn(new_command, "\n");
            new_command[len] = '\0';

            if (write(sd, &new_command, sizeof(new_command)) < 0) {
                perror ("[client]Eroare la write() pentru seePosts()1.\n");
                return;
            }

            if (strcmp(new_command, "quit") == 0)
                quit(sd);
            else
                if (strcmp(new_command, "add") == 0)
                    add_friend(sd);
                else 
                    if (strcmp(new_command, "send") == 0)
                        sendd(sd);
                    else
                        if (strcmp(new_command, "makePost") == 0)
                            make_post(sd);
                        else
                            if (strcmp(new_command, "set profile") == 0)
                                set_profile(sd);
                            else
                                if (strcmp(new_command, "seePosts") == 0)
                                    see_posts(sd);

        }

    }
    
}

int main (int argc, char *argv[])
{
    int sd;			// descriptorul de socket
    struct sockaddr_in server;	// structura folosita pentru conectare 
    char msg[100];
    char buf[10];

    if (argc != 3) {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi (argv[2]);

    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1){
        perror ("Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (port);
    
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1) {
        perror ("[client]Eroare la connect().\n");
        return errno;
    }

    //while (1) {

        printf ("[client]Introduceti o comanda(register, connect, quit): ");
        fflush (stdout);

        fgets(msg, sizeof(msg), stdin);
        
        size_t length = strcspn(msg, "\n");
        msg[length] = '\0';

        if (write (sd,&msg,sizeof(msg)) <= 0) {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
        }

        int nr;

        if (read (sd, &nr,sizeof(int)) < 0) {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
        }

        if (nr == 1) 
            registerr(sd);
        else
            if (nr == 2)
                connectt(sd);
            else {
                quit(sd);
            }

    //}

    //close (sd);
    return 0;
}