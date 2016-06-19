#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <math.h>
#include <errno.h>
#include <error.h>
#include <netdb.h>

/*
	PACCHETTO per TCP

   CMD	 portUDP   len oppure (y/n)         data
---------------------------------------------------------
|	|	|		|			|
|	|	|		|			|
---------------------------------------------------------
    1       16        1                                512
*/


/*
	PACCHETTO UDP
    Type	Value
-------------------------
|	    |		|
|	    |		|
-------------------------
	1	1
*/
#define CHA 'r' //richiesta di gioco 
#define CMD 'c' //invio comando
#define REQ 'a'
#define CONN 'n' //connessione al server (TCP)
#define WHO 'w' //lista giocatori
#define PNF 'f' //Player not found
#define PLO 'b' //Player busy (occupato)
#define AOR 'o' //accepted or refused
#define PAR 'p' //parametri da passare per l'inizio partita Port number e IP address
#define FREE 'e' //libera
#define WIN 'v' //vittoria

#define ABB 's' //abbandono (UDP)
#define MAX 6
#define STDIN 0 //descrittore standard input
#define CERCHIO 'O'
#define CROCE 'X'

#define MAXNAME 32

#define MAXTCP 512

char mio_gettone=CERCHIO;
char gettone_avversario=CERCHIO;

char griglia[6][7];
int top[7];

int partita=0;
char answ;

char buf_inUDP[16];
char buf_outUDP[16];
char buf_inTCP[MAXTCP];
char buf_outTCP[MAXTCP];

char buf_in[16];

int i=0;
int flush=1;
int disconnesso=0;

char my_name[MAXNAME];
char name_avversario[MAXNAME];

char myIP[15];
char ip_avversario[32];

char myUDPport[16];
char UDPport_avversario[16];	

	int sk_udp;
	int sk_tcp;
	
	/*parametri indirizzi socket*/
	struct sockaddr_in sk_addr; //miei parametri
	struct sockaddr_in sk_addr_tcp;
	struct sockaddr_in sk_addr_avversario;	//parametri avversario

	fd_set master_home;
	fd_set read_fds_home;
	int fdmax_home;

	fd_set master;
	fd_set read_fds;
	int fdmax;
	
	int yes=1;

void 
init_myIP()
{
	strcpy(myIP,"127.0.0.1");
}

void 
send_TCP()
{
	//send(sk_tcp,buf_outTCP,sizeof(buf_outTCP),0);
	int ret=send(sk_tcp,buf_outTCP,MAXTCP,0);
	if(ret==0)
	{
		perror("Errore send TCP()");
		_exit(1);
	}
}

void 
send_UDP()
{
	int ret=sendto(sk_udp,buf_outUDP,sizeof(buf_outUDP),0,(struct sockaddr*)&sk_addr_avversario,sizeof(sk_addr_avversario));
	if(ret<=0)
	{
		perror("errore sendto, errno: %d\n");
		_exit(1);
	}
	

}

void 
receive_TCP()
{
	int ret=recv(sk_tcp,buf_inTCP,MAXTCP,0);
	if(ret<=0)
		{
			if(ret<0)
				{
				printf("Errore recv()");
				_exit(1);
				}
			else if(ret==0)
			{
				printf("Connessione TCP con il server caduta :(\n");
				close(sk_tcp);
				FD_CLR(sk_tcp,&master_home);
				disconnesso=1;
			}
		}
}


void 
componi_udp(char type,char arg)
{
	buf_outUDP[0]=type;
	buf_outUDP[1]=arg;
}

void 
componi_tcp(char type,char *port,char len,char * value)
{
	int i=0;
	buf_outTCP[0]='\0';
	strncat(buf_outTCP,(const char*)&type,1);
	strncat(buf_outTCP,(const char*)port,16);
	for(i=strlen(port)+1;i<17;i++)buf_outTCP[i]=' ';
	buf_outTCP[17]='\0';
	strncat(buf_outTCP,(const char*)&len,1);
	strncat(buf_outTCP,(const char*)value,MAXNAME);
	for(i=strlen(value)+18;i<48;i++)buf_outTCP[i]=' ';
	buf_outTCP[49]='\0';
	//printf("Stringa inviata: %s\n",buf_outTCP);
	
}

