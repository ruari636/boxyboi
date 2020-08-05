/******************************************************************************************
*	Chili DirectX Framework Version 16.10.01											  *
*	Game.cpp																			  *
*	Copyright 2016 PlanetChili.net <http://www.planetchili.net>							  *
*																						  *
*	This file is part of The Chili DirectX Framework.									  *
*																						  *
*	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
******************************************************************************************/
#include "MainWindow.h"
#include "Game.h"
#include "Box.h"
#include <algorithm>
#include <sstream>
#include <typeinfo>
#include <functional>
#include <iterator>

Game::Game( MainWindow& wnd )
	:
	wnd( wnd ),
	gfx( wnd ),
	world( { 0.0f,-0.5f } ),
	pepe( gfx )
{
	pepe.effect.vs.cam.SetPos( { 0.0,0.0f } );
	pepe.effect.vs.cam.SetZoom( 1.0f / boundarySize );

	std::generate_n( std::back_inserter( boxPtrs ),nBoxes,[this]() {
		return Box::Spawn( boxSize,bounds,world,rng );
	} );

	class Listener : public b2ContactListener
	{
	public:
		void BeginContact( b2Contact* contact ) override
		{
			const b2Body* bodyPtrs[] = { contact->GetFixtureA()->GetBody(),contact->GetFixtureB()->GetBody() };
			if( bodyPtrs[0]->GetType() == b2BodyType::b2_dynamicBody &&
				bodyPtrs[1]->GetType() == b2BodyType::b2_dynamicBody )
			{
				Box* boxPtrs[] = { 
					reinterpret_cast<Box*>(bodyPtrs[0]->GetUserData()),
					reinterpret_cast<Box*>(bodyPtrs[1]->GetUserData())
				};
				auto& tid0 = typeid(boxPtrs[0]->GetColorTrait());
				auto& tid1 = typeid(boxPtrs[1]->GetColorTrait());

				if (tid0 != tid1)
				{
					boxPtrs[0]->ScheduleDestruction();
					boxPtrs[1]->ScheduleDestruction();
				}
				else
				{
					if (boxPtrs[0]->GetSize() > boxSize / (float)(splitsPerEdge * maxSplits))
					{
						boxPtrs[0]->ScheduleSplit();
					}
					else if (boxPtrs[1]->GetSize() > boxSize / (float)(splitsPerEdge * maxSplits))
					{
						boxPtrs[1]->ScheduleSplit();
					}
				}

				std::stringstream msg;
				msg << "Collision between " << tid0.name() << " and " << tid1.name() << std::endl;
				OutputDebugStringA( msg.str().c_str() );
			}
		}
	};
	static Listener mrLister;
	world.SetContactListener( &mrLister );
}

void Game::Go()
{
	gfx.BeginFrame();
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::UpdateModel()
{
   	if (wnd.kbd.KeyIsPressed(VK_SPACE) && time > timer)
	{
		time = 0.0f;
		for (int i = 0, size = boxPtrs.size(); i < size; i++)
		{
			boxPtrs[i] = SplitBox(std::move(boxPtrs[i]), splitsPerEdge);
		}
	}
	for (int i = 0, size = boxPtrs.size(); i < size; i++)
	{
		if (boxPtrs[i]->Split())
		{
			boxPtrs[i] = SplitBox(std::move(boxPtrs[i]), splitsPerEdge);
		}
	}

	const float dt = ft.Mark();
	time += dt;
	world.Step( dt,8,3 );

	for (size_t i = 0; i < boxPtrs.size(); i++)
	{
		size_t left = boxPtrs.size();
		if (boxPtrs[i]->ToBeDestroyed())
		{
			boxPtrs[i] = std::move(boxPtrs[boxPtrs.size() - 1]);
			boxPtrs.erase(boxPtrs.end() - 1);
			i--;
		}
	}
}

std::unique_ptr<Box> Game::SplitBox(std::unique_ptr<Box> target, int EdgeDiv)
{
	Box::Properties newProperties = target->GetProperties();
	float size = newProperties.size /= float(EdgeDiv);
	newProperties.position.x -= size * ((float)(EdgeDiv - 1) / 4);
	Vec2 start = newProperties.position;
	std::vector<std::unique_ptr<Box>> split; split.resize(EdgeDiv * EdgeDiv);
	for (int i = 0; i < EdgeDiv * EdgeDiv; newProperties.position.y += size)
	{
		int x = 0;
		for (newProperties.position.x = start.x; x < EdgeDiv;
			newProperties.position.x += size, x++)
		{
			split[i + x] = std::make_unique<Box>(newProperties, world);
		}
		i += x;
	}
	target = std::move(split[0]);
	boxPtrs.insert(boxPtrs.end(), std::make_move_iterator(split.begin() + 1), 
		std::make_move_iterator(split.end()));
	return target;
}

void Game::ComposeFrame()
{
	for( const auto& p : boxPtrs )
	{
		p->Draw( pepe );
	}
}