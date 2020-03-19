// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SGameMode.generated.h"


class ASCharacter;
class ASPlayerController;
class APlayerState;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerDeathSignature, class ASPlayerController*, Dier, class ASPlayerController*, Killer);

UCLASS()
class COOPLEARNING_API ASGameMode : public AGameMode
{
	GENERATED_BODY()
	
protected:

	UFUNCTION(Exec, Category = "Cheats")
	void ResetAllKDA();

	UFUNCTION()
	virtual void OnPlayerPossesWithAuthority(ASPlayerController* PC, APawn* NewPawn);
	
	UFUNCTION()
	virtual void OnPlayerCharacterDeath( ASCharacter* Character, AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly, Category = "GameMode")
	bool SkipCountdown;

public:

	virtual void PostLogin(APlayerController* NewPlayer) override;

	AActor* ChoseBestRespawnPlayerStart(AController* Player);

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetPlayerName(FString NewName, APlayerState* PlayerState);

	UFUNCTION(BlueprintCallable, Category = "GameMode")
	void SetPlayerMaterialFromId(int MatId, APlayerState* PlayerState);

	virtual void RestartPlayer(AController* NewPlayer) override;

	virtual void RestartPlayerDelayed(AController* Player, float Delay);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDeathSignature OnPlayerDeath;

	void RestartPlayerInProgress(AController * Player);

};