void 
inizializza_griglia()
{
	int i=0,j=0;
	for(i=0;i<7;i++)
		for(j=0;j<6;j++)griglia[j][i]=' ';
}

void 
readStdin()
{
	/*legge e svuota la standard input*/
	int i=0;
	while(1)
	{
		buf_in[i]=getchar();
		if(buf_in[i]=='\n')
		{
			buf_in[i]='\0';
			break;
		}
		i=i+1;
	}
}

void 
stampa_griglia()
{
	printf("\n");
	printf("6 | %c | %c | %c | %c | %c | %c | %c |\n",griglia[5][0],griglia[5][1],griglia[5][2],griglia[5][3],griglia[5][4],griglia[5][5],griglia[5][6]);
		
		printf("5 | %c | %c | %c | %c | %c | %c | %c |\n",griglia[4][0],griglia[4][1],griglia[4][2],griglia[4][3],griglia[4][4],griglia[4][5],griglia[4][6]);
		printf("4 | %c | %c | %c | %c | %c | %c | %c |\n",griglia[3][0],griglia[3][1],griglia[3][2],griglia[3][3],griglia[3][4],griglia[3][5],griglia[3][6]);
		printf("3 | %c | %c | %c | %c | %c | %c | %c |\n",griglia[2][0],griglia[2][1],griglia[2][2],griglia[2][3],griglia[2][4],griglia[2][5],griglia[2][6]);
		printf("2 | %c | %c | %c | %c | %c | %c | %c |\n",griglia[1][0],griglia[1][1],griglia[1][2],griglia[1][3],griglia[1][4],griglia[1][5],griglia[1][6]);
		printf("1 | %c | %c | %c | %c | %c | %c | %c |\n",griglia[0][0],griglia[0][1],griglia[0][2],griglia[0][3],griglia[0][4],griglia[0][5],griglia[0][6]);
		printf("  -----------------------------\n");
		printf("    a   b   c   d   e   f   g\n");
	
}

void 
svuota_griglia()
{
	inizializza_griglia();
	for(i=0;i<7;i++)top[i]=0;	
}

void svuota_tcp()
{
	for(i=0;i<MAXTCP;i++)
	{
		buf_outTCP[i]='\0';
		buf_inTCP[i]='\0';
	}
	//buf_outTCP[0]='\0';
	//buf_inTCP[0]='\0';
}

int 
ctoi(char c)
{
	switch(c){
		case '0': return 0;
		
		case '1': return 1;
			
		case '2': return 2;

		case '3': return 3;

		case '4': return 4;
	
		case '5': return 5;

		case '6': return 6;

		case '7': return 7;
		
		case '8': return 8;

		case '9': return 9;
		}
	return 'e';
}

char 
itoc(int c)
{
	switch(c){
		case 0: return '0';
		
		case 1: return '1';
			
		case 2: return '2';

		case 3: return '3';

		case 4: return '4';
	
		case 5: return '5';

		case 6: return '6';

		case 7: return '7';
		
		case 8: return '8';

		case 9: return '9';
		}
	return 0;
}

int 
ctr_win() //controlla se c'è una vittoria
{
	int i=0;
	int j=0;
	int win=0;
	/*controlla per colonne, righe e diagonale*/
	for(j=0;j<4;j++)
		for(i=0;i<3;i++)
		{	
			if(((griglia[i][j])==(griglia[i+1][j])&&(griglia[i][j])==(griglia[i+2][j])&&(griglia[i][j])==(griglia[i+3][j]))||((griglia[i][j])==(griglia[i][j+1])&&(griglia[i][j])==(griglia[i][j+2])&&(griglia[i][j])==(griglia[i][j+3]))||((griglia[i][j])==(griglia[i+1][j+1])&&(griglia[i][j])==(griglia[i+2][j+2])&&(griglia[i][j])==(griglia[i+3][j+3])))
				{	
					if(griglia[i][j]!=' ')
					{				
						if(griglia[i][j]==mio_gettone)	
							win=1; //vinco io
						else win=-1;	//vince l'avversario
					}
				}
		}
	i=0;
	j=0;

	for(j=0;j<7;j++)
		for(i=0;i<6;i++)
		{
			if(((griglia[i][j])==(griglia[i-1][j+1])&&(griglia[i][j])==(griglia[i-2][j+2])&&(griglia[i][j])==(griglia[i-3][j+3])&&(i>=3)&&(j<=3)))
			{
				if(griglia[i][j]!=' ')
					{				
						if(griglia[i][j]==mio_gettone)	
							win=1; //vinco io
						else win=-1;	//vince l'avversario
					}
			}
		}
	//printf("win:%d\n",win);
	return win;
}


