#ifndef __PLANT_H__
#define __PLANT_H__
#include "terrain.h"

enum PLANT_TYPES
{
	PLANT_TYPE_GRASS = 0,
	PLANT_TYPE_CANOPY,
};
typedef struct Plant
{	
	int			type;
	TerrainElementMesh 	*meshes;
	int			num_meshes;
	int 			min_temperature;
	int			max_temperature;
	int			ideal_min_temperature;
	int			ideal_max_temperature;
	float 			min_precipitation;
	float 			max_precipitation;

} Plant;
#endif
