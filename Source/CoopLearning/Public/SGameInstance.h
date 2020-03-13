// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SGameInstance.generated.h"


class USUserSaveGame;


UCLASS()
class COOPLEARNING_API USGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	USGameInstance();

protected:

	UPROPERTY(BlueprintReadWrite, Category = "GameInstance")
	USUserSaveGame* UserSaveGame;

	UPROPERTY(BlueprintReadOnly, Category = "GameInstance")
	FString UserSaveGameSlotName;

public:

	USUserSaveGame* GetUserSaveGame();

};
