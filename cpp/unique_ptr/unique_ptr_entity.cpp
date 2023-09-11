#include <iostream>
#include <memory>
#include <vector>

struct Entity;

struct floatVec2
{
  float x, y;
};

struct Transform
{
  floatVec2 pos;
};

struct Modifier
{
  virtual void Update(float dt, Entity& parent) = 0;
};


struct Entity
{
  Transform transform;
  std::vector<std::unique_ptr<Modifier>> modifiers;
  void Update(float dt) 
  {
    for (auto& modifier : modifiers)
    {
      modifier->Update(dt, *this);
    }
  }
};

struct PhysicsModifier : public Modifier
{
  floatVec2 vel;

  void Update(float dt, Entity& parent) override
  {
    parent.transform.pos.x += vel.x * dt;
    parent.transform.pos.y += vel.y * dt;
    std::cout << "PhysicsModifier::Update() x: " << parent.transform.pos.x << ", y: " << parent.transform.pos.y << std::endl;
  }
};


int main()
{
  Entity e;
  e.transform.pos.x = 1.0f;
  e.transform.pos.y = 2.0f;
  
  auto physicsModifier = std::make_unique<PhysicsModifier>();
  physicsModifier->vel.x = 1.0f;
  physicsModifier->vel.y = -1.0f;
  e.modifiers.push_back(std::move(physicsModifier));

  e.Update(0.1f);
  e.Update(0.1f);
  e.Update(0.1f);

}

