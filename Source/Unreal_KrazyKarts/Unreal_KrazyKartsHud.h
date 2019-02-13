// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "Unreal_KrazyKartsHud.generated.h"


UCLASS(config = Game)
class AUnreal_KrazyKartsHud : public AHUD
{
	GENERATED_BODY()

public:
	AUnreal_KrazyKartsHud();

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface
};
