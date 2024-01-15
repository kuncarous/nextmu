#ifndef __T_JOINT_BASE_H__
#define __T_JOINT_BASE_H__

#pragma once

#include "t_joint_config.h"
#include "t_joint_enum.h"
#include "t_joint_entity.h"
#include "t_joint_render.h"
#include "t_joint_create.h"

namespace TJoint
{
	template<typename... Other>
	using EnttViewType = entt::view<entt::get_t<Other...>>;
	typedef entt::registry EnttRegistry;
	typedef EnttViewType<Entity::Info> EnttView;
	typedef EnttView::iterator EnttIterator;

	class Template;
	void Initialize();
	std::optional<JointType> GetTemplateType(const mu_utf8string id);
	Template *GetTemplate(JointType type);

	class Template
	{
	public:
		virtual void Initialize() = 0;
		virtual void Create(EnttRegistry &registry, const NJointData &data) = 0;
		virtual EnttIterator Move(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last) = 0;
		virtual EnttIterator Action(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last) = 0;
		virtual EnttIterator Render(EnttRegistry &registry, EnttView &view, EnttIterator iter, EnttIterator last, NRenderBuffer &renderBuffer) = 0;
		virtual void RenderGroup(const NRenderGroup &renderGroup, NRenderBuffer &renderBuffer) = 0;

	protected:
		friend void Initialize();
		friend std::optional<JointType> GetTemplateType(const mu_utf8string id);
		friend Template *GetTemplate(JointType type);
		static std::map<mu_utf8string, JointType> TemplateTypes;
		static std::map<JointType, Template *> Templates;
	};
}

#endif