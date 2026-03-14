#pragma once

#include <cstdint>

namespace Flies
{
	using EntityID = uint32_t;
	using EntityGen = uint32_t;

	struct Entity
	{
		EntityID id;
		EntityGen gen;

		bool operator==(const Entity& other) const
		{
			return (id == other.id && gen == other.gen);
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
	};

	using Fly = Entity;
}
