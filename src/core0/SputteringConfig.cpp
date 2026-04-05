/**
 * @file SputteringConfig.cpp
 * @brief Implementation for the SputteringConfig component.
 *
 * @author Ryan Massie (rmassie)
 * @date 4/4/26
 */

#include "core0/SputteringConfig.h"
#include "USBSerial.h"

// Initialize static members
SputteringProcess::OperatingMode SputteringProcess::mode = SputteringProcess::OperatingMode::Manual;
std::vector<SputteringProcess::StageConfig> SputteringProcess::currentRecipe;
critical_section_t SputteringProcess::configLock;
std::atomic<bool> SputteringProcess::isExecuting{false};

void SputteringProcess::init()
{
    critical_section_init(&configLock);
}

bool SputteringProcess::loadRecipe(const std::vector<StageConfig>& newRecipe)
{
    if (validateRecipe(newRecipe))
    {
        critical_section_enter_blocking(&configLock);
        currentRecipe = newRecipe;
        critical_section_exit(&configLock);
        
        USBSerial::log(Source_Core0, "New recipe loaded successfully");
        return true;
    }
    
    USBSerial::log(Source_Core0, "Recipe rejected: Failed validation");
    return false;
}

bool SputteringProcess::validateRecipe(const std::vector<StageConfig>& recipe)
{
    // ... (logic remains same)
    if (recipe.size() < 3)
    {
        USBSerial::log(Source_Core0, "Validator: Recipe too short (min 3 stages)");
        return false;
    }

    // Must start with PumpDown
    if (recipe.front().stage != ProcessStage::PumpDown)
    {
        USBSerial::log(Source_Core0, "Validator: First stage must be PumpDown");
        return false;
    }

    // Must end with Vent
    if (recipe.back().stage != ProcessStage::Vent)
    {
        USBSerial::log(Source_Core0, "Validator: Last stage must be Vent");
        return false;
    }

    // At this point we know we have at least 3 stages, the first is PumpDown, 
    // and the last is Vent. This implies at least one stage in between.
    
    return true;
}
