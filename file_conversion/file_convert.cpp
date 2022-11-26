#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cglm/cglm.h>
#include <assimp/Importer.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

typedef struct
{
	int		bone_id;
        mat4            current_transform;
        float           current_timestamp;
        int             num_rotation_keys;
        int             num_position_keys;
        int             num_scale_keys; 
        vec4            *rotation_keys;
        vec3          	*scale_keys;
        vec3            *position_keys;
} Animation;

typedef struct
{
	int	id;
	float	weight;
} BoneID;


void write_animations(FILE *fp, const aiScene *scene, aiNode *node)
{
	for (int i = 0; i < scene->mNumAnimations; ++i)
	{
		aiAnimation *ai_animation = scene->mAnimations[i];
		for (int j = 0; j < ai_animation->mNumChannels; ++j)
		{
			Animation animation;
			aiNodeAnim *channel = ai_animation->mChannels[j];
			memset(&animation, 0, sizeof(Animation));

			animation.bone_id = j;
			animation.num_rotation_keys = channel->mNumRotationKeys;
			animation.num_position_keys = channel->mNumPositionKeys;
			animation.num_scale_keys = channel->mNumScalingKeys;

			animation.position_keys = (vec3 *)malloc(sizeof(vec3)*channel->mNumPositionKeys);
			animation.scale_keys = (vec3 *)malloc(sizeof(vec3)*channel->mNumScalingKeys);
			animation.rotation_keys = (vec4 *)malloc(sizeof(vec4)*channel->mNumRotationKeys);
			for (int k = 0; k < channel->mNumPositionKeys; ++k)
			{
				aiVector3D a_position = channel->mPositionKeys[k].mValue;
				vec3 position = {a_position.x, a_position.y, a_position.z};
				glm_vec3_copy(position, animation.position_keys[k]);
			}
			for (int k = 0; k < channel->mNumScalingKeys; ++k)
			{
				aiVector3D a_scale = channel->mScalingKeys[k].mValue;
				vec3 scale = {a_scale.x, a_scale.y, a_scale.z};
				glm_vec3_copy(scale, animation.scale_keys[k]);
			}
			for (int k = 0; k < channel->mNumRotationKeys; ++k)
			{
				aiQuaternion a_rotation = channel->mRotationKeys[k].mValue;
				vec4 rotation = {a_rotation.x, a_rotation.y, a_rotation.z, a_rotation.w};
				glm_vec4_copy(rotation, animation.rotation_keys[k]);
			}
			fwrite(&animation, sizeof(Animation), 1, fp);
		}
	}	
}

BoneID get_max_weight(aiBone **bones, int num_bones, float cap, int vertex_id)
{
	float max_weight = -1;
	int max_weight_id = -1;
	for (int i = 0; i < num_bones; ++i)
	{
		for (int j = 0; j < bones[i]->mNumWeights; ++j)
		{
			if (bones[i]->mWeights[j].mVertexId == vertex_id)
			{
				if ((bones[i]->mWeights[j].mWeight > max_weight) &&
				    (bones[i]->mWeights[j].mWeight < cap))
				{
					max_weight = bones[i]->mWeights[j].mWeight;
					max_weight_id = i;
				}
			}
		}
	}
	return { max_weight_id, max_weight };
}

