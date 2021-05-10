#pragma once

struct Bone
{
	Bone(unsigned int pIndexParent, const char* pName, LibMath::Matrix4 pLocalTransform, LibMath::Matrix4 pGlobalTransform)
		: m_indexParent(pIndexParent), m_name(pName), m_localTransform(pLocalTransform), m_globalTransform(pGlobalTransform)
	{
		m_invBindPose = m_globalTransform.GetInverse();
	}

	[[nodiscard]] bool IsIK() const
	{
		return m_name.find("ik_") == 0;
	}

	unsigned int m_indexParent = 0;
	std::string m_name;
	
	LibMath::Matrix4 m_localTransform;
	LibMath::Matrix4 m_globalTransform;
	LibMath::Matrix4 m_invBindPose;
};
