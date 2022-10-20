//#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string.h>
/*
typedef struct
{
	float	position[3];
	float	normal[3];
	float	tex_coords[3];	
} FC_Vertex;

typedef struct
{
	FC_Vertex 	*vertices;
	unsigned int	*faces
	//int		active;
	unsigned int 	num_vertices;
	unsigned int 	num_faces;
	//unsigned int 	vao;
	//unsigned int	vbo;
	//unsigned int	ebo;
	
} FC_Mesh;

typedef struct
{
	mat4	local_space;
	mat4	world_space;
	//mat4	view;  << This is the camear??
	B_Mesh 	meshes[MAX_MESHES];
} FC_Model;
*/

unsigned int write_meshes(FILE *fp, const aiScene *scene, aiNode *node)
{
	unsigned int total_bytes = 0;
	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		unsigned int local_bytes = 0;
		char mesh_header[512] = {0};
		sprintf(mesh_header, "B_MESH:");
		fwrite(mesh_header, strlen(mesh_header), 1, fp);
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		/* Should go { pos.x, pos.y, pos.z, norm.x, norm.y, norm.z, tex.x, tex.y, tex.z } */
		fprintf(stderr, "%i\n\n", mesh->mNumVertices);
		for (int j = 0; j < mesh->mNumVertices; ++j)
		{
			fwrite(&mesh->mVertices[j].x, sizeof(float), 1, fp);
			fwrite(&mesh->mVertices[j].y, sizeof(float), 1, fp);
			fwrite(&mesh->mVertices[j].z, sizeof(float), 1, fp);

			fwrite(&mesh->mNormals[j].x, sizeof(float), 1, fp);
			fwrite(&mesh->mNormals[j].y, sizeof(float), 1, fp);
			fwrite(&mesh->mNormals[j].z, sizeof(float), 1, fp);
			float z_coord = 0;
			if (mesh->mTextureCoords[0])
			{
    				fwrite(&mesh->mTextureCoords[0][j].x, sizeof(float), 1, fp);
    				fwrite(&mesh->mTextureCoords[0][j].y, sizeof(float), 1, fp);
				fwrite(&z_coord, sizeof(float), 1, fp);
			}
			else
			{
				fwrite(&z_coord, sizeof(float), 1, fp);
				fwrite(&z_coord, sizeof(float), 1, fp);
				fwrite(&z_coord, sizeof(float), 1, fp);
			}
			total_bytes += sizeof(float)*9;
			local_bytes += sizeof(float)*9;
		}
	}

	if (node->mNumMeshes > 0)
	{
		char mesh_end_header[512] = {0};
		sprintf(mesh_end_header, "END_MESHES");
		fwrite(mesh_end_header, strlen(mesh_end_header), 1, fp);
	}
	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		unsigned int local_bytes = 0;
		char face_header[512] = {0};
		sprintf(face_header, "B_FACES:");
		fwrite(face_header, strlen(face_header), 1, fp);
		local_bytes = 0;
		for (int j = 0; j < mesh->mNumFaces; ++j)
		{
			aiFace face = mesh->mFaces[j];
			for (int k = 0; k < face.mNumIndices; ++k)
			{
				fwrite(&face.mIndices[k], sizeof(unsigned int), 1, fp);
				total_bytes += sizeof(unsigned int);
				//local_bytes += sizeof(unsigned int);
			}
		}
	}

	if (node->mNumMeshes > 0)
	{
		char face_end_header[512] = {0};
		sprintf(face_end_header, "END_FACES");
		fwrite(face_end_header, strlen(face_end_header), 1, fp);
	}

    	for(int i = 0; i < node->mNumChildren; ++i)
    	{
    	    total_bytes += write_meshes(fp, scene, node->mChildren[i]);
    	}
	return total_bytes;
}
unsigned int write_vertex_data(FILE *fp, const aiScene *scene)
{
	int total_vertex_bytes = 0;
	fwrite("VERTEX_DATA\n", strlen("VERTEX_DATA\n"), 1, fp);
	unsigned int total_bytes = write_meshes(fp, scene, scene->mRootNode);

	return total_bytes;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		fprintf(stderr, "Usage: fileconvert [FILENAME] [OUTPUT]\n");
		return 0;
	}

	char filename[512] = {0};
	if (argc < 3)
	{
		strcpy(filename, "model.bgm");
	}
	else
	{
		memcpy(filename, argv[2], strlen(argv[2]));
	}
	FILE *fp = fopen(filename, "w");
	fseek(fp, 0L, SEEK_SET);


	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(argv[1], aiProcess_Triangulate | aiProcess_FlipUVs);
	unsigned int total_bytes = write_vertex_data(fp, scene);
	char bytes_header[512] = {0};
	sprintf(bytes_header, "TOTAL_BYTES: %u\n", total_bytes);
	fwrite(bytes_header, strlen(bytes_header), 1, fp);
	fclose(fp);
	return 0;
}
