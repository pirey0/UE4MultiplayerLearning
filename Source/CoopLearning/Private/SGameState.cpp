// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameState.h"
#include "SPlayerState.h"

FString ASGameState::GetAllPlayersInfo()
{
	FString Content;

	for (int i = 0; i < PlayerArray.Num(); i++)
	{
		ASPlayerState* PlayerState = Cast<ASPlayerState>(PlayerArray[i]);

		Content.Append(PlayerState->GetPlayerInfo());
		Content.Append(" \n ");

	}

	return Content;
}

