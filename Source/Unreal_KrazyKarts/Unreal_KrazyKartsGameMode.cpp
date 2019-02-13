// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Unreal_KrazyKartsGameMode.h"
#include "Unreal_KrazyKartsPawn.h"
#include "Unreal_KrazyKartsHud.h"

AUnreal_KrazyKartsGameMode::AUnreal_KrazyKartsGameMode()
{
	DefaultPawnClass = AUnreal_KrazyKartsPawn::StaticClass();
	HUDClass = AUnreal_KrazyKartsHud::StaticClass();
}
