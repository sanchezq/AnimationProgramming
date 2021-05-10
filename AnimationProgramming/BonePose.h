#pragma once

#include "Matrix/Matrix4.h"

struct BonePose
{
public:

	BonePose() = default;
	BonePose(LibMath::Vector3 translation, LibMath::Quaternion rotation);

	BonePose operator*(const BonePose& other) const;

	LibMath::Vector3 m_translation;
	LibMath::Quaternion m_rotation;
};
