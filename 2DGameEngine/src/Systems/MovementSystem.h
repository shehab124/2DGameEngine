#ifndef MOVEMENTSYSTEM_H
#define MOVEMENTSYSTEM_H

#include "../ECS/ECS.h"

class MovementSystem : public System
{
public:
	MovementSystem()
	{
		//TODO:
		// RequireComponent<TransformComponent>();

	}

	void Update()
	{
		// TODO:
		//Loop all entities that the system is interested in
		//for (auto entity : GetEntities())
		//{
		//	//update entity position based on its velocity
		//}

	}
};

#endif