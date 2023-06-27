#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include<sys/time.h>
#include <arpa/inet.h>
#include"commonFunc.h"

int create_socket(char *interface_rede){
    int s;
    struct ifreq ir;
    struct sockaddr_ll endereco;
    struct packet_mreq mr;

    s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	
    if (s == -1) {
        printf("Erro no Socket\n");
        exit(-1);
    }

    memset(&ir, 0, sizeof(struct ifreq));  	
    memcpy(ir.ifr_name, interface_rede, strlen(interface_rede));
    if (ioctl(s, SIOCGIFINDEX, &ir) == -1) {
        printf("Erro no ioctl\n");
        exit(-1);
    }
	

    memset(&endereco, 0, sizeof(endereco)); 	
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ir.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
        printf("Erro no bind\n");
        exit(-1);
    }


    memset(&mr, 0, sizeof(mr));          
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(s, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
        printf("Erro ao fazer setsockopt\n");
        exit(-1);
    }

    return s;
}

int fillBuffer(mensagem_t* msg, unsigned char* buffer){

    if(msg==NULL||buffer==NULL)
        return 0;

    unsigned char aux;

    buffer[0]=MARCINI;

    aux = msg->sequencia >>4;
    buffer[1]=0;
    buffer[1]=(msg->tamanho<<2)|aux;

    aux = ~(aux<<4)&(msg->sequencia);
    buffer[2]=0;
    buffer[2]=(aux<<4)|msg->tipo;

    memcpy(&buffer[3], msg->dados, msg->tamanho);
    aux=buffer[0];
    for(int i=1;i<3+msg->tamanho;i++)
        aux=aux^buffer[i];

    if(msg->tamanho<10){
        memset(&buffer[3+msg->tamanho], 0, 10-msg->tamanho);
        buffer[13]=aux;
        return 14;

    }

    buffer[3+msg->tamanho] = aux;
    return 4+msg->tamanho;
}

int separateMessage(mensagem_t* msg, unsigned char* buffer){

    if(msg==NULL||buffer==NULL)
        return 0;

    //printf("inicio------[0]%d [1]%d [2]%d------\n",buffer[0],buffer[1],buffer[2]);

    unsigned char aux = buffer[1];
    msg->tamanho=aux>>2;
    aux=buffer[1]%4;
    msg->sequencia=(aux<<4)|(buffer[2]>>4);


    aux=buffer[2]>>4;
    aux=aux<<4;
    aux=~(aux)&buffer[2];
    msg->tipo=aux;
    memcpy(msg->dados,&buffer[3],msg->tamanho);
    //msg->dados[msg->tamanho]='\0';

    //printf("fim---------tam:%d seq:%d tipo:%d------\n",msg->tamanho,msg->sequencia,msg->tipo);

    return 1;
}

DIR* openDir(char* path){
    
    DIR* dirstream = opendir(path);
    if(!dirstream)
        return NULL;
    

    return dirstream;

}

FILE* openFile(char* path,char* action){

    FILE *arq;    
    arq = fopen(path,action);
    if(!arq)
        return NULL;
    
    return arq;

}

void sendEmpty(int s,unsigned char seq,unsigned char tipo){
    
    mensagem_t m;
    unsigned char buffer[MAXBUFF];
    setMsgAttr(&m,1,seq, tipo);
    m.dados[0]=seq;
    int size=fillBuffer(&m,buffer);
    if(send(s, buffer, size, 0)<0)
        perror("erro no envio da mensagem:");
    printf("mensagem %d enviada tipo(%d)\n",seq,tipo);

}

int verifyPariVert(unsigned char* buffer,int size){

    if(!buffer)
        return 0;

    unsigned char aux=buffer[0];
    for(int i=1;i<size-1;i++)
        aux=aux^buffer[i];

    if(aux==buffer[size-1])
        return 1;


    return 0;

}

int verifyMsg(unsigned char* buffer,int size){

    if(!buffer)
        return 0;
    if(size>3){
        if(buffer[0]==MARCINI&&verifyPariVert(buffer,size)){
            return 1;

        }
    }

    return 0;

}

void setMsgAttr(mensagem_t* m,unsigned char tamanho,unsigned char sequencia, unsigned char tipo){

    if(!m)
        return;

    m->tamanho = tamanho;
    m->sequencia = sequencia;
    m->tipo = tipo;

}


void addToSeq(unsigned char* seq,int a){

    *seq=(*seq+a)%64;

}

unsigned char getSeqAdding(unsigned char* seq,int a){

    return (*seq+a)%64;

}

long long timestamp() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}
 
int protocoloValido(unsigned char* buffer, int buffer_size, unsigned char tipo,unsigned char* seq) {
    if (buffer_size <= 0) 
        return 0; 
    
    mensagem_t m;   
    if(verifyMsg(buffer,buffer_size)) {
        separateMessage(&m, buffer);
        if(*seq==m.sequencia){
            //printf("recebido msg com seq valida %d e tipo %d, buscando %d\n",m.sequencia,m.tipo,tipo);
            if(m.tipo==tipo){
                //printf("eeeh\n");
                if(m.dados[0]==*seq){
                    //printf("mensagem %d recebida, tipo %d\n",m.dados[0],m.tipo);
                    return 1;
                }else
                    return 0;
               

                return 1;
            }else if(m.tipo==NACK||m.tipo==ERROR){
                return -1;
            }
        }    
    }

    return 0;
}
 
// retorna -1 se deu timeout, ou quantidade de bytes lidos
int recvMensagem(int s, unsigned char tipo, unsigned char* seq) {
    int r;
    unsigned char buffer[MAXBUFF];
    long long comeco = timestamp();
    struct timeval timeout = { .tv_sec = TIMEOUTMILLIS/1000, .tv_usec = (TIMEOUTMILLIS%1000) * 1000 };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    int bytes_lidos;
    do {
        bytes_lidos = recv(s, buffer, MAXBUFF, 0);
        r=protocoloValido(buffer, bytes_lidos,tipo,seq);
        if (r==1) {
            //printf("oiii? %d\n",bytes_lidos);
            return bytes_lidos; 
        }else if(r==-1){
           //*seq=*seq+1;
            //printf("algo deu errado!\n");
            return -1;
        }
    } while (timestamp() - comeco <= TIMEOUTMILLIS);
    //printf("deu timeout!!\n");
    //*seq=*seq-1;
    return -1;
}
