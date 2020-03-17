// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Components/SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "CoopLearning.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/GameModeBase.h"
#include "SPlayerController.h"
#include "TimerManager.h"
#include "GameFramework/PlayerState.h"
#include "SZipline.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SGameInstance.h"
#include "SUserSaveGame.h"
#include "SGranade.h"
#include "SPlayerController.h"

// Sets default values
ASCharacter::ASCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	DetectionComp = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionComp"));
	DetectionComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	ZoomedFOV = 65;
	ZoomInterpSpeed = 40;

	WeaponAttachSocketName = "WeaponSocket";
	
	DefaultMeleeDistance = 100;

	DefaultMeleeDamage = 50;

	State = STATE_Normal;
}

void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	GranadeCount = StartGranadeCount;

	DefaultFOV = CameraComp->FieldOfView;

	if (Role == ROLE_Authority)
	{
		MeleeDistance = DefaultMeleeDistance;
		MeleeDamage = DefaultMeleeDamage;

		HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHeathChanged);

	}
}

void ASCharacter::MoveForward(float Value)
{
	if (State == STATE_Normal || State == STATE_Reloading || State == STATE_Action)
	{
		AddMovementInput(GetActorForwardVector() * Value * GetAimSlowdownMultiplyer());
	}
}

void ASCharacter::MoveRight(float Value)
{
	if (State == STATE_Normal || State == STATE_Reloading || State == STATE_Action)
	{
		AddMovementInput(GetActorRightVector() * Value * GetAimSlowdownMultiplyer());
	}
}

void ASCharacter::MoveCameraYaw(float Value)
{
	USGameInstance* GameInstance = Cast<USGameInstance>(GetGameInstance());
	float Sensitivity = 1;

	if (GameInstance)
	{
		USUserSaveGame* UserSaveGame = GameInstance->GetUserSaveGame();

		if (UserSaveGame)
		{
			Sensitivity = UserSaveGame->UserMouseSensitivity;
		}
	}

	AddControllerYawInput(Value * Sensitivity);
}

void ASCharacter::MoveCameraPitch(float Value)
{
	USGameInstance* GameInstance = Cast<USGameInstance>(GetGameInstance());
	float Sensitivity = 1;

	if (GameInstance)
	{
		USUserSaveGame* UserSaveGame = GameInstance->GetUserSaveGame();

		if (UserSaveGame)
		{
			Sensitivity = UserSaveGame->UserMouseSensitivity;
		}
	}

	AddControllerPitchInput(Value * Sensitivity);
}

float ASCharacter::GetAimSlowdownMultiplyer()
{
	return  1 - (AimProgress * 0.5);
}

void ASCharacter::BeginCrouch()
{
	if (State == STATE_Normal || State == STATE_Reloading)
	{
		Crouch();
	}

}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginJump()
{
	if (State == STATE_Normal || State == STATE_Reloading)
	{
		JumpKeyHoldTime = 0.1;
		Jump();
	}
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;

	if (Role < ROLE_Authority)
	{
		ServerBeginZoom();
	}
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;

	if (Role < ROLE_Authority)
	{
		ServerEndZoom();
	}
}

void ASCharacter::ServerBeginZoom_Implementation()
{
	bWantsToZoom = true;
}

bool ASCharacter::ServerBeginZoom_Validate()
{
	return true;
}

void ASCharacter::ServerEndZoom_Implementation()
{
	bWantsToZoom = false;
}

bool ASCharacter::ServerEndZoom_Validate()
{
	return true;
}

