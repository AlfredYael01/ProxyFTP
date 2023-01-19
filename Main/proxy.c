#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>
#include  <unistd.h>
#include  <stdbool.h>
#include "./simpleSocketAPI.h"


#define SERVADDR "127.0.0.1"        // Définition de l'adresse IP d'écoute
#define SERVPORT "0"                // Définition du port d'écoute, si 0 port choisi dynamiquement
#define LISTENLEN 1                 // Taille de la file des demandes de connexion
#define MAXBUFFERLEN 1024           // Taille du tampon pour les échanges de données
#define MAXHOSTLEN 64               // Taille d'un nom de machine
#define MAXPORTLEN 64               // Taille d'un numéro de port


int main(){
    int ecode;                       // Code retour des fonctions
    char serverAddr[MAXHOSTLEN];     // Adresse du serveur
    char serverPort[MAXPORTLEN];     // Port du server
    int descSockRDV;                 // Descripteur de socket de rendez-vous
    int descSockCOM;                 // Descripteur de socket de communication
    struct addrinfo hints;           // Contrôle la fonction getaddrinfo
    struct addrinfo *res;            // Contient le résultat de la fonction getaddrinfo
    struct sockaddr_storage myinfo;  // Informations sur la connexion de RDV
    struct sockaddr_storage from;    // Informations sur le client connecté
    socklen_t len;                   // Variable utilisée pour stocker les 
				                     // longueurs des structures de socket
    char buffer[MAXBUFFERLEN];       // Tampon de communication entre le client et le serveur
    
    // Initialisation de la socket de RDV IPv4/TCP
    descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
    if (descSockRDV == -1) {
         perror("Erreur création socket RDV\n");
         exit(2);
    }
    // Publication de la socket au niveau du système
    // Assignation d'une adresse IP et un numéro de port
    // Mise à zéro de hints
    memset(&hints, 0, sizeof(hints));
    // Initialisation de hints
    hints.ai_flags = AI_PASSIVE;      // mode serveur, nous allons utiliser la fonction bind
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_family = AF_INET;        // seules les adresses IPv4 seront présentées par 
				                      // la fonction getaddrinfo

     // Récupération des informations du serveur
     ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
     if (ecode) {
         fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
         exit(1);
     }
     // Publication de la socket
     ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
     if (ecode == -1) {
         perror("Erreur liaison de la socket de RDV");
         exit(3);
     }
     // Nous n'avons plus besoin de cette liste chainée addrinfo
     freeaddrinfo(res);

     // Récuppération du nom de la machine et du numéro de port pour affichage à l'écran
     len=sizeof(struct sockaddr_storage);
     ecode=getsockname(descSockRDV, (struct sockaddr *) &myinfo, &len);
     if (ecode == -1)
     {
         perror("SERVEUR: getsockname");
         exit(4);
     }
     ecode = getnameinfo((struct sockaddr*)&myinfo, sizeof(myinfo), serverAddr,MAXHOSTLEN, 
                         serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV);
     if (ecode != 0) {
             fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
             exit(4);
     }
     printf("L'adresse d'ecoute est: %s\n", serverAddr);
     printf("Le port d'ecoute est: %s\n", serverPort);

     // Definition de la taille du tampon contenant les demandes de connexion
     ecode = listen(descSockRDV, LISTENLEN);
     if (ecode == -1) {
         perror("Erreur initialisation buffer d'écoute");
         exit(5);
     }

	len = sizeof(struct sockaddr_storage);
     // Attente connexion du client
     // Lorsque demande de connexion, creation d'une socket de communication avec le client
     descSockCOM = accept(descSockRDV, (struct sockaddr *) &from, &len);
     if (descSockCOM == -1){
         perror("Erreur accept\n");
         exit(6);
     }
    // Echange de données avec le client connecté

    /*****
     * Testez de mettre 220 devant BLABLABLA ...
     * **/
    strcpy(buffer, "220 BLABLABLA\r\n");
    write(descSockCOM, buffer, strlen(buffer));
    
    /*******
     * 
     * A vous de continuer !
     * 
     * *****/
    //recuperation du user

    ecode=read(descSockCOM,buffer,MAXBUFFERLEN-1);
    if (ecode==-1){
    	perror("Problème lecture d'un socket");
    	exit(2);}
    buffer[ecode]='\0';
    printf("Recu du client %s\n",buffer);
    

	// USER anonymous@ftp.fau.de
	char login[30];
	sscanf(buffer,"%[^@]@%s",login,serverAddr);
	printf("Login %s\nServeur %s\n",login, serverAddr);
    int sockServeurCMD;
    ecode = connect2Server(serverAddr, "21", &sockServeurCMD);
    if (ecode== -1){
    perror("Problème connexion serveur FTP");
    exit(2);      
    }
    printf("Bien connecté au serveur FTP\n");

    //le serveur envoie 220 au proxy
    ecode=read(sockServeurCMD,buffer,MAXBUFFERLEN-1);
    if (ecode==-1){
    	perror("Problème lecture d'un socket");
    	exit(2);}  
    buffer[ecode]='\0';
    printf("Recu du serveur %s\n",buffer);

    //le proxy envoie USER anonymous au serveur et recoit 331
    strcpy(buffer,login);
    strcat(buffer,"\r\n");
    write(sockServeurCMD, buffer, strlen(buffer));
    ecode=read(sockServeurCMD,buffer,MAXBUFFERLEN-1);
    if (ecode==-1){
    	perror("Problème lecture d'un socket");
    	exit(2);}  
    buffer[ecode]='\0';
    printf("Recu du serveur %s\n",buffer);

    //le proxy envoie 331 au client et le proxy recoit PASS
    write(descSockCOM, buffer, strlen(buffer));
    ecode=read(descSockCOM,buffer,MAXBUFFERLEN-1);
    if (ecode==-1){
    	perror("Problème lecture d'un socket");
    	exit(2);}  
    buffer[ecode]='\0';
    printf("Recu du client %s\n",buffer);

    //Le proxy envoie le PASS au serveur
    write(sockServeurCMD ,buffer, strlen(buffer));
    ecode=read(sockServeurCMD,buffer,MAXBUFFERLEN-1);
    if (ecode==-1){
    	perror("Problème lecture d'un socket");
    	exit(2);}  
    buffer[ecode]='\0';
    printf("Recu du serveur %s\n",buffer);

    //le proxy envoie 230 ... au client et le proxy recoit syst
    write(descSockCOM, buffer, strlen(buffer));
    ecode=read(descSockCOM,buffer,MAXBUFFERLEN-1);
    if (ecode==-1){
    	perror("Problème lecture d'un socket");
    	exit(2);}  
    buffer[ecode]='\0';
    printf("Recu du client %s\n",buffer);

    //tant que le client n'a pas envoyé port alors l'echange continue
    while (strncmp(buffer,"PORT",4)!=0){
        //le proxy envoie au serveur le message du client
    	write(sockServeurCMD ,buffer, strlen(buffer));
    	ecode=read(sockServeurCMD,buffer,MAXBUFFERLEN-1);
    	if (ecode==-1){
    		perror("Problème lecture d'un socket");
    		exit(2);}  
    	buffer[ecode]='\0';
    	printf("Recu du serveur %s\n",buffer);
     
        //le proxy envoie au client le message du serveur
    	write(descSockCOM, buffer, strlen(buffer));
    	ecode=read(descSockCOM,buffer,MAXBUFFERLEN-1);
    	if (ecode==-1){
    		perror("Problème lecture d'un socket");
    		exit(2);}  
    	buffer[ecode]='\0';
    	printf("Recu du client %s\n",buffer);

        
    }
    
   
    //Fermeture de la connexion
    close(descSockCOM);
    close(descSockRDV);
}