unsigned int write_meshes(FILE *fp, const aiScene *scene, aiNode *node)
{
	unsigned int total_bytes = 0;
	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		unsigned int local_bytes = 0;
		char mesh_header[512] = {0};
		snprintf(mesh_header, 512, "B_MESH:");
		fwrite(mesh_header, strnlen(mesh_header, 512), 1, fp);
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		/* Should go { pos.x, pos.y, pos.z, norm.x, norm.y, norm.z, tex.x, tex.y, tex.z, bones_ids[0-3]} */
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
			if (mesh->HasBones())
			{
				BoneID bone_ids[4];
				memset(bone_ids, -1, sizeof(BoneID)*4);
				float max_weight = 2.0;
				for (int k = 0; k < 4; ++k)
				{
					bone_ids[k] = get_max_weight(mesh->mBones, mesh->mNumBones, max_weight, j);
					max_weight = bone_ids[k].weight;
				}
				for (int k = 0; k < 4; ++k)
				{
					fwrite(&bone_ids[k].id, sizeof(int), 1, fp);
				}
				for (int k = 0; k < 4; ++k)
				{
					fwrite(&bone_ids[k].weight, sizeof(float), 1, fp);
				}
			}
			else
			{
				BoneID bone_ids[4];
				memset(bone_ids, -1, sizeof(BoneID)*4);
				for (int k = 0; k < 4; ++k)
				{
					fwrite(&bone_ids[k].id, sizeof(int), 1, fp);
				}
				for (int k = 0; k < 4; ++k)
				{
					fwrite(&bone_ids[k].weight, sizeof(float), 1, fp);
				}
			}
			total_bytes += sizeof(float)*9 + sizeof(BoneID)*4;
			local_bytes += sizeof(float)*9 + sizeof(BoneID)*4;
		}
	}

	if (node->mNumMeshes > 0)
	{
		char mesh_end_header[512] = {0};
		snprintf(mesh_end_header, 512, "END_MESHES");
		fwrite(mesh_end_header, strnlen(mesh_end_header, 512), 1, fp);
	}
	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		unsigned int local_bytes = 0;
		char face_header[512] = {0};
		snprintf(face_header, 512, "B_FACES:");
		fwrite(face_header, strnlen(face_header, 512), 1, fp);
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

	for (int i = 0; i < node->mNumMeshes; ++i)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		char bone_header[128] = {0};
		snprintf(bone_header, 128, "B_BONES:");
		fwrite(bone_header, strnlen(bone_header, 128), 1, fp);
		if (mesh->HasBones())
		{
			for (int j = 0; j < mesh->mNumBones; ++j)
			{
				fwrite(&mesh->mBones[j]->mOffsetMatrix, sizeof(aiMatrix4x4), 1, fp);
			}
		}
		memset(bone_header, 0, 128);
		snprintf(bone_header, 128, "END_BONES");
		fwrite(bone_header, strnlen(bone_header, 128), 1, fp);
	}

	if (node->mNumMeshes > 0)
	{
		char face_end_header[512] = {0};
		snprintf(face_end_header, 512, "END_FACES");
		fwrite(face_end_header, strnlen(face_end_header, 512), 1, fp);
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
	fwrite("VERTEX_DATA:", strlen("VERTEX_DATA:"), 1, fp);
	unsigned int total_bytes = write_meshes(fp, scene, scene->mRootNode);
	fwrite("END_VERTEX_DATA", strlen("END_VERTEX_DATA"), 1, fp);
	fwrite("ANIMATIONS:", strlen("ANIMATIONS:"), 1, fp);
	if (scene->HasAnimations())
	{
		write_animations(fp, scene, scene->mRootNode);
	}
	fwrite("END_ANIMATIONS", strlen("END_ANIMATIONS"), 1, fp);

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
		strncpy(filename, "model.bgm", 512);
	}
	else
	{
		memcpy(filename, argv[2], strnlen(argv[2], 512));
	}
	FILE *fp = fopen(filename, "w");
	fseek(fp, 0L, SEEK_SET);


	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(argv[1], aiProcess_FlipUVs | aiProcess_Triangulate |    aiProcess_CalcTangentSpace);
	unsigned int total_bytes = write_vertex_data(fp, scene);
	char bytes_header[512] = {0};
	snprintf(bytes_header, 512, "TOTAL_BYTES: %u\n", total_bytes);
	fwrite(bytes_header, strnlen(bytes_header, 512), 1, fp);
	fclose(fp);
	return 0;
}
