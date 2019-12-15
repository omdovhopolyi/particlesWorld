#include "World.h"
#include "Math.h"
#include "common_data.h"
#include "common_types.h"
#include "ServiceProvider.h"

#include "SFML/Graphics/Color.hpp"

#include <iostream>
#include <cmath>
#include <random>

World::World()
{
}

World::~World()
{
}

bool World::init()
{
    ServiceProvider::getCommunicationService()->addListener(MessageType::mouseWheelMoved, this);
    ServiceProvider::getCommunicationService()->addListener(MessageType::mouseDown, this);
    ServiceProvider::getCommunicationService()->addListener(MessageType::mouseMoved, this);
    ServiceProvider::getCommunicationService()->addListener(MessageType::mouseUp, this);
    ServiceProvider::getCommunicationService()->addListener(MessageType::spawnParticle, this);

    mSpawnZone.setRadius(mSpawnRadius);
    mSpawnZone.setFillColor(sf::Color::Transparent);
    mSpawnZone.setOutlineColor(sf::Color(100, 100, 100));
    mSpawnZone.setOutlineThickness(1.f);
    mSpawnZone.setOrigin(mSpawnRadius, mSpawnRadius);

    debugInit();

    return true;
}

void World::debugInit()
{
    const DebugConfig& debugConfig = ServiceProvider::getConfigService()->getDebugConfig();

    mSpawnZone.setPosition(debugConfig.debugSpawnPosX, debugConfig.debugSpawnPosY);
}

void World::cleanup()
{
    for (Particle* particle : mParticles)
    {
        delete particle;
    }
}

void World::update(float dt)
{
    accumulateUpdateTime(dt);

    PhysicsService* physics = ServiceProvider::getPhysicsService();
    physics->clear();
    
    for (Particle* particle : mParticles)
    {
        physics->insert(particle);
    }

    physics->resolveCollisions();
    physics->applyGravity(mParticles);
    physics->applyForces();
    physics->dealWithWalls(mParticles);
    
    for (Particle* particle : mParticles)
    {
        particle->update(dt);
    }
}

void World::draw()
{
    ServiceProvider::getPhysicsService()->draw();

    sf::RenderWindow* window = ServiceProvider::getWindowService()->getWindow();
    window->draw(mSpawnZone);

    for (Particle* particle : mParticles)
    {
        particle->draw();
    }
}

void World::handleMessage(MessageType messageType, Message* message)
{
    if (messageType == MessageType::mouseWheelMoved)
    {
        if (MessageMouseWheelMove* wheelMessage = dynamic_cast<MessageMouseWheelMove*>(message))
        {
            bool isZoomIn = wheelMessage->isZoomIn();
            mSpawnRadius += (isZoomIn ? 5.f : -5.f);
            mSpawnRadius = std::max(0.f, mSpawnRadius);
            mSpawnZone.setRadius(mSpawnRadius);
            mSpawnZone.setOrigin(mSpawnRadius, mSpawnRadius);
        }
    }

    if (messageType == MessageType::mouseDown)
    {

    }

    if (messageType == MessageType::mouseMoved)
    {
        if (MessageMouseMove* moveMessage = dynamic_cast<MessageMouseMove*>(message))
        {
            sf::Vector2i mousePos = moveMessage->mMousePos;
            mSpawnZone.setPosition(sf::Vector2f(float(mousePos.x), float(mousePos.y)));
        }
    }

    if (messageType == MessageType::mouseUp)
    {

    }

    if (messageType == MessageType::spawnParticle)
    {
        float spawnPeriod = ServiceProvider::getConfigService()->getParticleSpawnPeriod();

        if (mUpdateTime > spawnPeriod)
        {
            resetUpdateTime();
            createParticle(mSpawnZone.getPosition(), mSpawnRadius);
        }
    }
}

void World::createParticle(const sf::Vector2f& zoneCenter, float zoneRadius)
{
    int randomDistance = static_cast<int>(zoneRadius);

    float randomX = static_cast<float>(Math::randomInt(0, randomDistance));
    float randomY = static_cast<float>(Math::randomInt(0, randomDistance));

    bool negateX = static_cast<bool>(Math::randomInt(0, 1));
    bool negateY = static_cast<bool>(Math::randomInt(0, 1));

    randomX = (negateX ? -randomX : randomX);
    randomY = (negateY ? -randomY : randomY);

    sf::Vector2f randomVector(randomX, randomY);
    Math::normalizeThis(randomVector);

    float randomLength = static_cast<float>(Math::randomInt(0, randomDistance));

    randomVector *= randomLength;

    sf::Vector2f particlePos = zoneCenter + randomVector;

    float particleRadius = ServiceProvider::getConfigService()->getParticleRadius();
    float drawRadius = ServiceProvider::getConfigService()->getParticleDrawRadius();

    Particle* particle = new Particle();
    particle->setPosition(particlePos);
    particle->setRadius(drawRadius);

    mParticles.push_back(particle);
}

void World::accumulateUpdateTime(float dt)
{
    mUpdateTime += dt;
}

void World::resetUpdateTime()
{
    mUpdateTime = 0.f;
}
