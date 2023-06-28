#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <dirent.h>
#include"commonFunc.h"
#include"serFunc.h"
#include"cliFunc.h"
#include  <openssl/md5.h>

void receiveBackup(int s,mensagem_t* m,unsigned char* seq,char* current_path){

    char path[MAXPATH];
    strcpy(path, current_path);

    strcat (path, "/");
    strcat (path, (char*)m->dados);

    FILE *arq = openFile(path,"wb");
    if(!arq){
        sendEmpty(s,*seq,NACK);
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
            if(*seq==m2.sequencia&&m2.tipo!=ACK&&m2.tipo!=NACK){
                
                switch(m2.tipo){

                    case DATA:
                        if(fwrite (m2.dados, sizeof(unsigned char), size-4, arq)) {
                            sendEmpty(s,*seq,ACK);
                            addToSeq(seq,1);
                        }else{
                            sendEmpty(s,*seq,NACK);
                        }
                    break;

                    case ENDOF:
                    case ENDGF:
                        sendEmpty(s,*seq,ACK);
                        endof=1;
                        addToSeq(seq,1);
                    break;

                }

            }else if(getSeqAdding(seq,-1)==m2.sequencia){
                if((m2.tipo==BACKUP||m2.tipo==BACKUPN))
                    sendEmpty(s,getSeqAdding(seq,-1),OK);
                else
                    sendEmpty(s,getSeqAdding(seq,-1),ACK);
            }   
                         

        }
    }while(!endof); 
    printf("Arquivo %s foi transmitido com sucesso.\n",(char*)m->dados);
    fclose(arq);

}

void verifyFileServer(int s,char* filename,char* current_path,unsigned char* seq){

    char path[MAXPATH];
    unsigned char buffer[MAXBUFF];
    mensagem_t m2;
    int size;
    

    if(fileExists(filename, current_path)){

        strcpy(path, current_path);
        strcat (path, "/");
        strcat (path, filename);
        // mensagem de "arquivo existe"
        FILE *fileServer = openFile(path,"rb");
        if(!fileServer){
            sendEmpty(s,*seq,ERROR);
            addToSeq(seq,1);
            return;
        }

        unsigned char *result = getMD5(fileServer); // Gera o MD5 Do arquivo do servidor

        setMsgAttr(&m2,MD5_DIGEST_LENGTH,*seq, MMD5);
        memcpy((char*)m2.dados, result,MD5_DIGEST_LENGTH);
        size=fillBuffer(&m2,buffer);
        if(send(s, buffer,size, 0)<0)
             perror("erro no envio da mensagem:");
        addToSeq(seq,1);

        free(result);
        fclose(fileServer);
    }else{
        sendEmpty(s,*seq,ERROR);
        addToSeq(seq,1);
        printf("Arquivo nao existe\n");
        return;
    }


}
