// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CoopLearningPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class COOPLEARNING_API ACoopLearningPlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly)
	int Kills;


	UPROPERTY(BlueprintReadOnly)
	int Deaths;

public:
	UFUNCTION(BlueprintCallable)
	void AddKill();

	UFUNCTION(BlueprintCallable)
	void AddDeath();

	int GetKills();

	int GetDeaths();

	UFUNCTION(BlueprintCallable)
	FString GetPlayerInfo();

};
