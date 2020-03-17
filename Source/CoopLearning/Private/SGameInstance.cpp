// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameInstance.h"
#include "SUserSaveGame.h"

USGameInstance::USGameInstance() 
{
	UserSaveGameSlotName = "UserData";
}

USUserSaveGame * USGameInstance::GetUserSaveGame()
{
	return UserSaveGame;
}

FNetUserData USGameInstance::GetNetUserData()
{
	FNetUserData UserData = FNetUserData();
	
	UserData.UserName = UserSaveGame->UserName;
	UserData.UserCharacterMatId = UserSaveGame->UserCharacterMatId;

	return UserData;
}
