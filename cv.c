#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX 512
#define FIFO "/tmp/so"

int show(char* messageToServer, char* clientFIFO, char* code) 
{
	int codigo;
	if((codigo = atoi(code)) < 0) 
	{
		perror("argumento nao inteiro"); 
		exit(-1);
	}

	snprintf(messageToServer, MAX, "%s;%d;%d", clientFIFO, 0, codigo);
	
	return 0;
}

int atualiza(char* messageToServer, char* clientFIFO, char* code, char* quant) 
{
	int codigo;
	int quantidade;
	
	quantidade = atoi(quant);

	if((codigo = atoi(code)) < 0) 
	{
		perror("argumento nao inteiro"); 
		exit(-1);
	}

	snprintf(messageToServer, MAX, "%s;%d;%d;%d", clientFIFO, 1, codigo, quantidade);
	return 0;
}

ssize_t readln (int fd, void *buf, size_t nbyte) {
    nbyte--;
    char *cbuf = (char*) buf;
    int i;
    int rd=1;
    off_t foundnl = 0;

    rd = read (fd, cbuf, nbyte);

    for (i = 0; i < rd; i++)
        if (cbuf[i] == '\n'){
            foundnl = 1;
            break;
        }

    cbuf[i] = 0;

    lseek (fd, (i - rd) + foundnl, SEEK_CUR);
    return (foundnl == 1 ? i : -i);
}

int main()
{
	char clientFIFO[100];
	char messageToServer[MAX];
	char messageFromServer[MAX];
	int toServer;
	int fromServer;
	int res;
	int argc = 0;
	char buffer[20];
	char* primeiro = malloc(sizeof(char) * 10);
	char* segundo = malloc(sizeof(char) * 10);

	//for (int i = 0; i < 100; ++i)
	//	clientFIFO[i]  = 0;
	//alternativa
	memset(clientFIFO, 0, 100);

	snprintf(clientFIFO, 100, "/tmp/so_%d", getpid());
		if(access(clientFIFO, R_OK) == -1){
			if(mkfifo(clientFIFO, 0666) != 0){
				perror("criacao fifo client");
				exit(-1);
			}
		}
		
	

	while(1){
		argc = 0;
		
		if(readln(0, buffer, 20) == -1)
				{perror("fim");exit(0);}

		primeiro = strtok(buffer, " ");

		if((segundo = strtok(NULL, " "))){
			argc++;
		}
		
		if(!strlen(buffer)) break;

		if(access(FIFO, W_OK) == -1){
			perror("acesso escrita fifo server");
			exit(-1);
		}
		
		memset(messageToServer,0,MAX);
		memset(messageFromServer,0,MAX);

		switch(argc){
			case 0:
				res = show(messageToServer, clientFIFO, primeiro);
					break;
			case 1: res = atualiza(messageToServer, clientFIFO, primeiro, segundo);
					break;
			default: perror("Demasiados argumentos"); exit(-1);
		}

		if(!res)
		{
			toServer = open(FIFO, O_WRONLY,  O_NONBLOCK);
			write(toServer,messageToServer,strlen(messageToServer));
			close(toServer);

			fromServer = open(clientFIFO, O_RDONLY,  O_NONBLOCK);
			read(fromServer, messageFromServer, MAX);
			close(fromServer);

			write(1, messageFromServer, strlen(messageFromServer));
			write(1,"\n",1);
		}
	}
	
	return 0;
}