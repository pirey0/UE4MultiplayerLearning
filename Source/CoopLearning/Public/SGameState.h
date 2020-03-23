// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SGameState.generated.h"


UCLASS()
class COOPLEARNING_API ASGameState : public AGameState
{
	GENERATED_BODY()
	

public:

	UFUNCTION(BlueprintCallable, Category = "GameState")
	FString GetAllPlayersInfo();

	UPROPERTY(BlueprintReadOnly, Category = "GameState")
	bool UnlimitedMags;

};
