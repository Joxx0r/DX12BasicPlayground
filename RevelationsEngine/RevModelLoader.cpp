#include "stdafx.h"
#include "RevModelLoader.h"
#include "RevModel.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include "RevUtils.h"
#include "d3dcompiler.h"
#include <memory>
#include "RevAnimation.h"
#include <fstream>

#include "RevArchive.h"


//path file exist
#include "Shlwapi.h"

std::string GetMaterialPath(aiMaterial* material, aiTextureType type, const char* basePath)
{
#if USE_ASSIMP
	aiString str;
	material->GetTexture(type, 0, &str);
	std::string element = str.C_Str();
	std::string data = element.substr(2, element.length() - 1);
	std::string pathFile = basePath;
	size_t elem = pathFile.find_last_of('\\');
	std::string otherString = pathFile.substr(0, elem + 1);
	otherString.append(data);
	return otherString;
#else
	std::string returnString;
	return returnString;
#endif
}

void CreateBaseVertex(
		const aiMesh* mesh, 
		UINT vertIndex, 
		RevVertex* outVertex)
{
	aiVector3D vert = mesh->mVertices[vertIndex];
	outVertex->m_location = RevVector3(vert.x, vert.y, vert.z);

	for (UINT texCordIndex = 0; texCordIndex < mesh->mNumUVComponents[0]; texCordIndex++)
	{
		outVertex->m_tex[0] = (float)mesh->mTextureCoords[0][vertIndex].x;
		outVertex->m_tex[1] = 1 - (float)mesh->mTextureCoords[0][vertIndex].y;
	}

	aiVector3D normal = mesh->mNormals[vertIndex];
	outVertex->m_normal = RevVector3(normal.x, normal.y, normal.z);

	aiVector3D biNormal = mesh->mBitangents[vertIndex];
	outVertex->m_biNormal = RevVector3(biNormal.x, biNormal.y, biNormal.z);


	aiVector3D tangent = mesh->mTangents[vertIndex];
	outVertex->m_tangent = RevVector3(tangent.x, tangent.y, tangent.z);
}


void LoadIndecies(const aiMesh* mesh, std::vector<UINT> &indices)
{
	for (UINT vertIndex = 0; vertIndex < mesh->mNumFaces; vertIndex++)
	{
		aiFace* face = &mesh->mFaces[vertIndex];
		for (UINT faceIndex = 0; faceIndex < face->mNumIndices; faceIndex++)
		{
			indices.push_back(
				face->mIndices[faceIndex]);
		}
	}
}

aiNode* FindNodeRecursive(aiNode* node, const char* name)
{
	if (strcmp(node->mName.C_Str(), name) == 0)
	{
		return node;
	}

	for (UINT index = 0; index < node->mNumChildren; index++)
	{
		aiNode* childNode = node->mChildren[index];
		if (aiNode* returnNode = FindNodeRecursive(childNode, name))
		{
			return returnNode;
		}
	}

	return nullptr;
}


void LoadTexturePaths(
		const aiMesh* mesh, 
		const struct aiScene* scene, 
		RevTextureInitializationData& outData,
		const char* path)
{
#if USE_ASSIMP
	outData.m_nTexturePaths = 0;
	UINT numMaterial = mesh->mMaterialIndex;
	if (mesh->mMaterialIndex >= 0)
	{

		outData.m_nTexturePaths = 4;
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		strcpy(outData.m_diffuseTexturePath, GetMaterialPath(material, aiTextureType_DIFFUSE, path).c_str());
		strcpy(outData.m_normalTexturePath, GetMaterialPath(material, aiTextureType_NORMALS, path).c_str());
		strcpy(outData.m_substanceTexturePath, GetMaterialPath(material, aiTextureType_SPECULAR, path).c_str());
		strcpy(outData.m_roughnessAOEmissivePath, GetMaterialPath(material, aiTextureType_LIGHTMAP, path).c_str());
	}
#endif
}

