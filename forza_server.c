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
#include <error.h>
#include <errno.h>

#define CHA 'r' //richiesta di gioco
#define CMD 'c' //invio comando
#define PNF 'f' //Player not found
#define PLO 'b' //Player busy (occupato)
#define REQ 'a' //request challenge
#define CONN 'n' //connessione al server (TCP)
#define WHO 'w'
#define AOR 'o' //accepted or refused
#define PAR 'p' //parametri da passare per l'inizio partita Port number e IP address
#define FREE 'e' //libera

#define MAX 5
#define STDIN 0 //descrittore standard input
#define CERCHIO 'O'
#define CROCE 'X'
#define MAXNAME 32

#define MAXTCP 512

int i=0;

int sk_TCP;

struct sockaddr_in my_par;

fd_set master;
fd_set read_fds;
int fdmax;

int num_players=0;

/*buffer per canale TCP*/
char buf_inTCP[MAXTCP];
char buf_outTCP[MAXTCP];	

struct players{
char name[MAXNAME];
char port[16];
int occupato;
int num_socket;
char ip[32];
struct sockaddr_in player_par;
struct players* rival; //punt. a struttura avversario
struct players* next;
};


struct players* testa=NULL;


void configura_player(struct players * elem,int sk,char * name)
{
	elem->num_socket=sk;
	inet_ntop(AF_INET,( void *) &elem->player_par.sin_addr.s_addr,elem->ip,sizeof(struct sockaddr_in));
	if(name!=NULL)
		strcpy(elem->name,name);
}

struct sockaddr_in* crea_player()
{
	struct players* elem;
	elem=(struct players *)malloc(sizeof(struct players));
	
	elem->next=testa;
	testa=elem;
	return &elem->player_par; //ritorna l'ind. dei parametri del client in modo da poterli passare all'accept che li setta
}

