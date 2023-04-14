#include "stdafx.h"
#include "t_joint_base.h"

namespace TJoint
{
	std::map<JointType, Template *> Template::Templates;

	Template *GetTemplate(JointType type)
	{
		auto iter = Template::Templates.find(type);
		if (iter == Template::Templates.end()) return nullptr;
		return iter->second;
	}
}