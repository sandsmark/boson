#include <lib3ds/file.h>
#include <lib3ds/node.h>
#include <lib3ds/mesh.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define false 0
#define true 1

/*
 * We use a hardcoded .3ds file. This is necessary, as we must hardcode the
 * "should be" values into this file.
 */
const char* g_fileName = "mob_puma.3ds";

/*
 * All node names in the .3ds file in the order this program processes them
 */
const char* g_nodeNames[] = {
	"rocketbo10",
	"rocketbo11",
	"rocketbo12",
	"rocketbo13",
	"rocketbo14",
	"rocketbo15",
	"rocketbo16",
	"rocketbo17",
	"rocketbo18",
	"rocketbo19",
	"rocketbo20",
	"rocketbo21",
	"rocketbo22",
	"rocketbo23",
	"rocketbo24",
	"rocketbo25",
	"rocketbo26",
	"rocketbox0",
	"rocketbox2",
	"rocketbox3",
	"rocketbox4",
	"rocketbox5",
	"rocketbox6",
	"rocketbox7",
	"rocketbox8",
	"rocketbox9",
	"$$$DUMMY",
	"rocketbo10",
	"rocketbo11",
	"rocketbo12",
	"rocketbo13",
	"rocketbo14",
	"rocketbo15",
	"rocketbo16",
	"rocketbo17",
	"rocketbo18",
	"rocketbo19",
	"rocketbo20",
	"rocketbo21",
	"rocketbo22",
	"rocketbo23",
	"rocketbo24",
	"rocketbo25",
	"rocketbo26",
	"rocketbox0",
	"rocketbox2",
	"rocketbox3",
	"rocketbox4",
	"rocketbox5",
	"rocketbox6",
	"rocketbox7",
	"rocketbox8",
	"rocketbox9",
	"$$$DUMMY",
	"Box05",
	"Cylinder02",
	"Sphere01",
	"window",
	"Nose",
	"Box02",
	"Box02",
	"$$$DUMMY",
	"Box14",
	"Box14",
	"Box01",
	"Heckflosse",
	"Tail",
	"Box12",
	"Box12",
	"rotorHead",
	"rotorHandl",
	"Box15",
	0
};

/*
 * All mesh names in the .3ds file in the order this program processes them
 */
const char* g_meshNames[] = {
	"Box01",
	"Box02",
	"Box05",
	"Box12",
	"Box14",
	"Box15",
	"Cylinder02",
	"Heckflosse",
	"Nose",
	"Sphere01",
	"Tail",
	"rocketbo10",
	"rocketbo11",
	"rocketbo12",
	"rocketbo13",
	"rocketbo14",
	"rocketbo15",
	"rocketbo16",
	"rocketbo17",
	"rocketbo18",
	"rocketbo19",
	"rocketbo20",
	"rocketbo21",
	"rocketbo22",
	"rocketbo23",
	"rocketbo24",
	"rocketbo25",
	"rocketbo26",
	"rocketbox0",
	"rocketbox2",
	"rocketbox3",
	"rocketbox4",
	"rocketbox5",
	"rocketbox6",
	"rocketbox7",
	"rocketbox8",
	"rocketbox9",
	"rotorHandl",
	"rotorHead",
	"window",
	0
};

/*
 * All "mesh->points" values from the .3ds file in the same order as the mesh
 * names above.
 */
unsigned long g_meshPointCount[] = {
	195,
	32,
	65,
	29,
	32,
	80,
	15,
	50,
	71,
	29,
	128,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	4,
	3,
	4,
	3,
	4,
	3,
	4,
	3,
	4,
	4,
	4,
	3,
	4,
	3,
	4,
	3,
	4,
	3,
	4,
	15,
	66,
	35,
	0
};

/*
 * We store a few mesh->pointL[(index/3)].pos[(index % 3)] values here. Due to
 * the immense amount of data, we store only a few entries of the array of the
 * first mesh.
 */
