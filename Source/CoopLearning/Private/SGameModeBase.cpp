// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameModeBase.h"
#include "SPlayerState.h"
#include "SGameStateBase.h"
#include "SPlayerController.h"
#include "SCharacter.h"


void ASGameModeBase::ResetAllKDA()
{
	ASGameStateBase* GS = Cast<ASGameStateBase>(GameState);

	for (size_t i = 0; i < GS->PlayerArray.Num(); i++)
	{
		ASPlayerState* PS = Cast<ASPlayerState>(GS->PlayerArray[i]);
		PS->Reset();
	}

}

void ASGameModeBase::CheckForGameEnd()
{

}

void ASGameModeBase::RestartGameMode()
{
	ResetAllKDA();
}

void ASGameModeBase::PostLogin(APlayerController * NewPlayer)
{
	ASPlayerState* PS = Cast<ASPlayerState>(NewPlayer->PlayerState);
	PS->SetColor(FLinearColor::MakeRandomColor());
	Super::PostLogin(NewPlayer);
	
	ASPlayerController* PC = Cast<ASPlayerController>(NewPlayer);

	if (PC)
	{
		PC->OnPossessWithAuthority.AddDynamic(this, &ASGameModeBase::OnPlayerPossesWithAuthority);
		 
		ASCharacter* NewCharacter = Cast<ASCharacter>(PC->GetPawn());

		if (NewCharacter) 
		{
			NewCharacter->OnDeath.AddDynamic(this, &ASGameModeBase::OnPlayerCharacterDeath);
		}
	}
}

void ASGameModeBase::OnPlayerPossesWithAuthority(ASPlayerController * PC, APawn * NewPawn)
{
	ASCharacter* NewCharacter = Cast<ASCharacter>(NewPawn);

	if (NewCharacter)
	{
		NewCharacter->OnDeath.AddDynamic(this, &ASGameModeBase::OnPlayerCharacterDeath);
	}

}

void ASGameModeBase::OnPlayerCharacterDeath(ASCharacter * Character, AController * InstigatedBy, AActor * DamageCauser)
{
	ASPlayerController* PC = Cast<ASPlayerController>(Character->Controller);
	if (PC)
	{
		PC->DelayedRespawnDefaultPawnAndPossess(PlayerRespawnTime);
	}

	ASPlayerState* DierState = Cast<ASPlayerState>(Character->GetPlayerState());
	ASPlayerState* InstigatorState = Cast<ASPlayerState>(InstigatedBy->PlayerState);

	DierState->AddDeath();
	InstigatorState->AddKill();

	UE_LOG(LogTemp, Log, TEXT("%s killed %s"), *DierState->GetPlayerName(), *InstigatorState->GetPlayerName());

	CheckForGameEnd();
}
