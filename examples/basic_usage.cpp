// Example of using reflection-enabled classes
#include "Player.h"
#include "Engine/Public/Core/TypeRegistry.h"
#include "Engine/Public/Core/BinarySerializer.h"
#include <iostream>
#include <memory>

int main() {
    // Create a player instance
    auto player = std::make_unique<Game::Player>();
    player->id = 1;
    player->name = "TestPlayer";
    player->health = 80;
    player->mana = 60;
    
    // Test reflection
    std::cout << "Class name: " << player->GetClassName() << "\n";
    
    const auto* type = player->GetType();
    if (type) {
        std::cout << "Type name: " << type->GetName() << "\n";
        std::cout << "Properties count: " << type->GetProperties().size() << "\n";
        
        // List properties
        for (const auto* prop : type->GetProperties()) {
            std::cout << "Property: " << prop->GetName();
            if (prop->HasFlag(Engine::Core::GProperty::Flags::Save)) {
                std::cout << " [Save]";
            }
            if (prop->HasFlag(Engine::Core::GProperty::Flags::Edit)) {
                std::cout << " [Edit]";
            }
            std::cout << "\n";
        }
    }
    
    // Test serialization
    Engine::Core::BinarySerializer serializer;
    player->Serialize(serializer);
    std::cout << "Serialized " << serializer.GetData().size() << " bytes\n";
    
    return 0;
}
