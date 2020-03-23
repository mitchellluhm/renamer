#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "renamer.h"

extern int errno;

const char* DEST = "/media/mitchell/Archive8_6/pnew/";
//const char* DEST = "/home/mitchell/pnew/";

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Please give one argument for the path");
		return -1;
	}

	char* dirpath = argv[1];
	int addSlash = dirpath[strlen(dirpath) - 1] != '/';
	DIR *dir = opendir(dirpath);
	if (dir != NULL)
	{
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
		{
			//printf ("%s\n", ent->d_name);
			struct AuthorFile authorfile;
			if (hasauthor(ent->d_name, &authorfile))
			{
				char* author = (char*)malloc(sizeof(char) * (authorfile.end - authorfile.start));
				getauthor(ent->d_name, authorfile, author);

				// allocate space for original location, full destinate, destination without last part
				char* orig;
				if (addSlash)
				{
					orig = (char*)malloc(sizeof(char) * (strlen(dirpath) + strlen(ent->d_name) + 2));
					sprintf(orig, "%s/%s", dirpath, ent->d_name);
				}
				else
				{
					orig = (char*)malloc(sizeof(char) * (strlen(dirpath) + strlen(ent->d_name) + 1));
					sprintf(orig, "%s%s", dirpath, ent->d_name);
				}
				char* dest = (char*)malloc(sizeof(char) * (strlen(DEST) + strlen(author) + strlen(ent->d_name) + 2));
				char* subdest = (char*)malloc(sizeof(char) * (strlen(DEST) + strlen(author) + 1));

				// populate the strings
				sprintf(dest, "%s%s/%s", DEST, author, ent->d_name);
				sprintf(subdest, "%s%s", DEST, author);

				printf("REALSIES:\n%s\n%s\n", orig, dest);
				printf("SUB: %s\n", subdest);

				// see if the dest path has the author directory already
				if (opendir(subdest) == NULL)
				{
					if (mkdir(subdest, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
					{
						printf("Successfully created path '%s'\n", subdest);
					}
					else
					{
						perror("Error creating directory");
					}

				}

				if (rename(orig, dest) == 0)
				{
					printf("Successfully renamed '%s' to '%s'\n", orig, dest);
				}
				else
				{
					perror("Error renaming");
				}

				// clean up memory
				free(author);
				free(orig);
				free(dest);
				free(subdest);
			}
		}
		closedir (dir);
	}
	else
	{
		/* could not open directory */
		perror ("");
		return -1;
	}
}

void getauthor(char* fname, struct AuthorFile afile, char* author)
{
	int j = 0;
	for (int i = afile.start + 1; i < afile.end; i++)
	{
		if (isalpha(fname[i]) || isdigit(fname[i]))
		{
			author[j++] = tolower(fname[i]);
		}
	}
	author[j] = '\0';
	printf("AUTHOR: %s\n", author);
}

int hasauthor(char* fname, struct AuthorFile *afile)
{
	int len = strlen(fname);
	for (int i = 0; i < len; i++)
	{
		if (fname[i] == '_')
		{
			for (int j = len; j > i; j--)
			{
				if (fname[j] == '_')
				{
					afile->start = i;
					afile->end = j;
					return 1;
				}
			}
			return 0;
		}
	}
	return 0;
}

// todo testing
// hasauthor one _, more than two _
