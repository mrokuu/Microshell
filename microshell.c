#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"



const char* EXIT_COMMAND = "exit\n";
const char* HELP_COMMAND = "help\n";
const char* CHANGE_DIR_COMMAND = "cd";
const int CHANGE_DIR_COMMAND_SIZE = 2;
const char* COPY_FILE_COMMAND = "cp ";
const int COPY_FILE_COMMAND_SIZE = 3;
const char* MOVE_FILE_COMMAND = "mv ";
const int MOVE_FILE_COMMAND_SIZE = 3;
const char* RUN_PROGRAM_COMMAND = "run ";
const int RUN_PROGRAM_COMMAND_SIZE = 4;

const char* HELP_TEXT = "Autor: "ANSI_COLOR_GREEN"Adam Mroczkowski\n"ANSI_COLOR_RESET
	"Komendy:\n"
	"help - pomoc\n"
	"cd " ANSI_COLOR_CYAN "[dir]" ANSI_COLOR_RESET" - zmiana katalogu, " ANSI_COLOR_CYAN "[dir]" ANSI_COLOR_RESET" - sciezka do katalogu lub "ANSI_COLOR_CYAN"[..]"ANSI_COLOR_RESET" - przechodzenie do katalogu nadrzednego\n"
	"exit - wyjscie z programu\n"
	"cp "ANSI_COLOR_CYAN"[origin]"ANSI_COLOR_RESET" "ANSI_COLOR_CYAN"[destination]"ANSI_COLOR_RESET" - kopiowanie pliku, "ANSI_COLOR_CYAN"[origin]"ANSI_COLOR_RESET" - nazwa pliku w aktualnym katalogu, "ANSI_COLOR_CYAN"[destination]"ANSI_COLOR_RESET" - miejsce do ktego ma byc skopiowany plik\n"
	"mv "ANSI_COLOR_CYAN"[origin]"ANSI_COLOR_RESET" "ANSI_COLOR_CYAN"[destination]"ANSI_COLOR_RESET" - przenoszenie pliku, "ANSI_COLOR_CYAN"[origin]"ANSI_COLOR_RESET" - nazwa pliku w aktualnym katalogu, "ANSI_COLOR_CYAN"[destination]"ANSI_COLOR_RESET" - miejsce do ktego ma byc przeniesiony plik\n"
	"run "ANSI_COLOR_CYAN"[program]"ANSI_COLOR_RESET" "ANSI_COLOR_CYAN"[arg]"ANSI_COLOR_RESET" - uruchamianie programu, "ANSI_COLOR_CYAN"[program]"ANSI_COLOR_RESET" - nazwa programu, "ANSI_COLOR_CYAN"[arg]"ANSI_COLOR_RESET" - dodatkowe argumenty programu";

const char* NO_COMMAND_TEXT = "Komenda nierozpoznana!";
const char* DIR_NOT_EXIST_TEXT = "Podany katalog nie istnieje";
const char* DIR_MISSED_TEXT = "Nie podano nazwy katalogu, nazwa powinna rozpoczyznac sie od znaku /";
const char* FILE_MISSED_TEXT = "Nie podano nazwy pliku!";
const char* FILE_NOT_EXIST_TEXT = "Podany plik nie istnieje!";
const char* PROGRAM_MISSED_TEXT = "Nie podano nazwy programu!";
const char* ARG_MISSED_TEXT = "Nie podano argumentow komendy!";

char* getinput(void);
char* askforcommand(void);
void docommand(char* command);
void showhelp(void);
void shownocommand(void);
void changedirectory(char* dir);
const char* getuser(void);
char* concat(const char *s1, const char *s2);
char* fixpathslashes(char* dir);
void copyfile(char* params, int deletesource);
void copy(FILE* source, FILE* destination);
char* getfilename(char* params);
char* getfilepath(char* filename);
char* getdir(char* params);
void run(char* params);
char** splitargs(char* args);

char* currentdir = "/";

const char* user = "";

int main(int argc, char *argv[])
{
	currentdir = getcwd(NULL, 0);
	user = getuser();

	char* command = askforcommand();
	while(strcmp(command, EXIT_COMMAND) != 0)
	{
		docommand(command);
		free(command);
		command = askforcommand();
	}

	free(currentdir);
	return 0;
}

const char* getuser(void)
{
	uid_t uid = geteuid();
	struct passwd *pw = getpwuid(uid);
	if (pw)
	{
		return pw->pw_name;
	}

	return "";
}

char* askforcommand(void)
{
	printf(ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET ":[" ANSI_COLOR_BLUE "%s" ANSI_COLOR_RESET "]$ ", user, currentdir);
	return getinput();
}

