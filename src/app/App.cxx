//
// Carson R - 1/23/2023
//

// Dependencies
#include <boost/thread/shared_mutex.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered/unordered_map.hpp>
// Local Includes
#include "app/App.hxx"

using namespace TitanOfAir;

// Singleton

App *sharedInstance = nullptr;

App *App::shared()
{
    if (sharedInstance == nullptr)
    {
        sharedInstance = new App();
        sharedInstance->eMutex->lock();
        sharedInstance->cMutex->lock();
    }
    return sharedInstance;
}

// Accessors

std::size_t App::eCount()
{
    return this->entities->size();
}

std::size_t App::cCount()
{
    return this->components->size();
}


// Construction & Destruction
App::App()
{
    this->eMutex = new boost::shared_mutex{};
    this->entities = new EntityContainer{};
    this->cMutex = new boost::shared_mutex{};
    this->components = new ComponentContainer{};
}

Response App::init()
{
    this->eMutex->unlock();
    this->cMutex->unlock();
    return Response::APP_SUCCESS;
}

App::~App()
{
    eMutex->lock();
    cMutex->lock();
    assert(this->entities->empty());
    delete this->entities;
    eMutex->unlock();
    delete eMutex;

    assert(this->components->empty());
    delete this->components;
    cMutex->unlock();
    delete cMutex;
}


Response App::add(Entity *e, Response* r)
{
    this->eMutex->lock_upgrade();
    if (has(e))
    {
        this->eMutex->unlock_upgrade();
        return  Response::APP_ENTY_PRESENT;
    }
    this->eMutex->unlock_upgrade_and_lock();
    auto res = this->entities->emplace(e->getID(), EntityTuple{e, r});
    this->eMutex->unlock();
    if (!res.second)
        return Response::APP_ENTY_OP_ERROR;
    return Response::APP_ENTY_OP_SUCCESS;
}

Response App::remove(Entity *e)
{
    this->eMutex->lock_upgrade();
    if (!has(e))
    {
        this->eMutex->unlock_upgrade();
        return Response::APP_ENTY_NOT_PRESENT;
    }
    this->eMutex->unlock_upgrade_and_lock();
    auto res = this->entities->erase(e->getID());
    this->eMutex->unlock();
    if (res != 1)
        return Response::APP_ENTY_OP_ERROR;
    return Response::APP_ENTY_OP_SUCCESS;
}

const Response* App::getStatusFor(Entity *e) const
{
    this->eMutex->lock_shared();
    if (!has(e))
        return nullptr;
    auto data = this->entities->at(e->getID());
    this->eMutex->unlock_shared();
    return const_cast<const Response*>(data.get<1>());
}

Response App::add(Component *c, Response* r)
{
    this->cMutex->lock_upgrade();
    if (has(c))
    {
        this->cMutex->unlock_upgrade();
        return  Response::APP_ENTY_PRESENT;
    }
    this->cMutex->unlock_upgrade_and_lock();
    auto res = this->components->emplace(c->getID(), ComponentTuple{c, r});
    this->cMutex->unlock();
    if (!res.second)
        return Response::APP_ENTY_OP_ERROR;
    return Response::APP_ENTY_OP_SUCCESS;
}

Response App::remove(Component *c)
{
    this->cMutex->lock_upgrade();
    if (!has(c))
    {
        this->cMutex->unlock_upgrade();
        return Response::APP_ENTY_NOT_PRESENT;
    }
    this->cMutex->unlock_upgrade_and_lock();
    auto res = this->components->erase(c->getID());
    this->cMutex->unlock();
    if (res != 1)
        return Response::APP_ENTY_OP_ERROR;
    return Response::APP_ENTY_OP_SUCCESS;
}

const Response* App::getStatusFor(Component *c) const
{
    this->cMutex->lock_shared();
    if (!has(c))
        return nullptr;
    auto data = this->components->at(c->getID());
    this->cMutex->unlock_shared();
    return const_cast<const Response*>(data.get<1>());
}

bool App::has(Entity *e) const
{
    return this->entities->find(e->getID()) != this->entities->end();
}

bool App::has(Component *c) const
{
    return this->components->find(c->getID()) != this->components->end();
}

size_t App::clearECS()
{
    this->eMutex->lock();
    size_t removed = 0;
    for(auto &entity : *entities)
    {
        delete entity.second.get<0>(), entity.second.get<1>();
        ++removed;
    }
    this->eMutex->unlock();
    this->cMutex->lock();
    for (auto &component : *components)
    {
        delete component.second.get<0>(), component.second.get<1>();
        ++removed;
    }
    this->cMutex->unlock();
    return removed;
}

