#ifndef ECS_H
#define ECS_H

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>

const unsigned int MAX_COMPONENTS = 32;


/////////////////////////
// SIGNATURE
// WE USE A BITSET TO KEEP TRACK OF WHICH COMPONENTS AN ENTITY HAS.
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


/////////////////////////////////////////////////////
// Pool

class IPool
{
public:
	virtual ~IPool() {}
};

/////////////////////////////////////////////////////
// A pool is just a vector of objects of type T
// Wrapper around entity?
/////////////////////////////////////////////////////
template<typename T>
class Pool: public IPool
{
private:
	std::vector<T> data;

public:
	Pool(int size = 100)
	{
		data.resize(size);
	}
	virtual ~Pool() = default;

	bool isEmpty() const
	{
		return data.empty();
	}

	int GetSize() const
	{
		return data.size();
	}

	void Resize(int n)
	{
		data.resize(n);
	}

	void Clear()
	{
		data.clear();
	}

	void Add(T object)
	{
		data.push_back(object);
	}

	void Set(int index, T object)
	{
		data[index] = object;
	}

	T& Get(int index)
	{
		return static_cast<T&>data[index];
	}

	T& operator [](unsigned int index)
	{
		return data[index];
	}
};




// Registry (coordinator)
// The Registry manages the creation and destruction of entities, as well
// adding systems and adding components to entities
class Registry
{
private:
	int  numEntities = 0;

	// Vector of component pools, each pool contains all the data for a certain component type
	// Vector index = Component type id
	// [Pool index = entity id]
	std::vector<IPool*> componentPools;

	// Vector of component signatures.
	// The signature lets us know which components are turned "on" for an entity
	// [vector index = entity id]
	std::vector<Signature> entityComponentSignatures;


	// Map of active systems
	// [index = system typeid]
	std::unordered_map<std::type_index, System*> systems;

	// Set of entities that are flagged to be added of removed in the next registry Update()
	std::set<Entity> entitiesToBeAdded;
	std::set<Entity> entitiesToBeKilled;

public:
	Registry() = default;

	void Update();

	// Entity management
	Entity CreateEntity();
	////////////////////////////////////////////////////////
	
	// Component management
	template <typename TComponent, typename ...TArgs> 
	void AddComponent(Entity entity, TArgs&& ...args);

	template <typename TComponent>
	void RemoveComponent(Entity entity);

	template <typename TComponent>
	bool HasComponent(Entity entity) const;
	////////////////////////////////////////////////////////
	
	// System management
	template <typename TSystem, typename ...TArgs>
	void AddSystem(TArgs&& ...args);
	
	template <typename TSystem>
	void RemoveSystem();

	template <typename TSystem>
	bool HasSystem() const;

	template <typename TSystem>
	TSystem& GetSystem() const;
	////////////////////////////////////////////////////////

	// Checks the component signature of an entity and add the entity to the systems that are interested in it
	void AddEntityToSystems(Entity entity);
};

// COMPONENT TEMPLATES
template <typename TComponent>
void System::RequireComponent()
{
	const auto cpmponentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

template<typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args)
{
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	if (componentId >= componentPools.size())
	{
		componentPools.resize(componentId + 1, nullptr);
	}

	if (!componentPools[componentId])
	{
		Pool<TComponent>* newComponentPool = new Pool<TComponent>();
		componentPools[componentId] = newComponentPool;
	}

	Pool<TComponent>* componentPool = componentPools[componentId];

	if (entityId >= componentPool->GetSize())
	{
		componentPool->Resize(numEntities);
	}

	TComponent newComponent(std::forward<TArgs>(args)...);

	componentPool->Set(entityId, newComponent);

	entityComponentSignatures[entityId].set(componentId);
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity)
{
	const auto componentId = Component<TComponent::GetId();
	const auto entityId = entity.GetId();

	entityComponentSignatures[entityId].set(componentId, false);
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const
{
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	return entityComponentSignatures[entityId].test(componentId);
}
//////////////////////////////////////////////////////////////////

// SYSTEM TEMPLATES
template <typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args)
{
	TSystem* newSystem(new TSystem(std::forward<TArgs>(args)...));
	systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem>
void Registry::RemoveSystem()
{
	auto system = systems.find(std::type_index(typeid(TSystem)));
	systems.erase(system);
}

template <typename TSystem>
bool Registry::HasSystem() const
{
	return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template <typename TSystem>
TSystem& Registry::GetSystem() const
{
	auto system = systems.find(std::type_index(typeid(TSystem)));
	return *(std::static_pointer_cast<TSystem>(system->second));
}
//////////////////////////////////////////////////////////////////
#endif 