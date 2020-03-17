// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpectatorPawn.h"
#include "TimerManager.h"
#include "SCharacter.h"
#include "SGameMode.h"
#include "Net/UnrealNetwork.h"
#include "SWeapon.h"
#include "SUserSaveGame.h"
#include "SGameInstance.h"

ASPlayerController::ASPlayerController() 
{
	bAutoManageActiveCameraTarget = false;
	bFindCameraComponentWhenViewTarget = true;
}

void ASPlayerController::BlendToController(AController * KillerController, float Time)
{
	if (KillerController && KillerController->GetPawn()) 
	{
		SetViewTargetWithBlend(KillerController->GetPawn(), Time, VTBlend_EaseOut, 0.5f);
	}
}

void ASPlayerController::SetRespawnWeapon(TSubclassOf<ASWeapon> NewWeaponType)
{
	ServerSetRespawnWeapon(NewWeaponType);
}

void ASPlayerController::ServerSetRespawnWeapon_Implementation(TSubclassOf<ASWeapon> NewWeaponType)
{
	RespawnWeapon = NewWeaponType;
	UE_LOG(LogTemp, Log, TEXT("%s: Changed RespawnWeapon to %s"), *GetName(), *NewWeaponType->GetName());
}

bool ASPlayerController::ServerSetRespawnWeapon_Validate(TSubclassOf<ASWeapon> NewWeaponType)
{
	return true;
}

TSubclassOf<ASWeapon> ASPlayerController::GetRespawnWeapon()
{
	return RespawnWeapon;
}

void ASPlayerController::ClientRequestNetUserData_Implementation()
{
	USGameInstance* GI = Cast<USGameInstance>(GetGameInstance());

	ServerSendNetUserData(GI->GetNetUserData());


}

void ASPlayerController::ServerSendNetUserData_Implementation(FNetUserData UserData)
{
	ASGameMode* GM = GetWorld()->GetAuthGameMode<ASGameMode>();

	if (GM)
	{
		GM->SetPlayerMaterialFromId(UserData.UserCharacterMatId, PlayerState);
		GM->SetPlayerName(UserData.UserName, PlayerState);
		UE_LOG(LogTemp, Log, TEXT("Recieved NetUserData from Client"));
	}
}

bool ASPlayerController::ServerSendNetUserData_Validate(FNetUserData UserData)
{
	return true;
}


void ASPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ASCharacter* Character = Cast<ASCharacter>(InPawn);

	if (Character)
	{
		SetViewTarget(InPawn);
	}


	if (Role >= ROLE_Authority)
	{
		OnPossessWithAuthority.Broadcast(this, InPawn);
	}
}

void ASPlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	if (Role >= ROLE_Authority)
	{
		OnUnPossessWithAuthority.Broadcast(this);
	}

}

void ASPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPlayerController, RespawnWeapon);
}