// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "CoopLearningGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class COOPLEARNING_API ACoopLearningGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	

public:

	UFUNCTION(BlueprintCallable, Category = "GameState")
	FString GetAllPlayersInfo();

};
