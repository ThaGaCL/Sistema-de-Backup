#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <dirent.h>
#include"commonFunc.h"
#include"serFunc.h"

void receiveBackup(int s,mensagem_t* m,unsigned char* seq,char* current_path){

    char path[MAXPATH];
    strcpy(path, current_path);

    if(strcmp(current_path,"./")!=0)
        strcat (path, "/");
    strcat (path, (char*)m->dados);

    FILE *arq = openFile(path,"w");
    if(!arq){
        sendEmpty(s,*seq,NACK);
        addToSeq(seq,-1);
        return;
    }

    sendEmpty(s,*seq,OK);
    addToSeq(seq,1);

    unsigned char buffer[MAXBUFF];
    mensagem_t m2;
    int endof=0;
    int size;

    do{
        size=recv(s, buffer, MAXBUFF, 0);
        if(size<0)
            perror("erro no recebimento:");

        if(verifyMsg(buffer,size)){
            separateMessage(&m2, buffer);
            if(*seq==m2.sequencia){
                printf("mensagem %d recebida! tipo(%d) tam(%d)\n",*seq,m2.tipo,m2.tamanho);
                addToSeq(seq,1);
                switch(m2.tipo){

                    case DATA:
                        if(fwrite (m2.dados, sizeof(unsigned char), size-4, arq)) {
                            sendEmpty(s,*seq,ACK);
                            addToSeq(seq,1);
                        }else{
                            sendEmpty(s,*seq,NACK);
                            addToSeq(seq,1);
                        }
                    break;

                    case ENDOF:
                    case ENDGF:
                        sendEmpty(s,*seq,ACK);
                        endof=1;
                        addToSeq(seq,1);
                    break;
                    default:

                        sendEmpty(s,*seq,NACK);
                        addToSeq(seq,1);
                    break;

                }

            }else if(getSeqAdding(seq,-2)==m2.sequencia&&(m2.tipo==BACKUP||m2.tipo==BACKUPN)){
                sendEmpty(s,getSeqAdding(seq,-1),OK);
            }else if(getSeqAdding(seq,-2)==m2.sequencia){
                sendEmpty(s,getSeqAdding(seq,-1),ACK);
            }                   

        }
    }while(!endof); 

    fclose(arq);


}
