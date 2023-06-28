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
#include"commonFunc.h"
#include"serFunc.h"
#include"cliFunc.h"


int main(){

    int s = create_socket("lo");//eno1
    char current_path[MAXPATH]=SERPATH;
    int ended=0;
    int size;
    unsigned char seq=0;
    mensagem_t m;
    unsigned char buffer[MAXBUFF];
    memset(buffer,'0', MAXBUFF);

    printf("==================Server Iniciado==================\n");
    printf("%s[%s]%s\n",ANSI_COLOR_CYAN,current_path,ANSI_COLOR_RESET);
    do{
        
        size=recv(s,buffer, MAXBUFF, 0);
        if(size>0&&verifyMsg(buffer,size)){
            separateMessage(&m, buffer);
            if(m.sequencia==seq){
                switch(m.tipo){

                    case BACKUP:
                    case BACKUPN:  
                        receiveBackup(s,&m,&seq,current_path);
                        printf("%s[%s]%s\n",ANSI_COLOR_CYAN,current_path,ANSI_COLOR_RESET);
                    break;

                    case RECOVER:
                    case RECOVERN:
                        sendEmpty(s,seq,ACK);
                        addToSeq(&seq,1);   
                        backup(s,(char*)m.dados,current_path,&seq);
                        printf("%s[%s]%s\n",ANSI_COLOR_CYAN,current_path,ANSI_COLOR_RESET);
                    break;

                    case CHOOSE_SER_DIR:   
                        custom_cd((char*)m.dados,current_path,SERPATH);
                        sendEmpty(s,seq,ACK);
                        addToSeq(&seq,1);
                        printf("%s[%s]%s\n",ANSI_COLOR_CYAN,current_path,ANSI_COLOR_RESET);
                    break;

                    case VERIFY:   
                        verifyFileServer(s,(char*)m.dados,current_path,&seq);
                        printf("%s[%s]%s\n",ANSI_COLOR_CYAN,current_path,ANSI_COLOR_RESET);
                    break;
                    case ENDOF:
                    case ENDGF:
                        sendEmpty(s,seq-1,ACK);
                    break;
                    default:   

                    break;

                }

                
            }else if(m.sequencia==getSeqAdding(&seq,-1)){
                if(m.tipo!=ACK&&m.tipo!=NACK&&m.tipo!=VERIFY)
                    sendEmpty(s,seq-1,ACK);
                else if(m.tipo==VERIFY){
                    addToSeq(&seq,-1);
                    verifyFileServer(s,(char*)m.dados,current_path,&seq);
                    printf("%s[%s]%s\n",ANSI_COLOR_CYAN,current_path,ANSI_COLOR_RESET);

                }else if(m.tipo==ACK){
                    sendEmpty(s,seq,END);
                }
            }
        }
    }while(!ended);

    return 0;

}
