// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SGameModeBase.generated.h"


class ASCharacter;
class ASPlayerController;

UCLASS()
class COOPLEARNING_API ASGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	UFUNCTION(Exec, Category = "Cheats")
		void ResetAllKDA();

	UFUNCTION()
	virtual void OnPlayerPossesWithAuthority(ASPlayerController* PC, APawn* NewPawn);
	
	UFUNCTION()
	virtual void OnPlayerCharacterDeath( ASCharacter* Character, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode")
	float PlayerRespawnTime;

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	virtual void CheckForGameEnd();

	UFUNCTION(Exec, Category = "Cheats")
	virtual void RestartGameMode();

public:

	virtual void PostLogin(APlayerController* NewPlayer) override;

};
