#ifndef ECS_H
#define ECS_H

#include <bitset>
#include <vector>

const unsigned int MAX_COMPONENTS = 32;


/////////////////////////
// SIGNATURE
// WE USE A BITSET TO KEEP TRACK OF WHICH COMPNENTS AN ENTITY HAS.
// AND ALSO KEEP TRACK OF WHICH ENITITIES A SYSTEM IS INTERESTED IN.
/////////////////////////
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent
{
protected:
	static int nextId;
};

template <typename T>
class Component: public IComponent
{
	//Returns the unique id of the Component<T>
	static auto GetId()
	{
		static auto id = nextId++;
		return id;
	}
};

class Entity
{
	private:
		int id;

	public:
		Entity(int id) : id(id) {};
		Entity(const Entity& entity) = default;
		int GetId() const;

		Entity& operator =(const Entity& other) = default;
		bool operator ==(const Entity& other) const { return id == other.id; }
		bool operator !=(const Entity& other) const { return id != other.id; }
		bool operator >(const Entity& other) const { return id > other.id; }
		bool operator <(const Entity& other) const { return id < other.id; }
};

class System
{
	private:
		Signature componentSignature;
		std::vector<Entity> entities;

	public:
		System() = default;
		~System() = default;

		void AddEntityToSystem(Entity entity);
		void RemoveEntityFromSystem(Entity entity);
		std::vector<Entity> GetSystemEntities() const;
		const Signature& GetComponentSignature() const;

		// Defines the component type that entities must have to be considered by the system
		template <typename TComponent> void RequireComponent();
};

class Registry
{

};

template <typename TComponent>
void System::RequireComponent()
{
	const auto cpmponentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

#endif 