int g_meshPoints_count = 21;
float g_meshPoints[] = {
	-5.000018,
	-394.999969,
	95.000015,
	-0.000017,
	-394.999969,
	95.000015,
	4.999982,
	-394.999969,
	95.000015,
	-5.000020,
	-375.000000,
	90.000000,
	-0.000020,
	-375.000000,
	90.000000,
	4.999980,
	-375.000000,
	90.000000,
	-10.000022,
	-339.999969,
	84.999992,
	0.0f
};

static int checkNode(Lib3dsNode* node, int* nodeIndex);
static int checkMesh(Lib3dsMesh* mesh, int meshIndex);

int main()
{
 Lib3dsFile* file = 0;
 Lib3dsNode* node = 0;
 Lib3dsMesh* mesh = 0;
 int nodeIndex = 0;
 int meshIndex = 0;
 file = lib3ds_file_load(g_fileName);
 if (!file) {
	printf("NULL file - could not load %s\n", g_fileName);
	return 1;
 }
 printf("File %s loaded successfully\n", g_fileName);

 printf("Checking nodes...\n");
 nodeIndex = 0;
 for (node = file->nodes; node; node = node->next) {
	if (!checkNode(node, &nodeIndex)) {
		printf("FAILED\n");
		return 1;
	}
 }
 printf("Node check succeeded.\n");

 printf("Checking meshes...\n");
 meshIndex = 0;
 for (mesh = file->meshes; mesh; mesh = mesh->next) {
	if (!checkMesh(mesh, meshIndex)) {
		printf("FAILED\n");
		return 1;
	}
	meshIndex++;
 }
 printf("Mesh check succeeded.\n");

 printf("SUCCESS\n");
 return 0;
}

static int checkNode(Lib3dsNode* node, int* nodeIndex)
{
 Lib3dsNode* n;
 for (n = node->childs; n; n = n->next) {
	if (!checkNode(n, nodeIndex)) {
		return false;
	}
 }
 if (g_nodeNames[*nodeIndex] == 0) {
	printf("more nodes than expected\n");
	return false;
 }
 if (strcmp(g_nodeNames[*nodeIndex], node->name) != 0) {
	printf("unexpected name for node %d. Have: %s. Expected: %s.\n", *nodeIndex, node->name, g_nodeNames[*nodeIndex]);
	return false;
 }
 (*nodeIndex)++;
 return true;
}

static int checkMesh(Lib3dsMesh* mesh, int meshIndex)
{
 if (g_meshNames[meshIndex] == 0) {
	printf("more meshes  than expected\n");
	return false;
 }
 if (strcmp(g_meshNames[meshIndex], mesh->name) != 0) {
	printf("unexpected name for mesh %d. Have: %s. Expected: %s.\n", meshIndex, mesh->name, g_meshNames[meshIndex]);
	return false;
 }
 if (g_meshPointCount[meshIndex] != mesh->points) {
	printf("unexpected points for mesh %d. Have: %lu. Expected: %lu.\n", meshIndex, mesh->points, g_meshPointCount[meshIndex]);
	return false;
 }
 if (meshIndex == 0) {
	int i = 0;
	for (i = 0; i + 2 < g_meshPoints_count && (i / 3) < mesh->points; i += 3) {
		float m0 = mesh->pointL[(i / 3)].pos[0];
		float m1 = mesh->pointL[(i / 3)].pos[1];
		float m2 = mesh->pointL[(i / 3)].pos[2];
		float v0 = g_meshPoints[i + 0];
		float v1 = g_meshPoints[i + 1];
		float v2 = g_meshPoints[i + 2];
		if (fabsf(v0 - m0) > 0.001f || fabsf(v1 - m1) > 0.001f || fabsf(v2 - m2) > 0.001f) {
			printf("Invalid point %d. Have: %f %f %f. Expected: %f %f %f.\n", (i/3), m0, m1, m2, v0, v1, v2);
			return false;
		}
	}
 }
 return true;
}

