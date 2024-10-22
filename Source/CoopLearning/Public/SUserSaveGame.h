// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SUserSaveGame.generated.h"


USTRUCT()
struct FNetUserData
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FString UserName;

	UPROPERTY()
	float UserCharacterMatId;

};

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
	int UserCharacterMatId;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UserData")
	FString UserServerDefaultIP;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UserData")
	float UserMouseSensitivity;

};
