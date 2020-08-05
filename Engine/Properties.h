#pragma once
#include "Vec2.h"
#include "Vec3.h"
#include "Mat2.h"
#include "Colors.h"
#include "Pipeline.h"
#include "SolidEffect.h"
#include "BodyPtr.h"
#include "Boundaries.h"
#include "ColourTrait.h"

struct Properties
{
	Properties() = delete;
	Properties(Properties& other)
	{
		*this = other;
	}
	Properties(Vec2 pos, Vec2 vel, std::unique_ptr<ColorTrait> c, float angle, float angVel, float size)
		:
		position(pos),
		velocity(vel),
		trait(std::move(c)),
		angle(angle),
		angularVel(angVel),
		size(size)
	{
	}
	void Update(const BodyPtr& pbody)
	{
		position = (Vec2)pbody->GetPosition();
		velocity = (Vec2)pbody->GetLinearVelocity();
		angularVel = pbody->GetAngularVelocity();
		angle = pbody->GetAngle();
	}
	Properties& operator=(Properties& other)
	{
		trait = other.trait->Clone();
		position = other.position;
		velocity = other.velocity;
		angle = other.angle;
		angularVel = other.angularVel;
		size = other.size;
		return *this;
	}
	Vec2 position;
	Vec2 velocity;
	std::unique_ptr<ColorTrait> trait;
	float angle;
	float angularVel;
	float size;
};