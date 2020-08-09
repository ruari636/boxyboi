#pragma once
#include <Box2D\Box2D.h>
#include "Box.h"
#include "ColourTrait.h"
#include <sstream>
#include <unordered_map>
#include <typeinfo>
#include <functional>
#include <typeindex>

struct trait_pair
{
	trait_pair(std::type_index c1, std::type_index c2)
		:
		c1(c1),
		c2(c2)
	{
	}
	void Allign(Box* b1, Box* b2) const
	{
		std::type_index C1 = typeid(b1->GetColorTrait());
		std::type_index C2 = typeid(b2->GetColorTrait());
		if (c1 != C1)
		{
			Box* temp = std::move(b1);
			*b1 = std::move(*b2);
			*b2 = std::move(*temp);
		}
	}
	bool operator==(const trait_pair& rhs) const
	{
		return c1 == rhs.c1 && c2 == rhs.c2;
	}
	std::type_index c1;
	std::type_index c2;
};

namespace std
{
	template<>
	struct hash<trait_pair>
	{
		size_t operator()(const trait_pair& key)const
		{
			size_t c1 = key.c1.hash_code();
			size_t c2 = key.c2.hash_code();
			return c1 ^ (c2 + 0x9e3779b9 + (c1 << 6) + (c1 >> 2));
		}
	};
}

class BoxContactResolver : public b2ContactListener
{
	std::unordered_map<trait_pair, std::function<void(Box*, Box*)>> pairToFunc;
	const int maxSplits = 2;
	const int splitsPerEdge = 2;
	const float boxSize = 1.0f;

public:
	BoxContactResolver(int maxSplits = 2, int splitsPerEdge = 2, float boxSize = 1.0f)
		:
		maxSplits(maxSplits),
		splitsPerEdge(splitsPerEdge),
		boxSize(boxSize)
	{
	}

	template<class C1, class C2, class A>
	void AddAction(A a)
	{
		pairToFunc[trait_pair{ typeid(C1), typeid(C2) }] = a;
		pairToFunc[trait_pair{ typeid(C2), typeid(C1) }] = std::bind(a,
			std::placeholders::_2, std::placeholders::_1); //stolen from chilli
	}

	void HandleBoxContact(Box* b1, Box* b2)
	{
		std::type_index c1 = typeid(b1->GetColorTrait());
		std::type_index c2 = typeid(b2->GetColorTrait());
		int n = 0;
		auto match = pairToFunc.find({ c1, c2 });
		if (match != pairToFunc.end())
		{
			match->second(b1, b2);
		}
	}

	void BeginContact(b2Contact* contact) override
	{
		const b2Body* bodyPtrs[] = { contact->GetFixtureA()->GetBody(),contact->GetFixtureB()->GetBody() };
		if (bodyPtrs[0]->GetType() == b2BodyType::b2_dynamicBody &&
			bodyPtrs[1]->GetType() == b2BodyType::b2_dynamicBody)
		{
			Box* boxPtrs[] = {
				reinterpret_cast<Box*>(bodyPtrs[0]->GetUserData()),
				reinterpret_cast<Box*>(bodyPtrs[1]->GetUserData())
			};

			std::string name1 = typeid(boxPtrs[0]->GetColorTrait()).name();
			std::string name2 = typeid(boxPtrs[1]->GetColorTrait()).name();

			HandleBoxContact(boxPtrs[0], boxPtrs[1]);

			std::stringstream msg;
			msg << "Collision between " << name1 << " and " << name2 << std::endl;
			OutputDebugStringA(msg.str().c_str());
		}
	}
};