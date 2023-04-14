#ifndef __T_JOINT_THUNDER1_V7_H__
#define __T_JOINT_THUNDER1_V7_H__

#pragma once

#include "t_joint_base.h"

class TJointThunder1V7 : public TJoint::Template
{
public:
	TJointThunder1V7();
public:
	virtual void Create(TJoint::EnttRegistry &registry, const NJointData &data) override;
	virtual TJoint::EnttIterator Move(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last) override;
	virtual TJoint::EnttIterator Action(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last) override;
	virtual TJoint::EnttIterator Render(TJoint::EnttRegistry &registry, TJoint::EnttView &view, TJoint::EnttIterator iter, TJoint::EnttIterator last, TJoint::NRenderBuffer &renderBuffer) override;
	virtual void RenderGroup(const TJoint::NRenderGroup &renderGroup, const TJoint::NRenderBuffer &renderBuffer) override;
};

#endif