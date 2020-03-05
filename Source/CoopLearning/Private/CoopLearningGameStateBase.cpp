// Fill out your copyright notice in the Description page of Project Settings.

#include "CoopLearningGameStateBase.h"
#include "CoopLearningPlayerState.h"

FString ACoopLearningGameStateBase::GetAllPlayersInfo()
{
	FString Content;

	for (int i = 0; i < PlayerArray.Num(); i++)
	{
		ACoopLearningPlayerState* PlayerState = Cast<ACoopLearningPlayerState>(PlayerArray[i]);

		Content.Append(PlayerState->GetPlayerInfo());
		Content.Append(" \n ");

	}


	return Content;
}