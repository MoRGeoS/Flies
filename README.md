# Flies
A simple ECS library written in C++.  
This project is based on a sparse set and doesn't use archetypes.  

## Content
- [What's new?](#whats-new)
- [Requirements](#requirements)
- [Download](#download)
- [Example](#example)

## What's new?
- Cross-platform added (Untested)

## Requirements
- CMake 3.19
- C++ 20

## Download
You'll need Git installed first.  
Clone the repository:
```
git clone https://github.com/MoRGeoS/Flies.git
```
Then build with CMake:
```
cmake -B build
```
That's it! You're free to use the library and report bugs!

## Example
Creating entities and processing physics with a simple ECS view:
```cpp
#include <Flies/World.h>

struct Position
{
	float x, y;
};

struct Velocity
{
	float dx, dy;
};

void ProcessPhysics(Flies::World& world, float dt);

int main()
{
	Flies::World world;

	for (int i = 0; i < 10; i++)
	{
		const auto entity = world.CreateEntity();
		world.InsertComponent(entity, Position{});
		if (i % 2)
		{
			world.InsertComponent(entity, Velocity{ 1, 1 });
		}
	}

	ProcessPhysics(world, 1.0f);
}

void ProcessPhysics(Flies::World& world, float dt)
{
	auto view = world.CreateView<Position, const Velocity>();

	for (auto [pos, vel] : view.ForEach())
	{
		pos.x += vel.dx * dt;
		pos.y += vel.dy * dt;
	}
}

```
