#ifndef STRIPIFIER_H
#define STRIPIFIER_H

#include <lib3ds/types.h>

/**
 * Generic linked list with one data variable. nothing special here. Use
 * @ref new_list_item() to create a new entry
 **/
struct List {
	struct List* next;
	int data;
};

/**
 * This struct represents a face and its adjacent faces. The variables f1, f2,
 * f3 are linked lists that contain all all faces that are adjacent to the
 * points (0 and 1 for f1 ; 0 and 2 for f2 ; 1 and 2 for f3).
 **/
struct Face {
	struct List* f1; // adjacent at points 0-1
	struct List* f2; // adjacent at points 0-2
	struct List* f3; // adjacent at points 1-2
	int index; // index of this mesh.
};

struct StripifyData {
	struct Face* allFaces;
	int allFacesCount;
	int* facesTaken; // facesTaken[i] == 1 <=> face i is in a strip

	int usedFacesCount;
};

// list functions
struct List* new_list_item(struct List* previous); // AB: don't return list - use double pointer in parameter instead
/**
 * Append to @p list and return the beginning of the list (handy when @p list is
 * NULL!)
 **/
struct List* append_to_list(struct List* list, int data);
void free_list_item(struct List* list);
void debug_list(struct List* list);
int has_item(struct List* list, int data);

// face functions
struct Face* new_face_item();
void dumpAdjacent(Lib3dsMesh* mesh, struct Face* faces, int facesCount);
void dumpFaceList(Lib3dsMesh* mesh, struct List* first);

// stripify functions
void newStripifyData(struct StripifyData* data, struct Face* faces, int facesCount);
void markFaceUsed(struct StripifyData* data, int face);

#endif