void LoadIndecies(const aiMesh* mesh, UINT** indices, UINT& outNumIndicies)
{
	outNumIndicies = 0;
	for (UINT vertIndex = 0; vertIndex < mesh->mNumFaces; vertIndex++)
	{
		aiFace* face = &mesh->mFaces[vertIndex];
		for (UINT faceIndex = 0; faceIndex < face->mNumIndices; faceIndex++)
		{
			outNumIndicies++;
		}
	}

	*indices = new UINT[outNumIndicies];

	UINT indiciesIndex = 0;
	for (UINT vertIndex = 0; vertIndex < mesh->mNumFaces; vertIndex++)
	{
		aiFace* face = &mesh->mFaces[vertIndex];
		for (UINT faceIndex = 0; faceIndex < face->mNumIndices; faceIndex++)
		{
			(*indices)[indiciesIndex] = face->mIndices[faceIndex];
			indiciesIndex++;
		}
	}
}

void LoadAnimatedModel(const struct aiScene* scene, RevModel* newModel, const char* path)
{
	RevAnimatedNodelInitializationData initializationData;

	for (UINT meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		const aiMesh* mesh = scene->mMeshes[meshIndex];
		if (mesh)
		{
			initializationData.m_nVertexes = mesh->mNumVertices;
			initializationData.vertices = new RevAnimatedVertex[initializationData.m_nVertexes];
			for (UINT vertIndex = 0; vertIndex < mesh->mNumVertices; vertIndex++)
			{
				RevAnimatedVertex vertexToUse = {};
				CreateBaseVertex(mesh, vertIndex, &vertexToUse);
				initializationData.vertices[vertIndex] = vertexToUse;
			}

			LoadIndecies(mesh, &initializationData.indices, initializationData.m_nIndices);
			LoadTexturePaths(mesh, scene, initializationData.m_textureInitialization, path);

			initializationData.m_nBones = mesh->mNumBones;
			initializationData.bones = new RevBone[initializationData.m_nBones];
			for (UINT boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
			{
				aiBone* bone = mesh->mBones[boneIndex];
				RevBone boneData = {};

				strcpy(boneData.m_name , bone->mName.C_Str());
				aiMatrix4x4 boneM = bone->mOffsetMatrix.Transpose();
				boneData.m_offsetMatrix.Load(&boneM.a1);
				for (UINT influenceIndex = 0; influenceIndex < bone->mNumWeights; influenceIndex++)
				{
					aiVertexWeight weight = bone->mWeights[influenceIndex];
					RevAnimatedVertex* animatedVertex = (RevAnimatedVertex*)&initializationData.vertices[weight.mVertexId];
					UINT indexToUse = UINT8_MAX;
					for (UINT boneInfluenceIndex = 0; boneInfluenceIndex < REV_MAX_INFLUENCES_PER_BONE; boneInfluenceIndex++)
					{
						if (animatedVertex->m_boneInfluences[boneInfluenceIndex] == UINT_MAX)
						{
							indexToUse = boneInfluenceIndex;
							break;
						}
					}

					float weightScale = weight.mWeight;

					if (indexToUse == UINT8_MAX)
					{
						float minimumWeightFound = weightScale;
						for (UINT boneInfluenceIndex = 0; boneInfluenceIndex < REV_MAX_INFLUENCES_PER_BONE; boneInfluenceIndex++)
						{
							const float& weightToCheck = animatedVertex->m_boneWeights[boneInfluenceIndex];
							if (weightToCheck < minimumWeightFound)
							{
								minimumWeightFound = weightToCheck;
								indexToUse = boneInfluenceIndex;
							}
						}
					}

					if (indexToUse != UINT8_MAX)
					{
						animatedVertex->m_boneWeights[indexToUse] = weightScale;
						animatedVertex->m_boneInfluences[indexToUse] = boneIndex;
					}
				}

				initializationData.bones[boneIndex] = boneData;
			}

			for (UINT boneIndex = 0; boneIndex < initializationData.m_nBones; boneIndex++)
			{
				RevBone& bone = initializationData.bones[boneIndex];

				aiNode* node = FindNodeRecursive(scene->mRootNode, bone.m_name);

				UINT parentIndex = UINT8_MAX;
				if (node && node->mParent)
				{
					for (UINT parentSearchIndex = 0; parentSearchIndex < initializationData.m_nBones; parentSearchIndex++)
					{
						RevBone& parentBone = initializationData.bones[parentSearchIndex];
						if (strcmp(parentBone.m_name, node->mParent->mName.C_Str()) == 0)
						{
							parentIndex = parentSearchIndex;
							break;
						}
					}
				}
				bone.m_parentIndex = parentIndex;
			}
		}
	}

	for (UINT animationIndex = 0; animationIndex < scene->mNumAnimations; animationIndex++)
	{
		const aiAnimation* animation = scene->mAnimations[animationIndex];
		RevAnimationInstanceData animationModelInstanceData = {};
		animationModelInstanceData.m_name = animation->mName.C_Str();
		
		animationModelInstanceData.m_duration = (float)animation->mDuration;
		DEBUG_ASSERT(animation->mNumChannels != 0);
		for (UINT channelIndex = 0; channelIndex < animation->mNumChannels; channelIndex++)
		{
			aiNodeAnim* channel = animation->mChannels[channelIndex];
			DEBUG_ASSERT(channel);

			RevAnimationChannel animationChannel = {};
			animationChannel.m_nodeName = channel->mNodeName.C_Str();

			assert(channel->mNumPositionKeys == channel->mNumRotationKeys
				&& channel->mNumPositionKeys == channel->mNumScalingKeys);
			animationChannel.m_keyFrames = std::vector<RevKeyFrame>(channel->mNumPositionKeys);

			animationChannel.m_boneIndex = UINT8_MAX;
			for (UINT boneIndex = 0; boneIndex < initializationData.m_nBones; boneIndex++)
			{
				const RevBone& bone = initializationData.bones[boneIndex];
				if (strcmp(bone.m_name, animationChannel.m_nodeName.c_str()) == 0)
				{
					animationChannel.m_boneIndex = boneIndex;
					break;
				}
			}

			for (UINT keyFrameIndex = 0; keyFrameIndex < channel->mNumPositionKeys; keyFrameIndex++)
			{
				RevKeyFrame newKeyFrame = {};
				aiVectorKey locationKey = channel->mPositionKeys[keyFrameIndex];
				aiQuatKey rotationKey = channel->mRotationKeys[keyFrameIndex];
				aiVectorKey scaleKey = channel->mScalingKeys[keyFrameIndex];

				assert(locationKey.mTime == rotationKey.mTime
					&& locationKey.mTime == scaleKey.mTime);

				newKeyFrame.m_time = (float)locationKey.mTime;
				newKeyFrame.m_rotation = RevQuat(rotationKey.mValue.x, rotationKey.mValue.y, rotationKey.mValue.z, rotationKey.mValue.w);
				newKeyFrame.m_location = RevVector3(locationKey.mValue.x, locationKey.mValue.y, locationKey.mValue.z);
				newKeyFrame.m_scale = RevVector3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z);
				
				animationChannel.m_keyFrames[keyFrameIndex] = newKeyFrame;
			}

			animationModelInstanceData.m_channels.push_back(animationChannel);
		}

		initializationData.instanceData.push_back(animationModelInstanceData);
	}
	
	for (UINT boneIndex = 0; boneIndex < initializationData.m_nBones; boneIndex++)
	{
		RevBone& bone = initializationData.bones[boneIndex];

		bone.m_channelIndex = UINT_MAX;
		for (UINT instanceIndex = 0; instanceIndex< initializationData.instanceData.size(); instanceIndex++)
		{
			RevAnimationInstanceData& instance = initializationData.instanceData[instanceIndex];
			for (UINT channelIndex = 0; channelIndex < instance.m_channels.size(); channelIndex++)
			{
				RevAnimationChannel& channel = instance.m_channels[channelIndex];
				if (channel.m_boneIndex == boneIndex)
				{
					bone.m_channelIndex = channelIndex;
					break;
				}
			}
		}
	}
	//readjust vertex weights to be normalized
	for (UINT32 index = 0; index < initializationData.m_nVertexes; index++)
	{
		RevAnimatedVertex& v = initializationData.vertices[index];
		float sum = 0.0f;
		for (UINT i = 0; i < ARRAYSIZE(v.m_boneInfluences); i++)
		{
			if (v.m_boneInfluences[i] == UINT8_MAX)
			{
				v.m_boneInfluences[i] = 0;
				v.m_boneWeights[i] = 0.0f;
			}

			sum += v.m_boneWeights[i];
		}

		if (sum != 0.0f && sum != 1.0f)
		{
			for (UINT i = 0; i < ARRAYSIZE(v.m_boneInfluences); i++)
			{
				v.m_boneWeights[i] /= sum;
			}
		}
	}
	
	RevEngineFunctions::CreateAnimatedModelGeometry(
		initializationData, 
		newModel->m_modelData);

	{
		RevArchiveSaver saver;
		std::string modelPath(path);
		modelPath = modelPath.substr(0, modelPath.find_last_of('.'));
		modelPath.append("_MODEL.rrev");
		std::fstream fstream;
		fstream.open(modelPath.c_str(), std::ios_base::out | std::ios_base::binary);
		{
			saver << (UINT8)RevModelType::Animated;
			initializationData.Serialize(saver);
			fstream << saver.Tell();
			fstream.write((const char*)saver.m_byteArray, saver.Tell());
			fstream.close();
		}
	}


}
void LoadNormalModel(const struct aiScene* scene, RevModel* newModel, const char* path)
{
#if USE_ASSIMP
	bool foundFile = false;

	RevNormalModelInitializationData initializationData = {};
	for (UINT meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		const aiMesh* mesh = scene->mMeshes[meshIndex];
		if (mesh)
		{
			initializationData.m_nVertexes = mesh->mNumVertices;
			initializationData.vertices = new RevVertex[initializationData.m_nVertexes];
			for (UINT vertIndex = 0; vertIndex < mesh->mNumVertices; vertIndex++)
			{
				RevVertex vertexToUse = {};
				CreateBaseVertex(mesh, vertIndex, &vertexToUse);
				initializationData.vertices[vertIndex] = vertexToUse;
			}

			LoadIndecies(mesh, &initializationData.indices, initializationData.m_nIndices);
			LoadTexturePaths(mesh, scene, initializationData.m_textureInitialization, path);
		}
	}

	RevEngineFunctions::CreateNormalModelGeometry(
		initializationData,
		newModel->m_modelData);

	{
		RevArchiveSaver saver;

		std::string modelPath(path);
		modelPath = modelPath.substr(0, modelPath.find_last_of('.'));
		modelPath.append("_MODEL.rrev");
		std::fstream fstream;
		fstream.open(modelPath.c_str(), std::ios_base::out | std::ios_base::binary);
		{
			saver << (UINT8)RevModelType::Normal;
			initializationData.Serialize(saver);
			fstream << saver.Tell();
			fstream.write((const char*)saver.m_byteArray, saver.Tell());
			fstream.close();
		}
	}
#endif
}

