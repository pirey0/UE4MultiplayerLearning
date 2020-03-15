// Fill out your copyright notice in the Description page of Project Settings.

#include "SGameMode.h"
#include "SPlayerState.h"
#include "SGameState.h"
#include "SPlayerController.h"
#include "SCharacter.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

void ASGameMode::ResetAllKDA()
{
	ASGameState* GS = Cast<ASGameState>(GameState);

	for (size_t i = 0; i < GS->PlayerArray.Num(); i++)
	{
		ASPlayerState* PS = Cast<ASPlayerState>(GS->PlayerArray[i]);
		PS->Reset();
	}

}

void ASGameMode::PostLogin(APlayerController * NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	ASPlayerController* PC = Cast<ASPlayerController>(NewPlayer);

	if (PC)
	{
		PC->OnPossessWithAuthority.AddDynamic(this, &ASGameMode::OnPlayerPossesWithAuthority);
		 
		ASCharacter* NewCharacter = Cast<ASCharacter>(PC->GetPawn());

		if (NewCharacter) 
		{
			NewCharacter->OnDeath.AddDynamic(this, &ASGameMode::OnPlayerCharacterDeath);
		}

		PC->ClientRequestNetUserData();
	}
}

AActor * ASGameMode::ChoseBestRespawnPlayerStart(AController* Player)
{
	UWorld* World = GetWorld();
	APlayerStart* BestStart = nullptr;
	float MinScore = TNumericLimits<float>::Max();

	TArray<AActor*> Characters;
		
	UGameplayStatics::GetAllActorsOfClass( this, ASCharacter::StaticClass(), Characters);



	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		float Score = 0;

			for (size_t i = 0; i < Characters.Num(); i++)
			{
				float Dist = FVector::Distance(PlayerStart->GetActorLocation(), Characters[i]->GetActorLocation());

				Score += 1 / FMath::Sqrt(Dist);

			}

		//UE_LOG(LogTemp, Log, TEXT("%s has a Score of: %f"), *PlayerStart->GetName(), Score);
		if (Score < MinScore) 
		{
			MinScore = Score;
			BestStart = PlayerStart;
		}
	}



	return BestStart;

}

void ASGameMode::SetPlayerName(FString NewName, APlayerState * PlayerState)
{
	ASPlayerState* PS = Cast<ASPlayerState>(PlayerState);

	if (PS)
	{
		PS->SetName(NewName);
	}

}

void ASGameMode::SetPlayerColor(FLinearColor NewColor, APlayerState * PlayerState)
{
	ASPlayerState* PS = Cast<ASPlayerState>(PlayerState);

	if (PS)
	{
		PS->SetColor(NewColor);
	}
}

void ASGameMode::SetPlayerColorFromFloat(float NewColorAsFloat, APlayerState * PlayerState)
{
	FLinearColor Color;

	UKismetMathLibrary::LinearColor_SetFromHSV(Color, NewColorAsFloat, 1, 1, 1);
	SetPlayerColor(Color, PlayerState);
}

void ASGameMode::RestartPlayer(AController * NewPlayer)
{
	AActor* PlayerStart = nullptr;
	PlayerStart = ChoseBestRespawnPlayerStart(NewPlayer);

	UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector NewSpawnLocation = PlayerStart->GetActorLocation();
	FRotator NewSpawnRotator = PlayerStart->GetActorRotation();

	APawn* SpawnedActor = GetWorld()->SpawnActor<APawn>(PawnClass, NewSpawnLocation, NewSpawnRotator, SpawnParameters);
	NewPlayer->Possess(SpawnedActor);
}

void ASGameMode::RestartPlayerDelayed(AController * Player, float Delay)
{
	UE_LOG(LogTemp, Log, TEXT("Called Respawn Pawn"));
	FTimerHandle UniqueHandle;
	FTimerDelegate RespawnDelegate = FTimerDelegate::CreateUObject(this, &ASGameMode::RestartPlayer, Player);

	GetWorldTimerManager().SetTimer(UniqueHandle, RespawnDelegate, Delay, false);
}

void ASGameMode::OnPlayerPossesWithAuthority(ASPlayerController * PC, APawn * NewPawn)
{
	ASCharacter* NewCharacter = Cast<ASCharacter>(NewPawn);

	if (NewCharacter)
	{
		NewCharacter->OnDeath.AddDynamic(this, &ASGameMode::OnPlayerCharacterDeath);
	}

}

void ASGameMode::OnPlayerCharacterDeath(ASCharacter * Character, AController * InstigatedBy, AActor * DamageCauser)
{
	ASPlayerController* PC = Cast<ASPlayerController>(Character->Controller);

	RestartPlayerDelayed(PC, MinRespawnDelay);

	ASPlayerState* DierState = Cast<ASPlayerState>(Character->GetPlayerState());
	ASPlayerState* InstigatorState = Cast<ASPlayerState>(InstigatedBy->PlayerState);

	if (DierState) 
	{
		DierState->AddDeath();
	}

	if (InstigatorState) 
	{
		InstigatorState->AddKill();
	}

	if (DierState && InstigatorState) 
	{
		UE_LOG(LogTemp, Log, TEXT("%s killed %s"), *DierState->GetPlayerName(), *InstigatorState->GetPlayerName());
	}

}