char* getinput(void) {
    char * line = malloc(100);
	char* linep = line;
    size_t lenmax = 100;
	size_t len = lenmax;
    int c;

    while((c = fgetc(stdin)) != EOF)
	{
        if(--len == 0)
		{
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL)
			{
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
        {
            break;
        }
    }

    *line = '\0';
    return linep;
}

void docommand(char* command)
{
	int commandlen = strlen(command);

	if(command[0] == '\n')
	{
		return;
	}
	else if(strcmp(command, HELP_COMMAND) == 0)
	{
		showhelp();
	}
	else if(strncmp(command, CHANGE_DIR_COMMAND, CHANGE_DIR_COMMAND_SIZE) == 0)
	{
		if(commandlen > CHANGE_DIR_COMMAND_SIZE)
		{
			char c = command[CHANGE_DIR_COMMAND_SIZE];
			if(c == 0 || c == '\n')
			{
				return;
			}
			else if(c == ' ')
			{
				char* dirparam = command + CHANGE_DIR_COMMAND_SIZE + 1;
				changedirectory(dirparam);
			}
			else
			{
				shownocommand();
			}
		}
	}
	else if(strncmp(command, COPY_FILE_COMMAND, COPY_FILE_COMMAND_SIZE) == 0)
	{
		char* params = command + COPY_FILE_COMMAND_SIZE;
		copyfile(params, 0);
	}
	else if(strncmp(command, MOVE_FILE_COMMAND, MOVE_FILE_COMMAND_SIZE) == 0)
	{
		char* params = command + MOVE_FILE_COMMAND_SIZE;
		copyfile(params, 1);
	}
	else if(strncmp(command, RUN_PROGRAM_COMMAND, RUN_PROGRAM_COMMAND_SIZE) == 0)
	{
		char* params = command + RUN_PROGRAM_COMMAND_SIZE;
		run(params);
	}
	else if(strncmp(command, COPY_FILE_COMMAND, COPY_FILE_COMMAND_SIZE - 1) == 0 && commandlen == COPY_FILE_COMMAND_SIZE)
	{
		printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, ARG_MISSED_TEXT);
	}
	else if(strncmp(command, MOVE_FILE_COMMAND, MOVE_FILE_COMMAND_SIZE - 1) == 0 && commandlen == MOVE_FILE_COMMAND_SIZE)
	{
		printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, ARG_MISSED_TEXT);
	}
	else if(strncmp(command, RUN_PROGRAM_COMMAND, RUN_PROGRAM_COMMAND_SIZE - 1) == 0 && commandlen == RUN_PROGRAM_COMMAND_SIZE)
	{
		printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, ARG_MISSED_TEXT);
	}
	else
	{
		shownocommand();
	}
}

void showhelp(void)
{
	printf("%s\n", HELP_TEXT);
}

void shownocommand(void)
{
	printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, NO_COMMAND_TEXT);
}

void changedirectory(char* dir)
{
	if(strncmp(dir, ".\n", 2) == 0 || strlen(dir) == 0)
	{
		return;
	}
	else if(strncmp(dir, "..\n", 3) == 0)
	{
		int len = strlen(currentdir);
		if(len > 1)
		{
			while(currentdir[--len] != '/')
				;

			if(len == 0)
			{
				free(currentdir);
				currentdir = (char*) malloc(2);
				currentdir = "/";
			}
			else
			{
				char* tempdir = (char*) malloc(len + 1);
				strncpy(tempdir, currentdir, len);
				tempdir[len] = '\0';
				free(currentdir);
				currentdir = tempdir;
			}
		}
	}
	else
	{
		DIR* newdir;
		if(dir[0] == '/')
		{
			dir[strlen(dir) - 1] = '\0';

			newdir = opendir(dir);
			if(newdir)
			{
				free(currentdir);
				currentdir = (char*) malloc(strlen(dir) + 1);
				strcpy(currentdir, dir);
				currentdir = fixpathslashes(currentdir);
			}
			else
			{
				printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, DIR_NOT_EXIST_TEXT);
			}
		}
		else
		{
			char* tempdir = concat(currentdir, "/");
			tempdir = concat(tempdir, dir);
			tempdir[strlen(tempdir) - 1] = '\0';

			newdir = opendir(tempdir);
			if (newdir)
			{
				free(currentdir);
				currentdir = tempdir;
				closedir(newdir);
				currentdir = fixpathslashes(currentdir);
			}
			else
			{
				free(tempdir);
				printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, DIR_NOT_EXIST_TEXT);
			}
		}
	}
}

