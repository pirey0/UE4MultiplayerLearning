// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpectatorPawn.h"
#include "TimerManager.h"
#include "SCharacter.h"
#include "SGameModeBase.h"
#include "Net/UnrealNetwork.h"
#include "SWeapon.h"

ASPlayerController::ASPlayerController() 
{
	bAutoManageActiveCameraTarget = false;
	bFindCameraComponentWhenViewTarget = true;
}

void ASPlayerController::RespawnDefaultPawnAndPossess()
{
	if (Role >= ROLE_Authority)
	{
		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();

		ASGameModeBase* GMB = Cast<ASGameModeBase>(GameMode);
		AActor* PlayerStart = nullptr;
		if (GMB)
		{
			PlayerStart = GMB->ChoseBestRespawnPlayerStart(this);
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Using default PlayerStart selection"));
			PlayerStart = GameMode->ChoosePlayerStart(this);
		}



		UClass* PawnClass = GameMode->GetDefaultPawnClassForController(this);

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector NewSpawnLocation = PlayerStart->GetActorLocation();
		FRotator NewSpawnRotator = PlayerStart->GetActorRotation();

		APawn* SpawnedActor = GetWorld()->SpawnActor<APawn>(PawnClass, NewSpawnLocation, NewSpawnRotator, SpawnParameters);
		Possess(SpawnedActor);
	}

}

void ASPlayerController::SpawnSpectatorAndPossess()
{

	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();

	ASpectatorPawn* spectatorPawn = SpawnSpectatorPawn();

	Possess(spectatorPawn);
}

void ASPlayerController::DelayedRespawnDefaultPawnAndPossess(float DelayTime)
{
	UE_LOG(LogTemp, Log, TEXT("Called Respawn Pawn"));
	GetWorldTimerManager().SetTimer(TimerHandle_Respawn, this, &ASPlayerController::RespawnDefaultPawnAndPossess, DelayTime, false);
}

void ASPlayerController::BlendToController(AController * KillerController, float Time)
{
	if (KillerController->GetPawn()) 
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