// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectKCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffect.h"
#include "Abilities/ProjectKGameplayAbility.h"

//////////////////////////////////////////////////////////////////////////
// AProjectKCharacter

AProjectKCharacter::AProjectKCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	static ConstructorHelpers::FObjectFinder<UDataTable> DataTable(TEXT("/Game/Data/DT_Character"));

	if (DataTable.Succeeded())
	{
		CharacterDataTable = DataTable.Object;
	}

	// GAS

	AbilitySystemComponent = CreateDefaultSubobject<UProjectKAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	//AbilitySystemComponent->SetIsReplicated(true);

	// Create the attribute set, this replicates by default
	AttributeSet = CreateDefaultSubobject<UProjectKAttributeSet>(TEXT("AttributeSet"));

}

void AProjectKCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//// Try setting the inventory source, this will fail for AI
	//InventorySource = NewController;

	//if (InventorySource)
	//{
	//	InventoryUpdateHandle = InventorySource->GetSlottedItemChangedDelegate().AddUObject(this, &ARPGCharacterBase::OnItemSlotChanged);
	//	InventoryLoadedHandle = InventorySource->GetInventoryLoadedDelegate().AddUObject(this, &ARPGCharacterBase::RefreshSlottedGameplayAbilities);
	//}

	// Initialize our abilities
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AddStartupGameplayAbilities();
	}
}

void AProjectKCharacter::UnPossessed()
{
	//// Unmap from inventory source
	//if (InventorySource && InventoryUpdateHandle.IsValid())
	//{
	//	InventorySource->GetSlottedItemChangedDelegate().Remove(InventoryUpdateHandle);
	//	InventoryUpdateHandle.Reset();

	//	InventorySource->GetInventoryLoadedDelegate().Remove(InventoryLoadedHandle);
	//	InventoryLoadedHandle.Reset();
	//}

	//InventorySource = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProjectKCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProjectKCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProjectKCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProjectKCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProjectKCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AProjectKCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AProjectKCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AProjectKCharacter::OnResetVR);
}


void AProjectKCharacter::OnResetVR()
{
	// If ProjectK is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in ProjectK.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AProjectKCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AProjectKCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AProjectKCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProjectKCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProjectKCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AProjectKCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AProjectKCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AProjectKCharacter::OnDie_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Die Die Die"));
}

void AProjectKCharacter::Initialize()
{
	if (CharacterDataTable != nullptr)
	{
		FCharacterData* Row = CharacterDataTable->FindRow<FCharacterData>(Name, FString(""));

		if (Row != nullptr)
		{
			Health = Row->Health;
			Stamina = Row->Stamina;
			MaxHealth = Row->MaxHealth;
			MaxStamina = Row->MaxStamina;
		}
	}
}

// GAS

UAbilitySystemComponent* AProjectKCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AProjectKCharacter::AddStartupGameplayAbilities()
{
	check(AbilitySystemComponent);

	//if (GetLocalRole() == ROLE_Authority)
	{
		// Grant abilities, but only on the server	
		for (TSubclassOf<UProjectKGameplayAbility>& StartupAbility : GameplayAbilities)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartupAbility, 1, INDEX_NONE, this));
		}
	}
}

void AProjectKCharacter::HandleDamage(float DamageAmount, const FHitResult& HitInfo, const struct FGameplayTagContainer& DamageTags, AProjectKCharacter* InstigatorPawn, AActor* DamageCauser)
{
	OnDamaged(DamageAmount, HitInfo, DamageTags, InstigatorPawn, DamageCauser);
}

void AProjectKCharacter::HandleHealthChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	// We only call the BP callback if this is not the initial ability setup
	//if (bAbilitiesInitialized)
	{
		OnHealthChanged(DeltaValue, EventTags);
	}
}

void AProjectKCharacter::HandleStaminaChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	//if (bAbilitiesInitialized)
	{
		OnStaminaChanged(DeltaValue, EventTags);
	}
}

void AProjectKCharacter::HandleMoveSpeedChanged(float DeltaValue, const struct FGameplayTagContainer& EventTags)
{
	// Update the character movement's walk speed
	//GetCharacterMovement()->MaxWalkSpeed = GetMoveSpeed();

	//if (bAbilitiesInitialized)
	{
		OnMoveSpeedChanged(DeltaValue, EventTags);
	}
}

float AProjectKCharacter::GetHealth() const
{
	if (!AttributeSet)
		return 1.f;

	return AttributeSet->GetHealth();
}

float AProjectKCharacter::GetMaxHealth() const
{
	return AttributeSet->GetMaxHealth();
}

float AProjectKCharacter::GetStamina() const
{
	return AttributeSet->GetStamina();
}

float AProjectKCharacter::GetMaxStamina() const
{
	return AttributeSet->GetMaxStamina();
}

float AProjectKCharacter::GetMoveSpeed() const
{
	return AttributeSet->GetMoveSpeed();
}
