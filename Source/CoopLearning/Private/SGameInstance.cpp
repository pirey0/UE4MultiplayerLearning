// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameInstance.h"


USGameInstance::USGameInstance() 
{
	UserSaveGameSlotName = "UserData";
}

USUserSaveGame * USGameInstance::GetUserSaveGame()
{
	return UserSaveGame;
}