void 
opzioni(char * str)
{		
	if(strcmp(str,"!exit")==0)
		{
			if(partita==0)
			 	_exit(1);
			return;
		}		
	
	else if(strcmp(str,"!who")==0)
	{	if(disconnesso==0)
		{
		componi_tcp(WHO,myUDPport,(char)1,my_name);
		send_TCP();
		svuota_tcp();	
		}
		else if(disconnesso==1)printf("Non sei connesso al server :( \n");
	return;
	}

	/*comando !show_map*/
	else if(strcmp(str,"!show_map")==0)
	{ 	
		stampa_griglia();
		return;
	}
	/*comando !help*/
	else if(strcmp(str,"!help")==0)
	{
		printf("Sono disponibili i seguenti comandi:\n");
		printf(" * !help --> mostra l'elenco dei comandi disponibili\n");
		printf(" * !disconnect --> disconnette il client dall'attuale partita\n");
		printf(" * !quit --> viene chiusa la connessione con il server\n");
		printf(" * !who --> mostra l'elenco dei client connessi al server\n");
		printf(" * !insert column --> insrisce il gettone in column (valido solo quando è il proprio turno)\n");
		printf(" * !show_map --> mostra la mappa di gioco\n");
		printf(" * !exit --> esci dal gioco\n");
		return;
	}
	
	/*comando !insert*/
	else if(strncmp(str,"!insert ",strlen("!insert "))==0)
	{
		if(partita==0)
		{
			printf("Non stai giocando con alcun giocatore !\n");
			return;
		}
		if((str[8])<'a'||(str[8])>'g')
		{
			printf("Casella errata!\n");
		}
		else
		{
			
			int tmp=ctoi(str[8]-49);
			if(top[tmp]==MAX)
				{
				printf("Colonna piena, inserisci in un altra colonna\n");
				 return;
				}
			griglia[top[tmp]][tmp]=mio_gettone;
			top[tmp]++;
			componi_udp(CMD,(str[8]-49));
			if(ctr_win()>0)
				{
				/*-------VITTORIA--------*/
					printf("\n");
					stampa_griglia();
					printf("\n");
					printf("Complimenti hai vinto :D !!\n");
					componi_udp(WIN,(str[8]-49));
					send_UDP();
					componi_tcp(FREE,myUDPport,(char)strlen(my_name),my_name); // informa il server che il giocatore e lo sfidante sono liberi quindi può considerarli liberi
					send_TCP();
					printf("\n");
					partita=0;
					close(sk_udp);
					return;
				}
			send_UDP();
			/*if(ret<0)
				{printf("Errore sendto()");printf("errno: %d\n",(int)errno);}*/
			svuota_tcp();
			printf("\n");
			printf("E' il turno di %s\n",name_avversario);
			FD_CLR(STDIN,&read_fds); //disattivo la console
			FD_CLR(STDIN,&master);
			flush=0;
			
		}
		
	return;	
	}

	else if(strncmp(str,"!connect",8)==0)
	{
		if(disconnesso==0)
		{
		strcpy(name_avversario,&buf_in[9]);
		int l=strlen(&buf_in[9]);
		componi_tcp(CHA,myUDPport,(char)l,name_avversario);
		send_TCP();
		svuota_tcp();
		}
		else if(disconnesso==1)printf("Non sei connesso al server :( \n");
	return;
	}
	
	else if(strncmp(str,"!quit",5)==0)
	{
		int rc=close(sk_tcp);
		if(rc<0){printf("error close %d",(int)errno);}
		printf("Disconnessione dal server\n");
		FD_CLR(sk_tcp,&master_home);
		disconnesso=1;
		return;
	}
	
	else printf("comando non valido\n");
}


void 
abbandono()
{
	componi_udp(ABB,'0');
	send_UDP();
	componi_tcp(FREE,myUDPport,(char)strlen(my_name),my_name); // informa il server che il giocatore e lo sfidante sono liberi quindi può considerarli liberi
	send_TCP();
}


