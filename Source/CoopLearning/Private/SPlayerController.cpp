// Fill out your copyright notice in the Description page of Project Settings.


#include "SPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/SpectatorPawn.h"
#include "TimerManager.h"

void ASPlayerController::RespawnDefaultPawnAndPossess()
{
	if (Role >= ROLE_Authority)
	{
		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();

		AActor* PlayerStart = GameMode->ChoosePlayerStart(this);

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
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ASPlayerController::SpawnSpectatorAndPossess, DelayTime, false);
}

void ASPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

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

