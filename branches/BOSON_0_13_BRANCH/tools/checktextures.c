/*
    This file is part of the Boson game
    Copyright (C) 2002 The Boson Team (boson-devel@lists.sourceforge.net)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <lib3ds/file.h>
#include <lib3ds/material.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

char** textures;
int textureCount;

void usage(const char* command);

// find .3ds files in dir and then parse the files using parseModel().
void findTexturesInModels(const char* dir);

// parse model and call addTexture for every texture in it
void parseModel(const char* file);
void addTexture(const char* file);
void compareTextures(const char* texturesDir);
int checkPresence(const char* textureFile); // 0 == not there, 1 == its there

void findTextureDir(const char* dir, char* textures);

int main(int argc, const char** argv)
{
 int i;
 int arg;
 int pathIndex = 0;
 char texturesDir[255];
 textures = (char**)malloc(sizeof(void*) * 200); // we can store up to 200 textures here
 textureCount = 0;
 texturesDir[0] = '\0';

 arg = 0;
 for (i = 1; i < argc; i++) {
	if (strcmp("-t", argv[i]) == 0) {
		arg = 1;
		continue;
	}
	if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
		usage(argv[0]);
		exit(0);
	}
	switch (arg) {
		case 0:
			// the arg is the path to the data files
			if (pathIndex != 0) {
				printf("WARNING: invalid parameters!\n");
			}
			pathIndex = i;
			break;
		case 1:
			strncpy(texturesDir, argv[i], 255);
			break;
		default:
			// oops - what happened here??
			break;
	}
	arg = 0;
	
 }
 printf("Searching for files in %s\n", pathIndex == 0 ? "." : argv[pathIndex]);
 findTexturesInModels(pathIndex == 0 ? "." : argv[pathIndex]);
 findTextureDir(pathIndex == 0 ? "." : argv[pathIndex], texturesDir);

 if (texturesDir[0] == '\0') {
	printf("Error: Cannot find textures subdir\n");
	exit(1);
 }

 compareTextures(texturesDir);

 for (i = 0; i < textureCount; i++) {
	free(textures[i]);
 }
 textureCount = 0;
 free(textures);
 return 0;
}

void usage(const char* command)
{
 printf("Usage: %s [OPTIONS] [PATH to data]\n", command);
 printf("Options:\n");
 printf("	-t PATH		path to the textures:\n");
}

void findTexturesInModels(const char* path)
{
 DIR* dir;
 struct dirent* entry = 0;
 struct stat buf;
 char filename[255];
 int i;
 dir = opendir(path);
 if (!dir) {
	printf("Could not open %s (while searching for models)\n", path);
	return;
 }
 while ((entry = readdir(dir)) != 0) {
	if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
		continue;
	}
	strncpy(filename, path, 154);
	strncat(filename, "/", 1);
	strncat(filename, entry->d_name, 100);
	stat(filename, &buf);
	if (S_ISDIR(buf.st_mode)) {
		findTexturesInModels(filename);
	} else if (S_ISREG(buf.st_mode)) {
		for (i = 0; i < 255 && entry->d_name[i] != '\0'; i++) {
			if (strcmp(&entry->d_name[i], ".3ds") == 0) {
				parseModel(filename);
			}
		}
	}
 }
 closedir(dir);
}

void parseModel(const char* filename)
{
 Lib3dsFile* file;
 Lib3dsMaterial* mat = 0;
 file = lib3ds_file_load(filename);
 if (!file) {
	printf("Error: could not load file %s\n", filename);
	return;
 }
 for (mat = file->materials; mat; mat = mat->next) {
	addTexture(mat->texture1_map.name);
 }
}

void addTexture(const char* filename)
{
 int i;
 if (strcmp(filename, "") == 0) {
	return;
 }
 for (i = 0; i < textureCount; i++) {
	if (strcasecmp(filename, textures[i]) == 0) {
		return;
	}
 }
 if (textures[textureCount]) {
	printf("Error: memory at count=%d already used\n", textureCount);
	return;
 }
 textures[textureCount] = (char*)malloc(sizeof(char) * 255);
 strncpy(textures[textureCount], filename, 255);
 textureCount++;
}

void findTextureDir(const char* path, char* textures)
{
 DIR* dir;
 struct dirent* entry = 0;
 struct stat buf;
 char filename[255];
 int i;
 dir = opendir(path);
 if (!dir) {
	printf("Could not open %s (while searching for texture dir)\n", path);
	return;
 }

 // go through every entry searching for a "textures" subdir. once it is found
 // we return it in textures.
 while ((entry = readdir(dir)) != 0 && textures[0] == '\0') {
	if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
		continue;
	}
	strncpy(filename, path, 154);
	strncat(filename, "/", 1);
	strncat(filename, entry->d_name, 100);
	stat(filename, &buf);
	if (S_ISDIR(buf.st_mode)) {
		if (strncmp(entry->d_name, "textures", 10) == 0) {
			strcpy(textures, filename);
		} else {
			findTextureDir(filename, textures);
		}
	}
 }
 closedir(dir);
}

void compareTextures(const char* path)
{
 DIR* dir;
 struct dirent* entry = 0;
 struct stat buf;
 char filename[255];
 int i;
 dir = opendir(path);
 if (!dir) {
	printf("Could not open %s (while comparing textures)\n", path);
	return;
 }
 while ((entry = readdir(dir)) != 0) {
	if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
		continue;
	}
	strncpy(filename, path, 154);
	strncat(filename, "/", 1);
	strncat(filename, entry->d_name, 100);
	stat(filename, &buf);
	if (!S_ISREG(buf.st_mode)) {
		// we care about files only
		continue;
	}
	if (strcmp(entry->d_name, ".cvsignore") == 0) {
		continue;
	}
	if (strncmp(entry->d_name, "Makefile", strlen("Makefile")) == 0) {
		continue;
	}
	if (strcmp(entry->d_name, "COPYRIGHT") == 0) {
		continue;
	}

	if (checkPresence(entry->d_name) == 0) {
		printf("Texture seems to be unused: %s\n", entry->d_name);
	}
 }

 closedir(dir);
}

int checkPresence(const char* texture)
{
 int i;
 for (i = 0; i < textureCount; i++) {
	if (strcasecmp(textures[i], texture) == 0) {
		return 1;
	}
 }
 return 0;
}
