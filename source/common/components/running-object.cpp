#include "running-object.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    // Reads linearVelocity & angularVelocity from the given json object
    void RunningObject::deserialize(const nlohmann::json& data){
        if(!data.is_object()) return;
        // linearVelocity = data.value("linearVelocity", linearVelocity);
        // angularVelocity = glm::radians(data.value("angularVelocity", angularVelocity));
    }
}