int 
ston(char* str)
{	
	
	int tmp=atoi(str);	
	return tmp;
}


void 
analizza_pkt_udp()
{
int tmp;
	switch(buf_inUDP[0]){
			
		case ABB: printf(" %s si è arreso : HAI VINTO !!\n",name_avversario);
				close(sk_udp);
				partita=0;
				break;
		case CMD:
				//printf("Ricevuto: %s\n",buf_inUDP);
			tmp=ctoi(buf_inUDP[1]);
			griglia[top[tmp]][tmp]=gettone_avversario;
			top[tmp]++;
			printf("%s ha marcato la casella %c%d\n",name_avversario,(char)itoc(tmp)+49,top[tmp]-1);
			printf("E' il tuo turno:\n");
			FD_SET(STDIN,&read_fds); //attivo la console
			FD_SET(STDIN,&master);			
			flush=1;
				break;
		case WIN:
			printf("\n");
			/*aggiorno la griglia*/
			tmp=ctoi(buf_inUDP[1]);
			griglia[top[tmp]][tmp]=gettone_avversario;
			top[tmp]++;
			/*notifico vittoria*/
			stampa_griglia();
			printf("\n");
			printf("Hai perso, ha vinto %s :(\n",name_avversario);
			partita=0;
			close(sk_udp);
			break;
					
				}
		
}

