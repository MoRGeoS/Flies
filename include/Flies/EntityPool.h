#pragma once

#include <vector>
#include <span>

#include <Flies/Entity.h>

namespace Flies
{
	class EntityPool
	{
	public:
		Entity Create();
		void Destroy(Entity entity);

		bool IsAlive(Entity entity) const;

		std::span<const EntityGen> Generations() const { return { m_Generations.data(), m_Generations.size() }; }

	private:
		std::vector<EntityGen> m_Generations;
		std::vector<EntityID> m_Free;
	};
}
