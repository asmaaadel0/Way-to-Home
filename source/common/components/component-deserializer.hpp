#pragma once

#include "../ecs/entity.hpp"
#include "camera.hpp"
#include "mesh-renderer.hpp"
#include "light.hpp"
#include "free-camera-controller.hpp"
#include "movement.hpp"
#include "running-object.hpp"
#include "collision.hpp"

namespace our {

    // Given a json object, this function picks and creates a component in the given entity
    // based on the "type" specified in the json object which is later deserialized from the rest of the json object
    inline void deserializeComponent(const nlohmann::json& data, Entity* entity){
        std::string type = data.value("type", "");
        Component* component = nullptr;
        //TODO: (Req 8) Add an option to deserialize a "MeshRendererComponent" to the following if-else statement
        // checking whether a given variable "type" is equal to the ID of a MeshRendererComponent, 
        // and if so, creating a new instance of the component and assigning it to the variable "component" of the entity.
        if(type == CameraComponent::getID()){
            component = entity->addComponent<CameraComponent>();
        } else if (type == FreeCameraControllerComponent::getID()) {
            component = entity->addComponent<FreeCameraControllerComponent>();
        } else if (type == MovementComponent::getID()) {
            component = entity->addComponent<MovementComponent>();
        } 
        else if(type == LightComponent::getID()){
            component = entity->addComponent<LightComponent>();
        }
        else if (type == MeshRendererComponent::getID()) {
            component = entity->addComponent<MeshRendererComponent>();
        } else if (type == RunningObject::getID()) {
            component = entity->addComponent<RunningObject>();
        } else if(type == CollisionComponent::getID()){
            component = entity->addComponent<CollisionComponent>();
        } 
        if(component) component->deserialize(data);
    }

}