void 
analizza_pkt_tcp()
{
char* str;

	switch(buf_inTCP[0]){
		
		case WHO : printf("Client connessi al server: ");
				if(strncmp("0000",&buf_inTCP[1],4)==0)printf("Nessun giocatore connesso!\n");
				else
				printf("%s\n",&buf_inTCP[18]);
				break;
		case PNF : printf("Player inesistente\n");
				break;
		
		case PLO : printf("Player già occupato in un altra partita\n");
				break;

		case REQ : 
				strncpy(name_avversario,&buf_inTCP[18],(int)buf_inTCP[17]);
				printf("Richiesta di gioco da parte di %s , accetti? (y/n) :\n",name_avversario);
				answ='c';
			while(answ!='n' && answ!='y')
			{	
				scanf("%c",&answ);
	
				if(answ=='n')
					{
						/*richiesta rifiutata*/
						componi_tcp(REQ,myUDPport,(char)'n',&buf_inTCP[17]);
						send_TCP();
						svuota_tcp();
						return;
					}
				else if(answ=='y')
					{	
						/*richiesta accettata*/
						
						componi_tcp(REQ,myUDPport,(char)'y',name_avversario);
						send_TCP();
						printf("partita avviata con %s \n",name_avversario);
						mio_gettone=CERCHIO;
						gettone_avversario=CROCE;
						printf("E' il turno di %s\n",name_avversario);
						FD_CLR(STDIN,&read_fds); //disattivo la console
						FD_CLR(STDIN,&master);
												
						flush=0;
						svuota_tcp();
						return;
					}
				else printf("Inserisci y oppure n  :\n");
			}
				break;
		case AOR:
				if(buf_inTCP[17]=='n')
				{
					printf("%s ha rifiutato la partita\n",&buf_inTCP[18]);
				}
				else if(buf_inTCP[17]=='y')
				{	
					mio_gettone=CROCE;
					gettone_avversario=CERCHIO;		
					printf("%s ha accettato la partita\n",name_avversario);
					printf("partita avviata con %s\n",name_avversario);
					printf("Il tuo simbolo è: X\n");
					printf("E' il tuo turno: \n");

					partita=1;
					strcpy(UDPport_avversario,&buf_inTCP[1]);/*settaggio parametri avversario*/
					strcpy(ip_avversario,&buf_inTCP[18]);
					//printf("Ip avversario : %s port : %s\n",ip_avversario,UDPport_avversario);

					/*----------connessione UDP-----------*/


	
	sk_udp=socket(AF_INET,SOCK_DGRAM,0); //creazione soket UDP per P2P
	memset(&sk_addr,0,sizeof(sk_addr));	//azzero struttura sk_addr
					
/*azzera le maschere per i descrittori usati nella select*/
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	FD_SET(sk_udp,&master);
	FD_SET(STDIN,&master);
	
	fdmax=sk_udp;
				
	socklen_t len_sk_avversario=sizeof(sk_addr_avversario);
	memset(&sk_addr_avversario,0,len_sk_avversario);
	sk_addr_avversario.sin_family=AF_INET;
	sk_addr_avversario.sin_port=htons((short)atoi(UDPport_avversario));
	inet_aton(ip_avversario,&sk_addr_avversario.sin_addr);

	sk_addr.sin_family=AF_INET; 	//IPv4 address
	sk_addr.sin_addr.s_addr=htonl(INADDR_ANY); //imposto l'ind. generico 0.0.0.0 con INADDR_ANY
	sk_addr.sin_port=htons(ston(myUDPport)); //host order(little-endian) to network order(big-endian)
	inet_pton(AF_INET,myIP,&sk_addr.sin_addr.s_addr); 	//IP da presentation a network
	int ret_bin=bind(sk_udp,(struct sockaddr *)&sk_addr,sizeof(sk_addr));
	if(ret_bin<0)
		{
		printf("Errore Bind");
		_exit(1);		
		}

}				
				break;
		case PAR:
				str=(char*)malloc(16);
				while(buf_inTCP[1+i]!=' '&&i<=16)
				{
					str[i]=buf_inTCP[1+i];
					i++;
				}
				strncpy(UDPport_avversario,&buf_inTCP[1],16);
				strncpy(ip_avversario,&buf_inTCP[18],(int) buf_inTCP[17]);
				//printf("Ip avversario : %s port : %s\n",ip_avversario,UDPport_avversario);
				
				/*----------connessione UDP-----------*/

/*aggiunge alla maschera i descrittori sk_udp e Stdin*/
	
	sk_udp=socket(AF_INET,SOCK_DGRAM,0); //creazione soket UDP per P2P

	memset(&sk_addr,0,sizeof(sk_addr));	//azzero struttura sk_addr
					
/*azzera le maschere per i descrittori usati nella select*/
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	FD_SET(sk_udp,&master);
	
	fdmax=sk_udp;			

	socklen_t len_sk_avversario=sizeof(sk_addr_avversario);
	memset(&sk_addr_avversario,0,len_sk_avversario);
	sk_addr_avversario.sin_family=AF_INET;
	sk_addr_avversario.sin_port=htons((short)atoi(UDPport_avversario));
	inet_aton(ip_avversario,&sk_addr_avversario.sin_addr);
	

	sk_addr.sin_family=AF_INET; 	//IPv4 address
	sk_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	sk_addr.sin_port=htons(ston(myUDPport)); //host order(little-endian) to network order(big-endian)
	inet_pton(AF_INET,myIP,&sk_addr.sin_addr.s_addr); 	//IP da presentation a network
	int ret_bin=bind(sk_udp,(struct sockaddr *)&sk_addr,sizeof(sk_addr));
	if(ret_bin<0)
		{
		printf("Errore Bind");
		_exit(1);		
		}

		partita=1;

				break;
	}
//printf("Ricevuto :%s\n",buf_inTCP);
}





