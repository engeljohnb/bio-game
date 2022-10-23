#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string.h>

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
	
			if (mesh->HasNormals())
			{
				fwrite(&mesh->mNormals[j].x, sizeof(float), 1, fp);
				fwrite(&mesh->mNormals[j].y, sizeof(float), 1, fp);
				fwrite(&mesh->mNormals[j].z, sizeof(float), 1, fp);
			}
			else
			{
				float norm = 0.0;
				fwrite(&norm, sizeof(float), 1, fp);
				fwrite(&norm, sizeof(float), 1, fp);
				fwrite(&norm, sizeof(float), 1, fp);
				fprintf(stderr, "Mesh has no normals\n");
			}
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
	const aiScene *scene = importer.ReadFile(argv[1], aiProcess_FlipUVs | aiProcess_Triangulate |    aiProcess_CalcTangentSpace);
	unsigned int total_bytes = write_vertex_data(fp, scene);
	char bytes_header[512] = {0};
	sprintf(bytes_header, "TOTAL_BYTES: %u\n", total_bytes);
	fwrite(bytes_header, strlen(bytes_header), 1, fp);
	fclose(fp);
	return 0;
}
