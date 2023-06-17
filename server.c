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


int main(){

    int s = create_socket("lo");//eno1
    char current_path[MAXPATH]="./";
    int ended=0;
    int size;
    unsigned char seq=0;
    mensagem_t m;
    unsigned char buffer[MAXBUFF];
    memset(buffer,'0', MAXBUFF);

    do{
        size=recv(s,(void*) buffer, MAXBUFF, 0);
        if(size<=0)
            perror("erro no recebimento:");
        else if(verifyMsg(buffer,size)){
            
            separateMessage(&m, buffer);
            if(m.sequencia==seq){
                addToSeq(&seq,1);
                switch(m.tipo){

                    case BACKUP:
                    case BACKUPN:  
                        receiveBackup(s,&m,&seq,current_path);
                    break;

                    case RECOVER:
                    case RECOVERN:   

                    break;

                    case CHOOSE_SER_DIR:   

                    break;

                    case VERIFY:   

                    break;

                    case FILENAME_RECOVER:   

                    break;

                    case MD5:   

                    break;
                    case ENDOF:
                    case ENDGF:
                        sendEmpty(s,seq,ACK);
                    break;
                    default:   

                    break;

                }


            }else{
                sendEmpty(s,seq,NACK);
            }
        }else if(m.sequencia==getSeqAdding(&seq,-2)){
            sendEmpty(s,seq-1,ACK);
        }
    }while(!ended);

    return 0;

}

