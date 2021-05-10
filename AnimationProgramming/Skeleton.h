#pragma once

#include "Bone.h"

class Skeleton
{
public:
	[[nodiscard]] const Bone* GetBoneByIndex(const unsigned int index) const
	{
		if (index < m_bones.size())
		{
			return &m_bones[index];
		}

		return nullptr;
	}

	void AddBone(unsigned int indexParent, LibMath::Matrix4 localTransform, LibMath::Matrix4 globalTransform)
	{
		m_bones.emplace_back(indexParent, GetSkeletonBoneName(m_bones.size()), localTransform, globalTransform);
	}

	[[nodiscard]] size_t GetBoneCount() const
	{
		return m_bones.size();
	}
private:
	std::vector<Bone> m_bones;
};

inline Skeleton m_skeleton;