RevModel* RevModelLoader::LoadModel(const char* path)
{
	ID3D12GraphicsCommandList* commandList = RevEngineFunctions::FindCommandList();
	RevThrowIfFailed(commandList->Reset(RevEngineFunctions::FindCommandAllocator(), nullptr));

	RevModel* newModel = new RevModel();
	newModel->m_modelData = new RevModelData();

	RevModelType type = RevModelType::Max;

	bool loadedBinary = false;
	std::string modelPath(path);
	modelPath = modelPath.substr(0, modelPath.find_last_of('.'));
	modelPath.append("_MODEL.rrev");

#if DO_LOAING_BINARY
	if (PathFileExists(modelPath.c_str()))
	{
		std::fstream fstream;
		fstream.open(modelPath.c_str(), std::ios_base::in | std::ios_base::binary);
		if (fstream.is_open())
		{
			UINT size = 0;
			fstream >> size;

			RevArchiveLoader loader(size);
			fstream.read((char*)loader.m_byteArray, size);
			loader << type;
			if (type == RevModelType::Animated)
			{
				RevAnimatedNodelInitializationData initData = {};
				initData.Serialize(loader);
				RevEngineFunctions::CreateAnimatedModelGeometry(
					initData,
					newModel->m_modelData);
			}
			else
			{
				RevNormalModelInitializationData initData = {};
				initData.Serialize(loader);
				RevEngineFunctions::CreateNormalModelGeometry(
					initData,
					newModel->m_modelData);
			}

			fstream.close();
			loader.Delete();
		}
	}
	else
#endif
	{
#if USE_ASSIMP
		const struct aiScene* scene = aiImportFile(path, 0);
		assert(scene);

		type = scene->HasAnimations() ? RevModelType::Animated : RevModelType::Normal;
		if (type == RevModelType::Normal)
		{
			LoadNormalModel(scene, newModel, path);
		}
		else
		{
			LoadAnimatedModel(scene, newModel, path);
		}
#else
		assert(0 && "Not using assimp and dont have proepr context");
#endif
	}

	
	newModel->Initialize(type);

	RevThrowIfFailed(commandList->Close());
	ID3D12CommandList* cmdsLists[] = { commandList  };
	RevEngineFunctions::FindCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	RevEngineFunctions::FlushCommandQueue();


	return newModel;
}
