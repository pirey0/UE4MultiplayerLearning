// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopLearningPlayerState.h"

void ACoopLearningPlayerState::AddKill()
{
	Kills += 1;
}

void ACoopLearningPlayerState::AddDeath()
{
	Deaths += 1;
}

int ACoopLearningPlayerState::GetKills()
{
	return Kills;
}

int ACoopLearningPlayerState::GetDeaths()
{
	return Deaths;
}

FString ACoopLearningPlayerState::GetPlayerInfo()
{
	
	return GetPlayerName() + " (K: " + FString::FromInt(Kills) + "  | D: " + FString::FromInt(Deaths) + ")";

}

void ACoopLearningPlayerState::SetName(FString S)
{
	SetPlayerName(S);
}
