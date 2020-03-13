// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SUserSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class COOPLEARNING_API USUserSaveGame : public USaveGame
{
	GENERATED_BODY()
	

public:

	USUserSaveGame();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UserData")
	FString UserName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UserData")
	float UserPreferredColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UserData")
	FString UserServerDefaultIP;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UserData")
	float UserMouseSensitivity;

};
