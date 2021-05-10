// AnimationProgramming.cpp : Defines the entry point for the console application.
//

#include <string>
#include <vector>

#include "stdafx.h"

#include "Engine.h"
#include "Skeleton.h"
#include "Simulation.h"
#include "AnimationClip.h"
#include "Interpolation.h"

class CSimulation : public ISimulation
{
	AnimationClip* m_walkAnim = nullptr;
	AnimationClip* m_runAnim = nullptr;
	float m_alphaTime = 0.f;
	
	virtual void Init() override
	{
		const int spine01 = GetSkeletonBoneIndex("spine_01");
		const int spineParent = GetSkeletonBoneParentIndex(spine01);
		const char* spineParentName = GetSkeletonBoneName(spineParent);

		float posX, posY, posZ, quatW, quatX, quatY, quatZ;
		GetAnimLocalBoneTransform("ThirdPersonWalk.anim", spineParent, GetAnimKeyCount("ThirdPersonWalk.anim") / 2, posX, posY, posZ, quatW, quatX, quatY, quatZ);

		InitSkeleton();
		m_walkAnim = new AnimationClip("ThirdPersonWalk.anim", GetAnimKeyCount("ThirdPersonWalk.anim"));
		m_runAnim = new AnimationClip("ThirdPersonRun.anim", GetAnimKeyCount("ThirdPersonRun.anim"));
	}

	void static InitSkeleton()
	{
		for (size_t nbBone = 0; nbBone < GetSkeletonBoneCount(); nbBone++)
		{
			InitOneBone(nbBone);
		}
	}

	void static InitOneBone(const unsigned int boneIndex)
	{
		const std::string skelName = GetSkeletonBoneName(boneIndex);
		if (skelName.find("ik_") == 0)
			return;
		
		float bonePosX, bonePosY, bonePosZ, boneQuatW, boneQuatX, boneQuatY, boneQuatZ;
		GetSkeletonBoneLocalBindTransform(boneIndex, bonePosX, bonePosY, bonePosZ, boneQuatW, boneQuatX, boneQuatY, boneQuatZ);

		printf("bone : pos(%.2f,%.2f,%.2f) rotation quat(%.10f,%.10f,%.10f,%.10f)\n", bonePosX, bonePosY, bonePosZ, boneQuatW, boneQuatX, boneQuatY, boneQuatZ);

		LibMath::Matrix4 localTransform = LibMath::Matrix4::Identity();
		localTransform.Rotate(LibMath::Quaternion(boneQuatX, boneQuatY, boneQuatZ, boneQuatW));
		localTransform.Translate(bonePosX, bonePosY, bonePosZ);

		const int parentBoneIndex = GetSkeletonBoneParentIndex(boneIndex);

		LibMath::Matrix4 globalTransform = localTransform;
		if (boneIndex > 0)
		{
			globalTransform = m_skeleton.GetBoneByIndex(parentBoneIndex)->m_globalTransform * localTransform;
		}

		m_skeleton.AddBone(parentBoneIndex, localTransform, globalTransform);
	}

	virtual void Update(float frameTime) override
	{
		m_alphaTime += frameTime;
		const float blendValue = static_cast<float>(sin(static_cast<double>(m_alphaTime))) * .5f + .5f;

		printf("BLEND: %f\n", blendValue);

		m_walkAnim->UpdateValuePos(frameTime);
		m_runAnim->UpdateValuePos(frameTime);
		
		DrawAxis();
		DrawSkinningBone(*m_walkAnim, *m_runAnim, blendValue);
	}

	void static DrawAxis()
	{
		// X axis
		DrawLine(0, 0, 0, 100, 0, 0, 1, 0, 0);

		// Y axis
		DrawLine(0, 0, 0, 0, 100, 0, 0, 1, 0);

		// Z axis
		DrawLine(0, 0, 0, 0, 0, 100, 0, 0, 1);
	}

	void static DrawBindPose()
	{
		for (size_t nbBone = 1; nbBone < m_skeleton.GetBoneCount(); nbBone++)
		{
			DrawOneBone(nbBone);
		}
	}

	void DrawAnimBone(int animKeyFrame) const
	{
		for (size_t nbBone = 1; nbBone < m_skeleton.GetBoneCount(); nbBone++)
		{
			DrawOneBoneAnim(nbBone, animKeyFrame);
		}
	}

