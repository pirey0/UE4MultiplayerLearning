// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopLearningGameModeBase.h"
#include "CoopLearningGameStateBase.h"
#include "CoopLearningPlayerState.h"

void ACoopLearningGameModeBase::ResetAllKDA() 
{
	ACoopLearningGameStateBase* GS = Cast<ACoopLearningGameStateBase>(GameState);

	for (size_t i = 0; i < GS->PlayerArray.Num(); i++)
	{
		ACoopLearningPlayerState* PS = Cast<ACoopLearningPlayerState>(GS->PlayerArray[i]);
		PS->Reset();
	}
	
	

}