int 
main(int argc,char* argv[])
{
	init_myIP();	
	
	if(argv[1]==NULL || argv[2]==NULL)
		{
			perror("Errore : Inserisci un comando del tipo ./forza_client <host> <porta> \n");
			//sleep(2);
			_exit(1);
		}

	/* timer per select*/
	struct timeval tv;
	tv.tv_sec=60;
	tv.tv_usec=0;	

	sk_tcp=socket(AF_INET,SOCK_STREAM,0); //creazione soket TCP

	memset(&sk_addr_tcp,0,sizeof(sk_addr));	//azzero struttura sk_addr_tcp
	
	sk_addr_tcp.sin_family=AF_INET; 	//IPv4 address
	sk_addr_tcp.sin_addr.s_addr=htonl(INADDR_ANY);
	sk_addr_tcp.sin_port=htons(ston(argv[2])); //host order(little-endian) to network order(big-endian)
	inet_pton(AF_INET,argv[1],&sk_addr_tcp.sin_addr.s_addr); 
	
	
	
	
	if(setsockopt(sk_tcp,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
	{
	perror("Setsock err\n");
	_exit(-1);	
	}

	
	
	
	
	/*azzera le maschere per i descrittori usati nella select home*/
	FD_ZERO(&master_home);
	FD_ZERO(&read_fds_home);
	/*aggiunge alla maschera i descrittori sk_tcp e Stdin*/
	FD_SET(sk_tcp,&master_home);
	FD_SET(STDIN,&master_home);
	
	fdmax_home=sk_tcp;

	
	

	inizializza_griglia();//azzera la griglia
	

	if(connect(sk_tcp,(struct sockaddr *)&sk_addr_tcp,sizeof(sk_addr_tcp))<0)
	{
		perror("errore connect()");
		_exit(-1);
	}
		
	printf("connessione al server %s (porta %s) effettuata con successo\n\n",argv[1],argv[2]);	
	printf("Sono disponibili i seguenti comandi:\n");
	printf(" * !help --> mostra l'elenco dei comandi disponibili\n");
	printf(" * !disconnect --> disconnette il client dall'attuale partita\n");
	printf(" * !quit --> viene chiusa la connessione con il server\n");
	printf(" * !who --> mostra l'elenco dei client connessi al server\n");
	printf(" * !insert column --> insrisce il gettone in column (valido solo quando è il proprio turno)\n");
	printf(" * !show_map --> mostra la mappa di gioco\n\n");
	
	while(1)
	{
		printf("Inserisci il tuo nome:");
		scanf("%s",my_name);
		printf("Inserisci la porta UDP di ascolto:");	
		scanf("%s",myUDPport);
		printf("\n");


		componi_tcp(CONN,myUDPport,(char)strlen(my_name),my_name);
		send_TCP();
		receive_TCP();
		if(strncmp(&buf_inTCP[17],"refused",6)==0)printf("Nome player già usato, scegline un altro\n");
		else if(strncmp(&buf_inTCP[17],"ok",2)==0)break;
		
	}	

		while(1) /*non in partita (home)*/
		{
		
			/*se la partita è terminata ritorno alla home*/
			
			printf(">");
			read_fds_home=master_home;
		fflush(stdout);
			if(select(fdmax_home+1,&read_fds_home,NULL,NULL,NULL)==-1)
			{
				perror("Select() error!");
				_exit(1);
			}	
			
					if(FD_ISSET(STDIN,&read_fds_home))
					{
						readStdin();//legge dallo stdin	e riempie buf_inUDP		
						opzioni(buf_in);//opera in base al comando dell'utente
							
					}
					else
					{
					
					receive_TCP();
					analizza_pkt_tcp();
					//FD_CLR(sk_tcp,&master_home);
					}
		if(partita==1)
		{
		svuota_griglia();
		while(1) /*modalità partita*/
		{
			/*reset del timer select()*/
			tv.tv_sec=60;
			tv.tv_usec=0;
			if(partita==0)break; 
			printf("#");
			read_fds=master;
		if(flush)	
		fflush(stdout);
			if(select(fdmax+1,&read_fds,NULL,NULL,&tv)==-1)
			{
				printf("Select() error! errno: %d\n",(int)errno);
				_exit(1);
			}
					if(tv.tv_sec==0)
						{
							
							printf("Sessione scaduta\n");
							partita=0;
							abbandono();
							close(sk_udp);
							break;
						}
					FD_SET(sk_udp,&master);
					if(FD_ISSET(STDIN,&read_fds))
					{
						readStdin();//legge dallo stdin	e riempie buf_inUDP	
						if(strcmp(buf_in,"!disconnect")==0)
							{
								printf("\n");
								partita=0;
								abbandono();//informa l'avversario della resa
								close(sk_udp);
								break;
							}	
						opzioni(buf_in);//opera in base al comando dell'utente
							
						
					}
		
					else
					{
						/*ricevuti dati dal socket udp*/
						socklen_t len_sk_avversario=sizeof(sk_addr_avversario);
						int retrec=recvfrom(sk_udp,buf_inUDP,sizeof(buf_inUDP),0,(struct sockaddr*)&sk_addr_avversario,&len_sk_avversario);
						if(retrec<0)
						{
							printf("Errore receiveUDP()\n");
							_exit(1);
						}
						if(retrec==0)
							printf("Lo sfidante ha chiuso la connessione UDP\n");
						analizza_pkt_udp();
							
					}
				
			
		}
	}
}
	return 0;
}
