#define SERPATH "./serverfolder"

void receiveBackup(int s,mensagem_t* m,unsigned char* seq,char* current_path);
void verifyFileServer(int s,char* filename,char* current_path,unsigned char* seq);
