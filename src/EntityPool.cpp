#include <Flies/EntityPool.h>

namespace Flies
{
	Entity EntityPool::Create()
	{
		if (!m_Free.empty())
		{
			EntityID id = m_Free.back(); m_Free.pop_back();
			EntityGen gen = m_Generations[id];

			return Entity{ id, gen };
		}

		EntityID id = (EntityID)m_Generations.size();
		EntityGen gen = 0; m_Generations.emplace(m_Generations.cbegin() + id, gen);

		return Entity{ id, gen };
	}

	void EntityPool::Destroy(Entity entity)
	{
		if (IsAlive(entity))
		{
			m_Free.push_back(entity.id);
			m_Generations[entity.id]++;
		}
	}

	bool EntityPool::IsAlive(Entity entity) const
	{
		return (
			m_Generations.size() > entity.id &&
			m_Generations[entity.id] == entity.gen
			);
	}
}