char* concat(const char *s1, const char *s2)
{
	char *result = malloc(strlen(s1) + strlen(s2) + 1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

char* fixpathslashes(char* dir)
{
	int len = strlen(dir);
	int k = 0;
	char* tempdir = (char*) malloc(len + 1);
	char* finaldir;

	for(int i = 0; i < len;)
	{
		tempdir[k++] = dir[i];
		if(dir[i] == '/')
		{
			while(++i < len && dir[i] == '/')
				;
		}
		else
		{
			i++;
		}
	}

	if(k > 1 && tempdir[k-1] == '/')
	{
		k--;
	}

	free(dir);
	finaldir = (char*) malloc(k + 1);
	strncpy(finaldir, tempdir, k);
	finaldir[k] = '\0';
	free(tempdir);
	return finaldir;
}

void copyfile(char* params, int deletesource)
{
	char* dir;
	DIR* destinationdir;
	FILE* sourcefile;
	FILE* destinationfile;
	char* filename;
	char* originfilepath;
	char* destfilepath;


	params[strlen(params) - 1] = '\0';


	filename = getfilename(params);
	if(filename == NULL)
	{
		printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, FILE_MISSED_TEXT);
		return;
	}
	originfilepath = getfilepath(filename);


	dir = getdir(params);
	if(dir == NULL)
	{
		printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, DIR_MISSED_TEXT);
		free(originfilepath);
		free(filename);
		return;
	}


	destinationdir = opendir(dir);
	if(destinationdir == NULL)
	{
		printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, DIR_NOT_EXIST_TEXT);
		free(originfilepath);
		free(filename);
		return;
	}
	closedir(destinationdir);


	sourcefile = fopen(getfilepath(filename), "rb");
	if(sourcefile == NULL)
	{
		printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, FILE_NOT_EXIST_TEXT);
		free(originfilepath);
		free(filename);
		return;
	}


	destfilepath = concat(dir, "/");
	destfilepath = concat(destfilepath, filename);
	destinationfile = fopen(destfilepath, "wb");

	printf("Plik zrodlowy: [%s]\n", originfilepath);
	printf("Plik docelowy: [%s]\n", destfilepath);


	copy(sourcefile, destinationfile);
	fclose(sourcefile);
	fclose(destinationfile);

	if(deletesource)
	{

		remove(originfilepath);
	}

	free(originfilepath);
	free(filename);
}


char* getfilename(char* params)
{
	int filenamelen = 0;
	int paramslen = strlen(params);
	char* filename;


	while(filenamelen < paramslen && params[++filenamelen] != ' ')
		;

	if(filenamelen == 0)
	{
		return NULL;
	}
	filename = (char*) malloc(filenamelen + 1);
	strncpy(filename, params, filenamelen);
	filename[filenamelen] = '\0';

	return filename;
}

u
char* getfilepath(char* filename)
{
	char* filepath = concat(currentdir, "/");
	filepath = concat(filepath, filename);
	return filepath;
}


char* getdir(char* params)
{
	char* dir;


	for(int i = 0; i < strlen(params); i++)
	{
		if(params[i] == ' ')
		{
			dir = &params[i + 1];
			break;
		}
	}

	return dir;
}


void copy(FILE* source, FILE* destination)
{
	char buff;
	size_t readcount = fread(&buff, sizeof(buff), 1, source);
	while(readcount > 0)
	{
		fwrite(&buff, readcount, 1, destination);
		readcount = fread(&buff, sizeof(buff), 1, source);
	}
}


void run(char* params)
{
	if (fork() == 0)
	{
		int len = 0;
		char* programname;
		int paramslen = strlen(params);
		char** args = NULL;


		params[paramslen - 1] = '\0';
		paramslen--;

		if(paramslen <= 0)
		{
			printf(ANSI_COLOR_RED "%s\n" ANSI_COLOR_RESET, PROGRAM_MISSED_TEXT);
			return;
		}

		args = splitargs(params);
		execvp(args[0], args);
		free(args);
		exit(0);
	}
    else
	{
		pid_t wpid;
		int status = 0;
		while ((wpid = wait(&status)) > 0)
			;
	}
}


char** splitargs(char* args)
{
    char** result;
    int count = 0;
	int argslen = strlen(args);
    char* lastcomma = NULL;
    const char* delim = " ";


	for(int i = 0; i < argslen; i++)
	{
		if (args[i] == ' ')
        {
            count++;
            lastcomma = &args[i];
        }
	}
    if(lastcomma < (args + argslen - 1))
	{
		count++;
	}


    if (result = malloc(sizeof(char*) * (count + 1)))
    {
        int i = 0;
        char* arg = strtok(args, delim);
        while (arg)
        {

			result[i++] = strdup(arg);
            arg = strtok(0, delim);
        }
        result[i] = NULL;
    }

    return result;
}
