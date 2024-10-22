// Fill out your copyright notice in the Description page of Project Settings.

#include "SPlayerState.h"
#include "Net/UnrealNetwork.h"

void ASPlayerState::AddKill()
{
	Kills += 1;
	KillsInARow += 1;
	DeathInARow = 0;
}

void ASPlayerState::AddDeath()
{
	Deaths += 1;
	DeathInARow += 1;
	KillsInARow = 0;
}

int ASPlayerState::GetKills()
{
	return Kills;
}

int ASPlayerState::GetDeaths()
{
	return Deaths;
}

void ASPlayerState::Reset()
{
	Deaths = 0;
	Kills = 0;
	KillsInARow = 0;
	DeathInARow = 0;
}

FString ASPlayerState::GetPlayerInfo()
{
	return GetPlayerName() + " (K: " + FString::FromInt(Kills) + "  | D: " + FString::FromInt(Deaths) + ")";
}

void ASPlayerState::SetName(FString S)
{
	SetPlayerName(S);
}

void ASPlayerState::SetMaterialId(int NewMaterialId)
{
	MaterialId = NewMaterialId;
}

void ASPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASPlayerState, Kills);
	DOREPLIFETIME(ASPlayerState, Deaths);
	DOREPLIFETIME(ASPlayerState, MaterialId);
}