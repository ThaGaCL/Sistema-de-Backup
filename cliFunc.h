#define MAXLINE 1024

#define CLIPATH "./clientfolder"

void menu();

void custom_ls(DIR *dirstream);

void custom_cd(char* path,char* current_path,char* base_path);

void backup(int s,char* filename,char* current_path,unsigned char *seq);

void firstWord(char** word,char* s);

void replaceFirst(char* s,char c1, char c2);

void setSerDir(int s,char* path,unsigned char* seq);

void requestBackup(int s,char* filename,unsigned char* seq);

void verifyFile(int s,char* filename,unsigned char* seq,char* current_path);
