// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SUserSaveGame.h"
#include "SPlayerController.generated.h"

class ASWeapon;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPossessSignature, class ASPlayerController*, PC, APawn*, NewPawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnPossessSignature, class ASPlayerController*, PC);

UCLASS()
class COOPLEARNING_API ASPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASPlayerController();

protected:

	void OnPossess(APawn* InPawn) override;

	void OnUnPossess() override;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PlayerController")
	TSubclassOf<ASWeapon> RespawnWeapon;

public:

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPossessSignature OnPossessWithAuthority;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUnPossessSignature OnUnPossessWithAuthority;

	UFUNCTION(BlueprintCallable, Category = "PlayerController")
	void BlendToController(AController* KillerController, float Time);

	UFUNCTION(BlueprintCallable, Category = "PlayerController")
	void SetRespawnWeapon(TSubclassOf<ASWeapon> NewWeaponType);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetRespawnWeapon(TSubclassOf<ASWeapon> NewWeaponType);

	TSubclassOf<ASWeapon> GetRespawnWeapon();

	UFUNCTION(Client, Reliable)
	void ClientRequestNetUserData();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSendNetUserData(FNetUserData UserData);
};
