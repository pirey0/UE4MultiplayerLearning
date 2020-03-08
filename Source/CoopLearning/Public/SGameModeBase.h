// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class COOPLEARNING_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	UFUNCTION(Exec, Category = "Cheats")
		void ResetAllKDA();

public:

	virtual void PostLogin(APlayerController* NewPlayer) override;

};
