// Fill out your copyright notice in the Description page of Project Settings.


#include "CoopLearningPlayerState.h"
#include "Net/UnrealNetwork.h"

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

void ACoopLearningPlayerState::Reset()
{
	Deaths = 0;
	Kills = 0;
}

FString ACoopLearningPlayerState::GetPlayerInfo()
{
	
	return GetPlayerName() + " (K: " + FString::FromInt(Kills) + "  | D: " + FString::FromInt(Deaths) + ")";

}

void ACoopLearningPlayerState::SetName(FString S)
{
	SetPlayerName(S);
}


void ACoopLearningPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACoopLearningPlayerState, Kills);
	DOREPLIFETIME(ACoopLearningPlayerState, Deaths);
}