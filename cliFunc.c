#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include"cliFunc.h"
#include"commonFunc.h"

void menu(){

    printf("------------------------------Menu-------------------------------\n");
    printf("| Comandos:                                                     |\n");
    printf("| backup <nome_arquivo>                                         |\n");
    printf("| backup <nome_arquivo> <nome_arquivo> <nome_arquivo>           |\n");
    printf("| recover <nome_arquivo>                                        |\n");
    printf("| recover <nome_arquivo> <nome_arquivo> <nome_arquivo>          |\n");
    printf("| defdir <nome_dir>                                             |\n");
    printf("| verify <nome_arquivo>                                         |\n");    
    printf("| exit                                                          |\n");
    printf("-----------------------------------------------------------------\n");

}

int indexOf(char* string, char c){

    char *p = strchr(string, c);
    if(p==NULL)
        return -1;
    
    return p - string;

}

int lastIndexOf(char* string, char c){

    char *p = strrchr(string, c);
    if(p==NULL)
        return -1;
    
    return p - string;

}

void custom_ls(DIR *dirstream){

    rewinddir (dirstream);
    struct dirent *dir_t = readdir(dirstream);
    while(dir_t){
        if(dir_t->d_type==8)
            printf("%s  ",dir_t->d_name);
        
        if(dir_t->d_type==DT_DIR)
            if(dir_t->d_name[0]!='.')
                printf("%s%s%s   ",ANSI_COLOR_BLUE,dir_t->d_name,ANSI_COLOR_RESET );
        
        
        dir_t = readdir(dirstream);

    }
    printf("\n");

}


void custom_cd(char* path,char* current_path){

    if(strcmp(path,"\0")==0){
        strcpy(current_path,"./");
        return;

    }
    int div=lastIndexOf(path,'\0')-1;
    
    while(path[div]==' ')
        div--;
    path[div+1]='\0';

    if(strcmp(path,"..")==0){
        if(strcmp(current_path,"./")!=0){
            div = lastIndexOf(current_path,'/');
            if(div==1)
                div=2;
            current_path[div]='\0';
        }
    }else if(strcmp(path,"..")!=0){
        div=strlen(current_path);
        if(div+strlen(path)<MAXPATH){
            if(strcmp(current_path,"./")!=0)
                strcat (current_path, "/");
            strcat (current_path, path);
            DIR *dirs= openDir(current_path);
            if(!dirs){
                perror("erro na abertura do diretorio");
                current_path[div]='\0';
            }
            closedir (dirs);
        }

    }



}

void backup(int s,char* filename,char* current_path,unsigned char* seq){

    char path[MAXPATH];
    strcpy(path, current_path);

    if(strcmp(current_path,"./")!=0)
        strcat (path, "/");
    strcat (path, filename);
    FILE *arq = openFile(path,"rb");
    if(!arq){
        perror("Erro na abertura do arquivo");
        return;
    }
        
    mensagem_t m;
    int size;
    unsigned char buffer[MAXBUFF];
    memset(buffer, 0, MAXBUFF);

    setMsgAttr(&m,strlen(filename)+1,*seq, BACKUP);
    memcpy((char*)m.dados, filename,strlen(filename));
    m.dados[strlen(filename)]='\0';
    size=fillBuffer(&m,buffer);
    do{
        if(send(s, buffer,size, 0)<0)
            perror("erro no envio da mensagem:");
        
        printf("mensagem %d enviada! tipo(%d) tam(%d)\n",*seq,m.tipo,m.tamanho);
        //addToSeq(seq,1);
    }while(recvMensagem(s,OK,seq)<0);
    addToSeq(seq,1);

    while(! feof(arq)){
        size=fread (m.dados, sizeof(unsigned char), MAXDATA, arq);
        printf("teste\n");
        setMsgAttr(&m,size,*seq, DATA);
        size=fillBuffer(&m,buffer);
        do{
            
            
            if(send(s, buffer, size, 0)<0)
                perror("erro no envio da mensagem:");
       
            printf("mensagem %d enviada! tipo(%d) tam(%d)\n",*seq,m.tipo,m.tamanho);
            //addToSeq(seq,1);
        }while(recvMensagem(s, ACK,seq)<0);
        addToSeq(seq,1);
    }

    do{

        sendEmpty(s,*seq,ENDOF);
        //addToSeq(seq,1);

    }while(recvMensagem(s,ACK,seq)<0);
    addToSeq(seq,1);

}


void replaceFirst(char* s,char c1, char c2){

    char *p=strchr(s,c1);
    *p=c2;

}



char* copyString(char* s,int num){

    char* string = malloc(sizeof(char)*(num+1));
    for(int i=0;i<num;i++){
        string[i]=s[i];
    }
    string[num]='\0';

    return string;

}

void firstWord(char** word,char* s){

    int i=0;

    while(s[i]==' ')
        i++;

    strcpy(s,&s[i]);

    i=1;

    int div = indexOf(s,' ');
    if(div<0){
        *word = copyString(s,strlen(s));
        s[0]='\0';
        return;

    }
    *word = copyString(s,div);
    
    while(s[div+i]==' ')
        i++;

    strcpy(s,&s[div+i]);
}





