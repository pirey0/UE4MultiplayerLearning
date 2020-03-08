// Fill out your copyright notice in the Description page of Project Settings.
#include "SPlayerState.h"
#include "SGameStateBase.h"
#include "SGameModeBase.h"


void ASGameModeBase::ResetAllKDA()
{
	ASGameStateBase* GS = Cast<ASGameStateBase>(GameState);

	for (size_t i = 0; i < GS->PlayerArray.Num(); i++)
	{
		ASPlayerState* PS = Cast<ASPlayerState>(GS->PlayerArray[i]);
		PS->Reset();
	}

}

void ASGameModeBase::PostLogin(APlayerController * NewPlayer)
{
	ASPlayerState* PS = Cast<ASPlayerState>(NewPlayer->PlayerState);
	PS->SetColor(FLinearColor::MakeRandomColor());
	Super::PostLogin(NewPlayer);
}
