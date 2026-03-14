#include <Flies/World.h>

namespace Flies
{
	World::~World() 
	{
		Cleanup(); 
	}

	Entity World::CreateEntity()
	{
		return m_EntityPool.Create();
	}

	void World::DestroyEntity(Entity entity)
	{
		if (IsAlive(entity))
		{
			for (auto& entry : m_Storages)
			{
				if (entry.Storage)
				{
					entry.Remove(entity.id);
				}
			}
			m_EntityPool.Destroy(entity);
		}
	}

	bool World::IsAlive(Entity entity) const
	{
		return m_EntityPool.IsAlive(entity);
	}

	void World::Cleanup()
	{
		for (auto& entry : m_Storages)
		{
			delete entry.Storage;
			entry.Storage = nullptr;
		}
	}
}
