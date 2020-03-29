#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include "renamer.h"

const char* DEST = "/media/Archive8_6/pnew/";
//const char* DEST = "/home/mitchell/pnew/";

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		printf("Please give one argument for the path");
		return -1;
	}

	const char* dirpath = argv[1];
	DIR *dir = opendir(dirpath);
	if (dir != NULL)
	{
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
		{
			// check if ent is a directory
			if (ent->d_type == DT_DIR)
			{
				if (strcmp(ent->d_name, ".") == 0 ||
					strcmp(ent->d_name, "..") == 0)
				{
					continue;
				}

				printf("handledirectory(%s, %s)\n", dirpath, ent->d_name);
				handledirectory(dirpath, ent->d_name);
			}
			else if (ent->d_type == DT_REG)
			{
				handlefile(dirpath, ent->d_name);
			}
			else
			{
				printf("Unsure what to do with '%s'\n", ent->d_name);
			}
		}
		closedir (dir);
	}
	else
	{
		perror ("Could not open directory");
		return -1;
	}
}

void handledirectory(const char* dirpath, char* ent)
{
	char* dirpathnew = (char*)malloc(sizeof(char) * strlen(dirpath) + strlen(ent) + 2);
	sprintf(dirpathnew, "%s/%s", dirpath, ent);

	DIR *dir = opendir(dirpathnew);
	if (dir != NULL)
	{
		struct dirent *entnew;
		while ((entnew = readdir(dir)) != NULL)
		{
			if (entnew->d_type == DT_REG)
			{
				char* orig = (char*)malloc(sizeof(char) * strlen(dirpathnew) + strlen(entnew->d_name) + 2);
				char* dest = (char*)malloc(sizeof(char) * strlen(dirpath) + strlen(entnew->d_name) + 2);
				sprintf(orig, "%s/%s", dirpathnew, entnew->d_name);
				sprintf(dest, "%s/%s", dirpath, entnew->d_name);

				printf("mv %s %s\n", orig, dest);
				rename(orig, dest);

				printf("handlefile %s %s\n", dirpath, dest);
				handlefile(dirpath, entnew->d_name);

				free(orig);
				free(dest);
			}
			else if (entnew->d_type == DT_DIR &&
					(strcmp(entnew->d_name, ".") == 0 ||
					 strcmp(entnew->d_name, "..") == 0))
			{
				continue;
			}
			else
			{
				printf("Unsure what to do with '%s'\n", entnew->d_name);
			}
		}
		closedir(dir);
	}
	else
	{
		perror("Could not open directory");
	}

	free(dirpathnew);
}

void handlefile(const char* dirpath, char* ent)
{
	int len = strlen(ent);
	if (len > 5 &&
		strcmp(ent + (len - 5), ".part") == 0)
	{
		printf("Skipping partial file: %s\n", ent);
		return;
	}

	printf("inside handlefile %s %s\n", dirpath, ent);
	struct AuthorFile authorfile;
	if (hasauthor(ent, &authorfile))
	{
		char* author = (char*)malloc(sizeof(char) * authorfile.end - authorfile.start);
		getauthor(ent, authorfile, author);
		char* orig = (char*)malloc(sizeof(char) * strlen(dirpath) + strlen(ent) + 2);
		char* dest = (char*)malloc(sizeof(char) * strlen(DEST) + strlen(author) + strlen(ent) + 2);
		char* subdest = (char*)malloc(sizeof(char) * strlen(DEST) + strlen(author) + 1);
		sprintf(orig, "%s/%s", dirpath, ent);
		sprintf(dest, "%s%s/%s", DEST, author, ent);
		sprintf(subdest, "%s%s", DEST, author);

		if (opendir(subdest) == NULL)
		{
			printf("mkdir %s\n", subdest);
			mkdir(subdest, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		}
		// see if dest dirs exist

		printf("mv %s %s\n", orig, dest);
		rename(orig, dest);
		free(author);
		free(orig);
		free(dest);
		free(subdest);
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
