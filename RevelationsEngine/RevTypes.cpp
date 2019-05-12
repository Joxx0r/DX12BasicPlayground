#include "stdafx.h"
#include "RevTypes.h"
#include "RevArchive.h"

#pragma optimize("", off )

D3D12_VERTEX_BUFFER_VIEW RevModelData::VertexBufferView()const
{
	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = m_vertexBufferGpu->GetGPUVirtualAddress();
	vbv.StrideInBytes = m_vertexStride;
	vbv.SizeInBytes = m_vertexBufferByteSize;

	return vbv;
}

D3D12_INDEX_BUFFER_VIEW RevModelData::IndexBufferView()const
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
	ibv.Format = m_indexFormat;
	ibv.SizeInBytes = m_indexBufferByteSize;
	return ibv;
}

void RevNormalModelInitializationData::Serialize(RevArchive& archive)
{
	archive << m_nVertexes;
	archive << m_nIndices;
	archive << m_nVertexes;
	m_textureInitialization.Serialize(archive);
	archive.SerializeBinaryData((void**)&vertices, sizeof(RevVertex) * m_nVertexes);
	archive.SerializeBinaryData((void**)&indices, sizeof(UINT) * m_nIndices);
}

void RevTextureInitializationData::Serialize(RevArchive& archive)
{
	archive << m_nTexturePaths;
	archive.Serialize(m_diffuseTexturePath, sizeof(m_diffuseTexturePath));
	archive.Serialize(m_normalTexturePath, sizeof(m_normalTexturePath));
	archive.Serialize(m_substanceTexturePath, sizeof(m_substanceTexturePath));
	archive.Serialize(m_roughnessAOEmissivePath, sizeof(m_roughnessAOEmissivePath));
}

void RevAnimatedNodelInitializationData::Serialize(class RevArchive& archive)
{
	archive << m_nVertexes;
	archive << m_nIndices;
	archive << m_nVertexes;
	archive << m_nBones;

	UINT nInstances = (UINT)instanceData.size();
	archive << nInstances;
	m_textureInitialization.Serialize(archive);
	archive.SerializeBinaryData((void**)&vertices, sizeof(RevAnimatedVertex) * m_nVertexes);
	archive.SerializeBinaryData((void**)&indices, sizeof(UINT) * m_nIndices);
	archive.SerializeBinaryData((void**)&bones, sizeof(RevBone) * m_nBones);
	archive << instanceData;
}

void RevAnimationInstanceData::Serialize(class RevArchive& archive)
{
	UINT nChannels = (UINT)m_channels.size();
	archive << nChannels;
	archive << m_name;
	archive << m_duration;
	archive << m_channels;
}

void RevAnimationChannel::Serialize(class RevArchive& archive)
{
	archive << m_nodeName;
	archive << m_boneIndex;
	archive << m_keyFrames;

}

void RevKeyFrame::Serialize(class RevArchive& archive)
{
	archive << m_rotation;
	archive << m_location;
	archive << m_scale;
	archive << m_time;
}



#pragma optimize("", on )