#include "stdafx.h"
#include "t_joint_base.h"

namespace TJoint
{
	std::map<mu_utf8string, JointType> Template::TemplateTypes;
	std::map<JointType, Template *> Template::Templates;

	void Initialize()
	{
		for (auto iter = Template::Templates.begin(); iter != Template::Templates.end(); ++iter)
		{
			iter->second->Initialize();
		}
	}

	std::optional<JointType> GetTemplateType(const mu_utf8string id)
	{
		auto iter=Template::TemplateTypes.find(id);
		if (iter == Template::TemplateTypes.end()) return std::nullopt;
		return iter->second;
	}

	Template *GetTemplate(JointType type)
	{
		auto iter = Template::Templates.find(type);
		if (iter == Template::Templates.end()) return nullptr;
		return iter->second;
	}
}