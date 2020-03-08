// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class COOPLEARNING_API ASGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	

public:

	UFUNCTION(BlueprintCallable, Category = "GameState")
		FString GetAllPlayersInfo();

};
