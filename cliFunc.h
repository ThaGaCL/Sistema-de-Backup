#define MAXLINE 1024
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_CYAN    "\x1b[96m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void menu();

void custom_ls(DIR *dirstream);

void custom_cd(char* path,char* current_path);

void backup(int s,char* filename,char* current_path,unsigned char *seq);

void firstWord(char** word,char* s);

void replaceFirst(char* s,char c1, char c2);