	void DrawSkinningBone(const AnimationClip& clip1, const AnimationClip& clip2, float blendValue) const
	{
		std::vector<LibMath::Matrix4> vecMat;

		size_t clip1Actual, clip1Next, clip2Actual, clip2Next;
		float blendValueClip1, blendValueClip2;

		clip1.SetBlendFrames(clip1Actual, clip1Next, blendValueClip1);
		clip2.SetBlendFrames(clip2Actual, clip2Next, blendValueClip2);

		for (size_t nbBone = 0; nbBone < m_skeleton.GetBoneCount(); nbBone++)
		{
			const Bone* bone = m_skeleton.GetBoneByIndex(nbBone);

			if (!bone->IsIK())
			{
				BonePose clip1PoseMin = m_walkAnim->GetBoneByIndex(clip1Actual, nbBone);
				BonePose clip1PoseMax = m_walkAnim->GetBoneByIndex(clip1Next, nbBone);
				BonePose clip2PoseMin = m_runAnim->GetBoneByIndex(clip2Actual, nbBone);
				BonePose clip2PoseMax = m_runAnim->GetBoneByIndex(clip2Next, nbBone);
				
				BonePose clip1Pose = BlendPoses(clip1PoseMin, clip1PoseMax, blendValueClip1);
				BonePose clip2Pose = BlendPoses(clip2PoseMin, clip2PoseMax, blendValueClip2);
				
				BonePose bonePose = BlendPoses(clip1Pose, clip2Pose, blendValue);

				vecMat.push_back(	LibMath::Matrix4::Translation(bonePose.m_translation) *
										LibMath::Matrix4::Rotation(bonePose.m_rotation) *
										m_skeleton.GetBoneByIndex(nbBone)->m_invBindPose);			
			}
		}

		SetSkinningPose((float*)vecMat.data(), vecMat.size());
	}

	BonePose BlendPoses(const BonePose& bonePoseMin, const BonePose& bonePoseMax, float blendValue) const
	{
		return BonePose(LibMath::Interpolation::Lerp(bonePoseMin.m_translation, bonePoseMax.m_translation, blendValue),
						LibMath::Interpolation::Slerp(bonePoseMin.m_rotation, bonePoseMax.m_rotation, blendValue));
	}
	
	void static DrawOneBone(const int boneIndex)
	{
		const Bone* bone = m_skeleton.GetBoneByIndex(boneIndex);

		if (bone == nullptr || bone->IsIK())
		{
			return;
		}

		const LibMath::const_row start = bone->m_globalTransform[3];
		const LibMath::const_row end = m_skeleton.GetBoneByIndex(bone->m_indexParent)->m_globalTransform[3];

		DrawLine(start[0], start[1] - 50, start[2], end[0], end[1] - 50, end[2], 0, 0, 0);
	}

	void static DrawOneBoneBindPose(int boneIndex)
	{
		const Bone* bone = m_skeleton.GetBoneByIndex(boneIndex);

		if (bone == nullptr || bone->IsIK())
		{
			return;
		}

		const LibMath::const_row start = bone->m_globalTransform[3];
		const LibMath::const_row end = m_skeleton.GetBoneByIndex(bone->m_indexParent)->m_globalTransform[3];

		DrawLine(start[0], start[1] - 50, start[2], end[0], end[1] - 50, end[2], 0, 0, 0);
	}

	void DrawOneBoneAnim(int boneIndex, int animKeyFrame) const
	{
		const Bone* bone = m_skeleton.GetBoneByIndex(boneIndex);

		if (bone == nullptr || bone->IsIK())
		{
			return;
		}

		LibMath::Vector3 start = m_walkAnim->GetBoneByIndex(animKeyFrame, boneIndex).m_translation;
		LibMath::Vector3 end = m_walkAnim->GetBoneByIndex(animKeyFrame, bone->m_indexParent).m_translation;

		DrawLine(start[0], start[1] - 50, start[2], end[0], end[1] - 50, end[2], 0, 0, 0);
	}
};




int main()
{
	CSimulation simulation;
	Run(&simulation, 1400, 800);

	return 0;
}
