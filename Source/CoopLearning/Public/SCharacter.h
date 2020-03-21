// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class ASWeapon;
class USHealthComponent;
class USphereComponent;
class ASZipline;
class ASGranade;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeathSignature, class ASCharacter*, Character, class AController*, InstigatedBy, AActor*, DamageCauser);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnClosestWeaponChangeSignature, class ASCharacter*, Character, class ASWeapon*, OldClosestWeapon, class ASWeapon*, NewClosestWeapon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDealtDamageSignature, class ASCharacter*, Character, float , Amount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnReloadSignature, class ASCharacter*, Character, float, ReloadTime);


UENUM()
enum ECharacterState
{
	STATE_Normal,
	STATE_Reloading,
	STATE_Action,
	STATE_Zipline
};

UCLASS()
class COOPLEARNING_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void MoveCameraYaw(float Value);

	void MoveCameraPitch(float Value);

	float GetAimSlowdownMultiplyer();

	void BeginCrouch();

	void EndCrouch();

	void BeginJump();

	void BeginZoom();

	void EndZoom();

	void BeginReload();

	void TryPickup();

	void BeginDrop();

	void BeginInteract();

	void BeginMelee();

	void BeginGranade();

	void BeginSneak();

	void EndSneak();

	void EquipWeapon(ASWeapon* weapon);

	void TryUseZipline();

	void EndZiplineUse();

	ASWeapon* UnequipWeapon();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTryDrop();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTryPickup();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* CameraComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USHealthComponent* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* DetectionComp;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	ASWeapon* CurrentWeapon;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	ASWeapon* ClosestWeapon;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASWeapon> StarterWeaponClass;

	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName WeaponAttachSocketName;

	bool bWantsToZoom;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, Category = "Player", meta = (ClampMin = 0.1, ClampMax = 100))
	float ZoomInterpSpeed;

	float DefaultFOV;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	float AimProgress;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginZoom();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndZoom();

	UFUNCTION()
	void OnHeathChanged(USHealthComponent* SourceHealthComp, float Health, float HealthDelta, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bDied;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnDeathEffects();

	ASWeapon* GetClosestWeapon(FVector sourceLocation, TArray<AActor*> actors);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	TEnumAsByte<ECharacterState> State;

	TEnumAsByte<ECharacterState> PreviousState;

	FTimerHandle TimerHandle_StateSet;

	void SetStateToPrevious();

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	ASZipline* CurrentZipline;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool ZiplineDirectionIsForward;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float ZiplineSpeed;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerTryInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginMelee();

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	float DefaultMeleeDistance;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	float DefaultMeleeDamage;

	float MeleeDistance;

	float MeleeDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	TSubclassOf<UDamageType> MeleeDamageType;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	TSubclassOf<ASGranade> GranadeType;

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	int StartGranadeCount;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	int GranadeCount;

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginGranade();

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float GranadeThrowForce;

	virtual void PossessedBy(AController* NewController) override;

	UFUNCTION(Client, Reliable)
	void ClientNotifyClosestWeaponChange(ASWeapon* OldClosestWeapon, class ASWeapon* NewClosestWeapon);

	UPROPERTY(EditDefaultsOnly, Category = "Player")
	float SneakSpeedMultiplyer;

	bool InSneak;

	UFUNCTION(BlueprintCallable)
	float GetSneakSpeedMultiplyer();

	UFUNCTION(Exec)
	void Suicide();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FVector GetPawnViewLocation() const override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeathSignature OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnClosestWeaponChangeSignature OnClosestWeaponChange;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDealtDamageSignature OnDealDamage;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnReloadSignature OnReload;

	UFUNCTION(BlueprintCallable)
	void SetCharacterState(ECharacterState NewState, float Duration = -1);

	UFUNCTION(BlueprintCallable, Category = "Player")
		void BeginFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
		void StopFire();

	USHealthComponent* GetHealthComponent();

	void NotifyDamageDealt(float Amount);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastNotifyDamageDealt(float Amount);

	void SpawnWeapon();
};
