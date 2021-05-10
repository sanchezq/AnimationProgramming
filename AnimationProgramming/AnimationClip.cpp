#include "AnimationClip.h"

#include <iostream>

#include "Engine.h"
#include "Interpolation.h"

BonePose::BonePose(LibMath::Vector3 translation, LibMath::Quaternion rotation) :
	m_translation(translation), m_rotation(rotation)
{}

BonePose BonePose::operator*(const BonePose& other) const
{
	BonePose result;
	result.m_rotation = other.m_rotation * m_rotation;
	result.m_rotation.Normalize();

	result.m_translation = other.m_translation + (other.m_rotation * m_translation);

	return result;
}


AnimationKeyFrame::AnimationKeyFrame(const int index, std::string name) :
	m_index(index), m_name(std::move(name))
{
	InitSkeleton();
}

void AnimationKeyFrame::AddBone(LibMath::Vector3 translation, LibMath::Quaternion rotation)
{
	m_bones.emplace_back(translation, rotation);
}

const BonePose& AnimationKeyFrame::GetBoneByIndex(const int index) const
{
	return m_bones[index];
}

void AnimationKeyFrame::InitSkeleton()
{
	for (size_t nbBone = 0; nbBone < GetSkeletonBoneCount(); nbBone++)
	{
		InitOneBone(nbBone);
	}
}

void AnimationKeyFrame::InitOneBone(int boneIndex)
{
	const std::string skelName = GetSkeletonBoneName(boneIndex);
	if (skelName.find("ik_") == 0)
		return;
	
	BonePose keyFramePose;
	GetAnimLocalBoneTransform(m_name.c_str(), boneIndex, m_index, keyFramePose.m_translation.x, keyFramePose.m_translation.y, keyFramePose.m_translation.z, keyFramePose.m_rotation.W, keyFramePose.m_rotation.X, keyFramePose.m_rotation.Y, keyFramePose.m_rotation.Z);

	BonePose bindBonePose;
	GetSkeletonBoneLocalBindTransform(boneIndex, bindBonePose.m_translation.x, bindBonePose.m_translation.y, bindBonePose.m_translation.z, bindBonePose.m_rotation.W, bindBonePose.m_rotation.X, bindBonePose.m_rotation.Y, bindBonePose.m_rotation.Z);

	keyFramePose = keyFramePose * bindBonePose;
	const int parentBoneIndex = GetSkeletonBoneParentIndex(boneIndex);

	
	if (boneIndex > 0)
	{
		const BonePose& parentKeyFramePose = m_bones[parentBoneIndex];
		keyFramePose = keyFramePose * parentKeyFramePose;
	}

	m_bones.emplace_back(keyFramePose);
}


AnimationClip::AnimationClip(std::string name, int keyCount)  :
	m_name(std::move(name)), m_keyCount(keyCount)
{
	for (int keyIndex = 0; keyIndex < keyCount; keyIndex++)
	{
		InitKeyFrame(keyIndex);
	}
}

std::string AnimationClip::GetName() const
{
	return m_name;
}

void AnimationClip::InitKeyFrame(int keyFrame)
{
	m_animationKeyFrames.emplace_back(keyFrame, m_name);
}

const BonePose& AnimationClip::GetBoneByIndex(const int keyFrame, const int index) const
{
	return m_animationKeyFrames[keyFrame].GetBoneByIndex(index);
}

void AnimationClip::UpdateValuePos(float deltaTime)
{
	m_animDelta += deltaTime;
	if (m_animDelta >= 1.f)
		m_animDelta -= 1.f;

	m_frame = LibMath::Interpolation::MapInRange(m_animDelta, 0.f, 1.f, 0.f, static_cast<float>(m_keyCount));
}

void AnimationClip::SetBlendFrames(size_t& bonePosMin, size_t& bonePosMax, float& BlendValue) const
{
	bonePosMin = m_frame;

	bonePosMax = bonePosMin + 1;
	if (bonePosMax >= m_keyCount)
		bonePosMax -= m_keyCount;

	BlendValue = m_frame - static_cast<float>(bonePosMin);
}