void ASCharacter::BeginFire()
{
	if (CurrentWeapon && (State == STATE_Normal || State == STATE_Zipline))
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

USHealthComponent* ASCharacter::GetHealthComponent()
{
	return HealthComp;
}

void ASCharacter::NotifyDamageDealt(float Amount)
{
	MulticastNotifyDamageDealt(Amount);
}

void ASCharacter::SpawnWeapon()
{
	TSubclassOf<ASWeapon> WeaponClass = StarterWeaponClass;

	ASPlayerController* PC = Cast<ASPlayerController>(GetController());

	if (PC)
	{
		WeaponClass = PC->GetRespawnWeapon();
		UE_LOG(LogTemp, Log, TEXT("Setting PC Respawn Weapon"));
	}

	UE_LOG(LogTemp, Log, TEXT("Respawning with Weapon: %s"), *WeaponClass->GetName());

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASWeapon* NewWeapon = GetWorld()->SpawnActor<ASWeapon>(WeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	EquipWeapon(NewWeapon);
}

void ASCharacter::MulticastNotifyDamageDealt_Implementation(float Amount)
{
	OnDealDamage.Broadcast(this, Amount);
}

void ASCharacter::BeginReload()
{
	if (CurrentWeapon && (State == STATE_Normal || State == STATE_Zipline))
	{
		CurrentWeapon->Reload();
		StopFire();
		float ReloadTime = CurrentWeapon->GetReloadTime();
		SetCharacterState(STATE_Reloading, ReloadTime);
		OnReload.Broadcast(this, ReloadTime);
	}
}

void ASCharacter::TryPickup()
{
	//Check on the client first if there is an overlapping weapon
	//will be checked again on server if this is the client

	if (CurrentWeapon) 
	{
		return;
	}

	if (ClosestWeapon)
	{
		if (Role >= ROLE_Authority) 
		{
			EquipWeapon(ClosestWeapon);
		}
		else 
		{
			ServerTryPickup();
		}
	}
}

void ASCharacter::ServerTryPickup_Implementation()
{
	if (ClosestWeapon)
	{
		EquipWeapon(ClosestWeapon);
	}
}

bool ASCharacter::ServerTryPickup_Validate()
{
	return true;
}

void ASCharacter::BeginDrop()
{
	if (CurrentWeapon)
	{
		ServerTryDrop();
		StopFire();
	}
}

void ASCharacter::BeginInteract()
{
	if (Role < ROLE_Authority) 
	{
		ServerTryInteract();
		return;
	}

	if (State == STATE_Zipline) 
	{
		EndZiplineUse();
	}
	else 
	{
		TryPickup();
		TryUseZipline();
	}

}

void ASCharacter::BeginMelee()
{
	if (Role < ROLE_Authority) 
	{
		ServerBeginMelee();
		return;
	}

	if (State == STATE_Normal) 
	{
		UE_LOG(LogTemp, Log, TEXT("Begin Melee"));
		SetCharacterState(STATE_Action, 1.0f);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		FHitResult Hit;

		FVector TraceStart;
		FRotator EyeRotation;
		GetActorEyesViewPoint(TraceStart, EyeRotation);

		FVector HandPosition = GetMesh()->GetSocketLocation(WeaponAttachSocketName);
		FVector EndPosition = HandPosition + EyeRotation.Vector() * MeleeDistance;

		if (GetWorld()->LineTraceSingleByChannel(Hit, HandPosition, EndPosition, COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();
			UE_LOG(LogTemp, Log, TEXT("Melee Hit %s"), *HitActor->GetName());
			UGameplayStatics::ApplyPointDamage(HitActor, MeleeDamage, EyeRotation.Vector(), Hit, GetInstigatorController(), this, MeleeDamageType);
		}
	}

}

void ASCharacter::BeginGranade()
{
	if (State == STATE_Normal && GranadeCount > 0) 
	{
		ServerBeginGranade();
	}

}

void ASCharacter::ServerTryDrop_Implementation()
{
	UnequipWeapon();
}

bool ASCharacter::ServerTryDrop_Validate()
{
	return true;
}

void ASCharacter::EquipWeapon(ASWeapon * NewWeapon)
{
	if (NewWeapon && Role >= ROLE_Authority)
	{
		if (CurrentWeapon)
		{
			UnequipWeapon();
		}

		CurrentWeapon = NewWeapon;
		NewWeapon->GetEquippedBy(this);
		NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);	
	}
}

void ASCharacter::TryUseZipline()
{
	TArray<AActor*> ZiplinesInArea;
	GetOverlappingActors(ZiplinesInArea, TSubclassOf<ASZipline>());

	if (ZiplinesInArea.Num() > 0) 
	{
		CurrentZipline = Cast<ASZipline>(ZiplinesInArea[0]);

		if (CurrentZipline) 
		{

			FVector EyesLocation;
			FRotator EyesRotation;

			GetActorEyesViewPoint(EyesLocation, EyesRotation);
			
			ZiplineDirectionIsForward = CurrentZipline->GetDirectionIsForward(EyesRotation.Vector());
			UE_LOG(LogTemp, Log, TEXT("New Direction is %s"), (ZiplineDirectionIsForward ? TEXT("True") : TEXT("False")))
			SetCharacterState(STATE_Zipline);

			Cast<UCharacterMovementComponent>(GetMovementComponent())->SetMovementMode(EMovementMode::MOVE_Flying);		
		}
	}
}

void ASCharacter::EndZiplineUse()
{
	Cast<UCharacterMovementComponent>(GetMovementComponent())->SetMovementMode(EMovementMode::MOVE_Walking);
	SetCharacterState(STATE_Normal);
}

ASWeapon* ASCharacter::UnequipWeapon()
{
	if (CurrentWeapon && Role >= ROLE_Authority) 
	{
		StopFire();

		ASWeapon* Weapon = CurrentWeapon;
		Weapon->Unequip();
		CurrentWeapon = nullptr;
		return Weapon;
	}

	return nullptr;
}

void ASCharacter::OnHeathChanged(USHealthComponent * SourceHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0.0f && !bDied && Role >= ROLE_Authority)
	{
		//Deaths
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		BeginDrop();
		GetMesh()->SetSimulatePhysics(true);
		MulticastOnDeathEffects();
		UE_LOG(LogTemp, Log, TEXT("Calling OnDeath"));
		OnDeath.Broadcast(this, InstigatedBy, DamageCauser);

		ASPlayerController* PC = Cast<ASPlayerController>(Controller);
		DetachFromControllerPendingDestroy();

		if (PC)
		{
			PC->BlendToController(InstigatedBy, 1);
		}

		SetLifeSpan(10.0f);
	}

}

void ASCharacter::MulticastOnDeathEffects_Implementation()
{
	bDied = true;
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->SetSimulatePhysics(true);
}

ASWeapon* ASCharacter::GetClosestWeapon(FVector sourceLocation, TArray<AActor*> actors)
{
	if (actors.Num() <= 0)
	{
		return nullptr;
	}

	ASWeapon* closestActor = nullptr;
	float currentClosestDistance = TNumericLimits<float>::Max();

	for (int i = 0; i < actors.Num(); i++)
	{
		ASWeapon* WeaponCast = Cast<ASWeapon>(actors[i]);
		if (WeaponCast) 
		{
			if (WeaponCast == CurrentWeapon) 
			{
				continue;
			}

			float distance = FVector::DistSquared(sourceLocation, actors[i]->GetActorLocation());
			if (distance < currentClosestDistance)
			{
				currentClosestDistance = distance;
				closestActor = WeaponCast;
			}
		}
	}

	return closestActor;
}

void ASCharacter::ServerTryInteract_Implementation()
{
	BeginInteract();
}

bool ASCharacter::ServerTryInteract_Validate()
{
	return true;
}

void ASCharacter::ServerBeginMelee_Implementation()
{
	BeginMelee();
}

bool ASCharacter::ServerBeginMelee_Validate()
{
	return true;
}

void ASCharacter::ServerBeginGranade_Implementation()
{
	if (State == STATE_Normal && GranadeCount > 0) 
	{
		GranadeCount -= 1;
		SetCharacterState(STATE_Action, 1.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector EyeLocation;
		FRotator EyeRotator;
		GetActorEyesViewPoint(EyeLocation, EyeRotator);

		//Spawn Granade a meter in front
		ASGranade* Granade = GetWorld()->SpawnActor<ASGranade>(GranadeType, GetActorLocation() + EyeRotator.Vector() * 100, FRotator::ZeroRotator, SpawnParams);
		Granade->SetOwner(this);

		FVector Impulse = EyeRotator.Vector() * GranadeThrowForce;

		Granade->GetMeshComp()->AddImpulse(Impulse, NAME_None, true);	
	}
}

bool ASCharacter::ServerBeginGranade_Validate()
{
	return GranadeCount > 0;
}

void ASCharacter::PossessedBy(AController * NewController)
{
	Super::PossessedBy(NewController);
	SpawnWeapon();
}

void ASCharacter::ClientNotifyClosestWeaponChange_Implementation(ASWeapon * OldClosestWeapon, ASWeapon * NewClosestWeapon)
{
	ClosestWeapon = NewClosestWeapon;

	OnClosestWeaponChange.Broadcast(this, OldClosestWeapon, NewClosestWeapon);
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role >= ROLE_AutonomousProxy) 
	{
		float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;


		float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

		CameraComp->SetFieldOfView(NewFOV);

		AimProgress = 1 - ((NewFOV - ZoomedFOV) / (DefaultFOV - ZoomedFOV));
	}

	if (Role >= ROLE_Authority) 
	{
		TArray<AActor*> ActorsInArea;
		GetOverlappingActors(ActorsInArea, TSubclassOf<AActor>());

		ASWeapon* NewClosestWeapon = GetClosestWeapon(GetActorLocation(), ActorsInArea);

		if (NewClosestWeapon != ClosestWeapon)
		{
			ASWeapon* OldWeapon = ClosestWeapon;
			ClosestWeapon = NewClosestWeapon;
			ClientNotifyClosestWeaponChange(OldWeapon, ClosestWeapon);
		}
	}


	if (Role >= ROLE_AutonomousProxy && State == STATE_Zipline)
	{
		if (CurrentZipline)
		{
			FVector Dir = CurrentZipline->GetDirection(ZiplineDirectionIsForward);
			//AddMovementInput(Dir, ZiplineSpeed, true);
			GetMovementComponent()->Velocity = Dir * ZiplineSpeed;

			if (Role >= ROLE_Authority) 
			{
				if (CurrentZipline->DestinationReached(GetActorLocation(), ZiplineDirectionIsForward,200))
				{
					UE_LOG(LogTemp, Log, TEXT("Ending Zipline because Target reached"));
					EndZiplineUse();
				}
			}
		}
		else
		{
			if (Role >= ROLE_Authority)
			{
				UE_LOG(LogTemp, Log, TEXT("Ending Zipline because no Zipline is assigned"));
				EndZiplineUse();
			}
		}
	}



}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::MoveCameraPitch);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::MoveCameraYaw);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::BeginJump);


	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &ASCharacter::BeginDrop);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::BeginReload);

	PlayerInputComponent->BindAction("Interaction", IE_Pressed, this, &ASCharacter::BeginInteract);

	PlayerInputComponent->BindAction("Melee", IE_Pressed, this, &ASCharacter::BeginMelee);

	PlayerInputComponent->BindAction("Granade", IE_Pressed, this, &ASCharacter::BeginGranade);

}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp) 
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::SetCharacterState(ECharacterState NewState, float Duration)
{
	PreviousState = State;
	State = NewState;

	//UE_LOG(LogTemp, Log, TEXT( "%s changed state to %s"), *GetPlayerState()->GetPlayerName(), *GETENUMSTRING("ECharacterState", State));

	if (Duration > 0)
	{
		GetWorldTimerManager().SetTimer(TimerHandle_StateSet, this, &ASCharacter::SetStateToPrevious, Duration, false);
	}
	else 
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_StateSet);
	}

}

void ASCharacter::SetStateToPrevious()
{
	SetCharacterState(PreviousState);
}


void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
	DOREPLIFETIME(ASCharacter, AimProgress);
	DOREPLIFETIME(ASCharacter, State);
	DOREPLIFETIME(ASCharacter, CurrentZipline);
	DOREPLIFETIME(ASCharacter, ZiplineDirectionIsForward);
	DOREPLIFETIME(ASCharacter, GranadeCount);
}