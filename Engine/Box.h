#pragma once

#include <Box2D\Box2D.h>
#include "IndexedTriangleList.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Mat2.h"
#include "Colors.h"
#include "Pipeline.h"
#include "SolidEffect.h"
#include "BodyPtr.h"
#include "Boundaries.h"
#include <random>
#include "ColourTrait.h"
#include "Properties.h"

class Box
{
public:
	static std::unique_ptr<Box> Box::Spawn( float size,const Boundaries& bounds,b2World& world,std::mt19937& rng );
	Box( std::unique_ptr<ColorTrait> pColorTrait, b2World& world,const Vec2& pos,
		float size = 1.0f,float angle = 0.0f,Vec2 linVel = {0.0f,0.0f},float angVel = 0.0f )
		:
		p(pos, linVel, std::move(pColorTrait), angle, angVel, size),
		world(world)
	{
		Init();
		{
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;
			bodyDef.position = b2Vec2( pos );
			bodyDef.linearVelocity = b2Vec2( linVel );
			bodyDef.angularVelocity = angVel;
			bodyDef.angle = angle;
			pBody = BodyPtr::Make( world,bodyDef );
		}
		{
			b2PolygonShape dynamicBox;
			dynamicBox.SetAsBox( size,size );
			b2FixtureDef fixtureDef;
			fixtureDef.shape = &dynamicBox;
			fixtureDef.density = 1.0f;
			fixtureDef.friction = 0.0f;
			fixtureDef.restitution = 1.0f;
			pBody->CreateFixture( &fixtureDef );
		}
		pBody->SetUserData( this );
	}
	Box(Properties& p_in, b2World& world)
		:
		p(p_in),
		world(world)
	{
		*this = Box(p.trait->Clone(), world, p.position, p.size,
			p.angle, p.velocity, p.angularVel);
	}
	Box& operator=(Box& other)
	{
		p = other.p;
		Init();
		{
			b2BodyDef bodyDef;
			bodyDef.type = b2_dynamicBody;
			bodyDef.position = b2Vec2(other.p.position);
			bodyDef.linearVelocity = b2Vec2(other.p.velocity);
			bodyDef.angularVelocity = other.p.angularVel;
			bodyDef.angle = other.p.angle;
			pBody = BodyPtr::Make(world, bodyDef);
		}
		{
			b2PolygonShape dynamicBox;
			dynamicBox.SetAsBox(other.GetSize(), other.GetSize());
			b2FixtureDef fixtureDef;
			fixtureDef.shape = &dynamicBox;
			fixtureDef.density = 1.0f;
			fixtureDef.friction = 0.0f;
			fixtureDef.restitution = 1.0f;
			pBody->CreateFixture(&fixtureDef);
		}
		pBody->SetUserData(this);
		return *this;
	}
	~Box() = default;
	void Draw( Pipeline<SolidEffect>& pepe ) const
	{
		pepe.effect.vs.BindTranslation( GetPosition() );
		pepe.effect.vs.BindRotation( Mat2::Rotation( GetAngle() ) * Mat2::Scaling( GetSize() ) );
		pepe.effect.ps.BindColor( GetColorTrait().GetColor() );
		pepe.Draw( model );
	}
	void ApplyLinearImpulse( const Vec2& impulse )
	{
		pBody->ApplyLinearImpulse( (b2Vec2)impulse,(b2Vec2)GetPosition(),true );
	}
	void ApplyAngularImpulse( float impulse )
	{
		pBody->ApplyAngularImpulse( impulse,true );
	}
	void ScheduleDestruction() { destroy = true; }
	void ScheduleSplit() { split = true; }
	bool Split() { bool ret = split; split = false; return ret; }
	bool ToBeDestroyed()
	{
		return destroy;
	}
	float GetAngle() const
	{
		return pBody->GetAngle();
	}
	Vec2 GetPosition() const
	{
		return (Vec2)pBody->GetPosition();
	}
	float GetAngularVelocity() const
	{
		return pBody->GetAngularVelocity();
	}
	Vec2 GetVelocity() const
	{
		return (Vec2)pBody->GetLinearVelocity();
	}
	float GetSize() const
	{
		return p.size;
	}
	Properties GetProperties()
	{
		UpdateProperties();
		return Properties(p);
	}
	void UpdateProperties()
	{
		p.Update(pBody);
	}
	void SetColour(std::unique_ptr<ColorTrait> c) { p.trait = std::move(c); }
	const ColorTrait& GetColorTrait() const
	{
		return *(p.trait);
	}
private:
	static void Init()
	{
		if( model.indices.size() == 0 )
		{
			model.vertices = { { -1.0f,-1.0 },{ 1.0f,-1.0 },{ -1.0f,1.0 },{ 1.0f,1.0 } };
			model.indices = { 0,1,2, 1,2,3 };
		}
	}
private:
	b2World& world;
	static IndexedTriangleList<Vec2> model;
	BodyPtr pBody;
	Properties p;
	bool destroy = false;
	bool split = false;
};