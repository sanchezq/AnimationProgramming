#pragma once

#include "BonePose.h"

#include <string>
#include <vector>

class AnimationKeyFrame
{
public:

	AnimationKeyFrame() = delete;
	AnimationKeyFrame(const int index, std::string name);

	void AddBone(LibMath::Vector3 translation, LibMath::Quaternion rotation);
	const BonePose& GetBoneByIndex(const int index) const;

	/* Init Bones of the given skeleton*/
	void InitSkeleton();

	void InitOneBone(int boneIndex);

private:

	int m_index;
	std::string m_name;

	std::vector<BonePose> m_bones;
};
