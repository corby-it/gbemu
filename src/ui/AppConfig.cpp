

#include "AppConfig.h"

const char* emulationSpeedToStr(EmulationSpeed es)
{
	switch (es)
	{
	case EmulationSpeed::Quarter: return "25%";
	case EmulationSpeed::Half: return "50%";
	case EmulationSpeed::Full: return "100%";
	case EmulationSpeed::Unbound: return "Unbound";
	default:
		return "???";
	}
}




AppConfig::AppConfig()
    : loadingRes(CartridgeLoadingRes::Ok)
	, emulationSpeed(EmulationSpeed::Full)
	, currentSaveStateErr(SaveStateError::NoError)
	, showMemoryEditor(false)
{}

