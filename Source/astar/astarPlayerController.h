// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "UObject/NoExportTypes.h"
#include "astarPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNotifyFail);

USTRUCT(BlueprintType)
struct FLoc
{
	GENERATED_BODY()
	
	FLoc() :X(0), Y(0) {}
		
	FLoc(int32 XValue, int32 YValue) :X(XValue), Y(YValue) {}

	FLoc(const FLoc& Loc):FLoc(Loc.X, Loc.Y) {}

	bool operator ==(const FLoc& Other) const
	{
		return (Other.X == X) && (Other.Y == Y);
	}
	
	FLoc operator+(FLoc& Other)
	{
		FLoc Ret;
		Ret.Y = Y + Other.Y;
		Ret.X = X + Other.X;
		return Ret;
	}
	UPROPERTY()
	int32 X;

	UPROPERTY()
	int32 Y;
};

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FLoc& Loc);
#else
FORCEINLINE uint32 GetTypeHash(const FLoc& Loc)
{
	uint32 Hash = FCrc::MemCrc32(&Loc, sizeof(FLoc));
	return Hash;
}
#endif

struct LocInfo
{
	bool operator<(const LocInfo& Other) const { return F < Other.F; }
	bool operator>(const LocInfo& Other) const { return F > Other.F; }

	int32 F;
	int32 G;
	FLoc Loc;
};

UCLASS()
class AastarPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AastarPlayerController();
	UPROPERTY(BlueprintAssignable)
	FOnNotifyFail OnNotifyFail;
protected:
	uint32 bMoveToMouseCursor : 1;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
private:
	void LeftClickPressed();
	void LeftClickReleased();
	void RightClickPressed();
	void RightClickReleased();
	void SButtonPressed();
	void QButtonPressed();

	void SetBlock();
	void DestroyBlock();
	bool IsExistBlock(int32 X, int32 Y);
	void PathFind();
	bool AStar(TMap<FLoc, FLoc>& Parent);
	void CalcPath(TMap<FLoc, FLoc> Parent);
	UFUNCTION()
	void SpawnPath();
	UFUNCTION(BlueprintCallable)
	void InitPath();
	bool CanGoCrossAway(int32 Dir, TArray<int32> PassDirs);
	bool CanGoCornerCrossAway(int32 Dir, TArray<int32> PassDirs);

	bool bLeftPressed; 
	bool bRightPressed; 
	bool bIsExistSBlock;
	bool bIsExistQBlock;
	
	UPROPERTY(BluePrintReadWrite, Meta = (AllowPrivateAccess = "true"))
	bool bCanCrossMove;
	UPROPERTY(BluePrintReadWrite, Meta = (AllowPrivateAccess = "true"))
	bool bCanCornerCrossMove;
	bool bSpawnPath;
	float SpawnTime;
	FLoc SBlockLoc;
	FLoc QBlockLoc;
	TArray<FLoc> Path;
	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> Block;
	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> StartBlock;
	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> ExitBlock;
	UPROPERTY(EditAnywhere, Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> PathBlock;
	UPROPERTY()
	AActor* StartBlockActor;
	UPROPERTY()
	AActor* ExitBlockActor;
	UPROPERTY(VisibleAnywhere, Meta = (AllowPrivateAccess = "true"))
	TArray<FLoc> SpawnedBlockLoc;
	TArray<AActor*> PathBlocks;
};