void stampa_giocatori(struct players* testa)
{
	struct players* p=NULL;
	p=testa;
	if(p==NULL)return;
	while(p)
	{
		
		printf("Nome: %s\n",p->name);
		printf("Porta: %s\n",p->port);
		printf("socket: %d\n",p->num_socket);
		
		
		p=p->next;
	}
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

int ctoi(char c)
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

void send_TCP()
{
	printf("a\n");
	//send(sk_tcp,buf_outTCP,sizeof(buf_outTCP),0);
	//int ret=send(sk_TCP,buf_outTCP,10,0);
	printf("b\n");
	/*if(ret<=0)
	{
		printf("Errore send TCP()");
	}
	printf("Caratteri inviati: %d\n",ret);*/
}


void cerca_per_sk(int sk,void ** t)
{
	struct players* p;
	p=testa;
	while(p)
	{
		if(p->num_socket==sk)
			{
				
				*t=(void *)p;
			}
		p=p->next;
		
	}
return ;
}



void elimina_players(int sk)
{
	struct players* p;
	struct players* r;
	if(testa!=NULL) {

		/* eliminazione elementi dal secondo in poi */
    if(testa->next!=NULL) {
      p=testa;
      while(p->next!=NULL) {
        if(p->next->num_socket ==sk ) {
          r=p->next;
          p->next=p->next->next;
          free(r);
        }
        else
          p=p->next;
      }
    }

		/* elimina primo elemento se == sk */
    if(testa->num_socket==sk) {
      r=testa;
      testa=testa->next;
      free(r);
    }
  }
	
}


void componi_tcp(char type,char *port,char len,char * value)
{
	svuota_tcp();
	int i=0;
	buf_outTCP[0]='\0';
	strncat(buf_outTCP,(const char*)&type,1);
	strncat(buf_outTCP,(const char*)port,16);
	for(i=strlen(port)+1;i<17;i++)buf_outTCP[i]=' ';
	buf_outTCP[17]='\0';
	strncat(buf_outTCP,(const char*)&len,1);
	strcat(buf_outTCP,(const char*)value);
	if((33+MAXNAME*(int)len)<511)
	{
	for(i=strlen(value)+18;i<32+MAXNAME*(int)len;i++)buf_outTCP[i]=' ';
	
	buf_outTCP[33+MAXNAME*(int)len]='\0';
	}
}



void controllo_tipo(int j) /*Gli passo il file descriptor del socket per operare sulla giusta connessione TCP 
				da dove si è ricevuto il pacchetto*/
{
	
	struct players* p=NULL;

	/*richiesta di connessione*/
	if(CONN==buf_inTCP[0])
	{ 
				/*aggiungo il plyer alla lista*/
			
			cerca_per_sk(j,(void *) &p);
			
/*-------------------controlla se il nome è gia' presente---------*/
		//int trovato=0;
		
		struct players* t=testa;
		
		while(t)
		{
			
			if(strncmp(&buf_inTCP[18],t->name,(int)strlen(t->name))==0 && (strlen(t->name)==(int)buf_inTCP[17]))
				{
					//connessione rifiutata-->>invio risposta		/*nome già nella lista*/
					//trovato=1;
					componi_tcp(CONN,"0000",(char) 0,"refused");
					send(j,(void*)buf_outTCP,MAXTCP,0); 
					return;
				}
			t=t->next;
		}
			
				
		
/*-----------------*/
			/*---Inserimento dati nella lista dei players---*/
			//printf("arr:%s\n",buf_inTCP);
			//printf("Len: %d\n",(int)buf_inTCP[17]);
			p->name[0]='\0';
			strncat((char *)p->name,&buf_inTCP[18],(int)buf_inTCP[17]);
			printf("%s si e' connesso\n",(char *)p->name);
			printf("%s e' libero\n",(char *)p->name);
			strncpy((char *)p->port,&buf_inTCP[1],16);
			p->occupato=0;
			num_players++;
			componi_tcp(CONN,"0000",(char )0,"ok");
			send(j,(void *)buf_outTCP,MAXTCP,0);
			svuota_tcp();
	}

	if(WHO==buf_inTCP[0]) /*arrivo pacchetto per richiesta lista giocatori*/
	{
		
		svuota_tcp();
		char * str=(char *)malloc(500);
		struct players* t;
		cerca_per_sk(j,(void *) &t);
		struct players* p=testa;
		while(p)
		{
			if(strcmp(t->name,p->name)!=0) /* scorro la lista dei giocatori escludendo il nome del player che ha fatto la richiesta who*/
			{
			if(p->occupato==1)strcat(str," (Occupato) ");
			else strcat(str," (Libero) ");
			strcat(str,p->name);	
			strcat(str,"| ");
			
			}
			p=p->next;
		}
		svuota_tcp();
		if(num_players==1)
			componi_tcp(WHO,"0000",(char)num_players,str);
		else
			componi_tcp(WHO,"1111",(char)num_players,str);
		send(j,(void *)buf_outTCP,MAXTCP,0);
		str[0]='\0';
		free(str);
		svuota_tcp();
	}

	if(CHA==buf_inTCP[0]) /*arrivo pacchetto con richiesta di sfida*/
	{
		int trovato=0;
		
		struct players* t=testa;
		
		while(t)
		{
			if(strncmp(&buf_inTCP[18],t->name,strlen(t->name))==0 && (strlen(t->name)==(int)buf_inTCP[17]))
				{
					if(t->occupato==1)
					{
						componi_tcp(PLO,"0000",(char )0,"Player occupato");
						send(j,(void *)buf_outTCP,MAXTCP,0);
						trovato=1;
					}
					else
					{
					
					trovato=1;
					//char* tmp=(char*)malloc((int)buf_inTCP[17]+1);
					//strncpy(tmp,&buf_inTCP[18],(int)buf_inTCP[17]);
					cerca_per_sk(j,(void *) &p);
					
//
					componi_tcp('a',p->port,(char) strlen(p->name),p->name);
					
					send(t->num_socket,(void*)buf_outTCP,MAXTCP,0);
					t->rival=p; //faccio puntare la struttura associata a chi viene sfidato con la struttura di chi lancia la sfida
					p->rival=t;     /*Adesso le strutture dei due rivali si puntano a vicenda*/
					p->occupato=1;
					t->occupato=1;
					svuota_tcp();
					}
				}
			t=t->next;
		
		}		
		if(trovato==0)
			{			
				componi_tcp(PNF,"0000",(char )0,"Player not found");
				send(j,(void *)buf_outTCP,MAXTCP,0);
			}	
		//stampa_giocatori(testa);
	}
	if(REQ==buf_inTCP[0]) //risposta da parte del player sfidato alla richiesta di sfida
	{
		//printf("ricevuto : %s\n",buf_inTCP);
		if(buf_inTCP[17]=='n')
		{
			//printf("ok\n");
			//richiesta rifiutata da parte del player
			cerca_per_sk(j,(void *) &p);
			p->occupato=0;
			p->rival->occupato=0;
//
			componi_tcp(AOR,"0000",'n',p->name);
			send(p->rival->num_socket,(void *)buf_outTCP,MAXTCP,0);
			p->rival->rival=NULL; /*sgancio le 2 strutture*/
			p->rival=NULL;
		}
		if(buf_inTCP[17]=='y')
		{
			
			//richiesta accettata da parte del player
			cerca_per_sk(j,(void *) &p);
			componi_tcp(PAR,p->rival->port,(char) strlen(p->rival->ip),p->rival->ip);
			send(p->num_socket,(void *)buf_outTCP,MAXTCP,0);
			componi_tcp(AOR,p->port,'y',p->ip);
			send(p->rival->num_socket,(void *)buf_outTCP,MAXTCP,0);
		}
		
	}
	
	if(FREE==buf_inTCP[0])
	{
		cerca_per_sk(j,(void *) &p);
		p->occupato=0;
		p->rival->occupato=0;
		printf("%s si è disconnesso da %s\n",p->name,p->rival->name);
		printf("%s è libero\n",p->name);
		printf("%s è libero\n",p->rival->name);
	}
	
}



int ston(char* str)
{	
	
	int tmp=atoi(str);	
return tmp;
}

int main(int argc,char * argv[])
{	
	if(argv[1]==NULL || argv[2]==NULL)
		{
			perror("Errore : Inserisci un comando del tipo ./forza_server <host> <porta> \n");
			//sleep(2);
			_exit(1);
		}

	char yes;
	int i=0;
	int new_client_socket=0;
	/* timer per select*/
	struct timeval tv;
	tv.tv_sec=60;
	tv.tv_usec=0;	
		
	
	char c;
	sk_TCP=socket(AF_INET,SOCK_STREAM,0);
	memset(&my_par,0,sizeof(my_par));
	my_par.sin_family=AF_INET;
	my_par.sin_port=htons(ston(argv[2]));
	inet_pton(AF_INET,argv[1],&my_par.sin_addr.s_addr);
	
if(setsockopt(sk_TCP,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
	{
	printf("Setsock err\n");
	_exit(-1);	
	}	
	int ret=bind(sk_TCP,(struct sockaddr *)&my_par,sizeof(my_par));
	if(ret<0)
	{
		printf("Errore bind()");
		_exit(-1);
	}
	
	ret=listen(sk_TCP,10);
	if(ret<0)
	{
		printf("Errore listen");
		_exit(-1);
	}
	
	printf("Indirizzo: %s (Porta: %s)\n",argv[1],argv[2]);
		
	/*azzera le maschere per i descrittori usati nella select*/
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	/*aggiunge alla maschera i descrittori sk_udp*/
	FD_SET(sk_TCP,&master);
	
	fdmax=sk_TCP;	
	while(1)
	{
	
		/*reset del timer select()*/
		tv.tv_sec=60;
		tv.tv_usec=0;
		read_fds=master;

	
		if(select(fdmax+1,&read_fds,NULL,NULL,&tv)==-1)
		{
			printf("Select() error!");
			_exit(1);
		}
		
		for(i=0;i<=fdmax;i++)
		{
			if(FD_ISSET(i,&read_fds))
			{
			if(i==sk_TCP)
			{
				/*socket listener di ascolto, richiesta di connessione da parte di un client*/
				socklen_t size=sizeof(struct sockaddr_in);
				struct sockaddr_in * ind= crea_player();
				new_client_socket=accept(sk_TCP,(struct sockaddr* )ind,&size);
				configura_player(testa,new_client_socket,NULL);
				FD_SET(new_client_socket,&master);	
				printf("Connessione stabilita con il client\n");
				if(fdmax<new_client_socket)fdmax=new_client_socket;

				
			}
			
			else
			{
				
				/*dati in arrivo da un client, bisogna veder quale*/
				int ret=recv(i,(void *)buf_inTCP,MAXTCP,0);  //ricevo i dati	
				if(ret>0)
				{
				//printf("Type :%s\n",buf_inTCP);
				controllo_tipo(i);				

				
				//FD_CLR(i,&master); //tolgo il descrittore da quelli da controllare
				}

				else
				{
					struct players * p;
					cerca_per_sk(i,(void *) &p);
					printf("%s si e' disconnesso dal server\n",p->name);
					FD_CLR(i,&master);
					close(i);
					elimina_players(i);
					//stampa_giocatori(testa);
					num_players--;
					
					
				}
				
			}
		}
	}
}	
		
	scanf("Inserisci invio,%c\n",&c);
	return 0;
}
