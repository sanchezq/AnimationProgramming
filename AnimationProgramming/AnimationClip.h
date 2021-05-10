#pragma once

#include "AnimationKeyFrame.h"

class AnimationClip
{
public:

	AnimationClip() = delete;
	AnimationClip(std::string name, int keyCount);
	
	[[nodiscard]] std::string GetName() const;

	void InitKeyFrame(int keyFrame);
	void UpdateValuePos(float deltaTime);
	void SetBlendFrames(size_t& bonePosMin, size_t& bonePosMax, float& BlendValue) const;

	[[nodiscard]] const BonePose& GetBoneByIndex(const int keyFrame, const int index) const;

private:

	std::string m_name;
	size_t m_keyCount;
	float m_animDelta;
	float m_frame;
	
	std::vector<AnimationKeyFrame> m_animationKeyFrames;
};
