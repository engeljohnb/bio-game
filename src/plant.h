#ifndef __PLANT_H__
#define __PLANT_H__
#include "terrain.h"

#define MAX_TERRAIN_ELEMENT_MESHES 4
#define MAX_PLANT_TEXTURES 4

enum PLANT_TYPES
{
	PLANT_TYPE_GRASS = 0,
	PLANT_TYPE_CANOPY,
	PLANT_TYPE_TREE_TRUNK,
	PLANT_TYPE_GENERATED_TREE_TRUNK,
};

//TODO: Remove scale_coefficients
typedef struct Plant
{	
	int			type;
	TerrainElementMesh 	meshes[MAX_TERRAIN_ELEMENT_MESHES];
	int			num_meshes;
	int 			min_temperature;
	int			max_temperature;
	int			ideal_min_temperature;
	int			ideal_max_temperature;
	float 			min_precipitation;
	float 			max_precipitation;
	float			scale_coefficients[MAX_TERRAIN_ELEMENT_MESHES];
	B_Texture		textures[MAX_PLANT_TEXTURES];
	int			num_textures;
} Plant;
#endif
