// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPossessSignature, class ASPlayerController*, PC, APawn*, NewPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnPossessSignature, class ASPlayerController*, PC);

UCLASS()
class COOPLEARNING_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	UFUNCTION(BlueprintCallable)
	void RespawnDefaultPawnAndPossess();

	void OnPossess(APawn* InPawn) override;

	void OnUnPossess() override;

public:
	void SpawnSpectatorAndPossess();

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnPossessSignature OnPossessWithAuthority;

	UPROPERTY(BlueprintAssignable, Category = "Events")
		FOnUnPossessSignature OnUnPossessWithAuthority;

	void DelayedRespawnDefaultPawnAndPossess(float DelayTime);
};
