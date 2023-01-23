// Copyright Epic Games, Inc. All Rights Reserved.

#include "astarPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "astarCharacter.h"
#include "Engine/World.h"

AastarPlayerController::AastarPlayerController() : bLeftPressed(false), bRightPressed(false), bIsExistSBlock(false), bIsExistQBlock(false),
bCanCrossMove(false), bCanCornerCrossMove(false), bSpawnPath(false)
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	SBlockLoc = FLoc{0, 0};
	QBlockLoc = FLoc{ 0, 0 };
	SpawnTime = 0.f;
	
}

void AastarPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bLeftPressed)
	{
		SetBlock();
	}
	if (bRightPressed)
	{
		DestroyBlock();
	}
	if (bSpawnPath)
	{
		SpawnTime += DeltaTime;
		if (SpawnTime >= 0.05f)
		{
			SpawnPath();
			SpawnTime = 0.f;
		}
	}
	else
	{
		SpawnTime = 0.f;
	}
}

void AastarPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("LeftClick", IE_Pressed, this, &AastarPlayerController::LeftClickPressed);
	InputComponent->BindAction("LeftClick", IE_Released, this, &AastarPlayerController::LeftClickReleased);

	InputComponent->BindAction("RightClick", IE_Pressed, this, &AastarPlayerController::RightClickPressed);
	InputComponent->BindAction("RightClick", IE_Released, this, &AastarPlayerController::RightClickReleased);

	InputComponent->BindAction("SButton", IE_Pressed, this, &AastarPlayerController::SButtonPressed);
	InputComponent->BindAction("QButton", IE_Pressed, this, &AastarPlayerController::QButtonPressed);

	InputComponent->BindAction("PathFind", IE_Pressed, this, &AastarPlayerController::PathFind);
}

void AastarPlayerController::LeftClickPressed()
{
	bLeftPressed = true;
	
}

void AastarPlayerController::LeftClickReleased()
{
	bLeftPressed = false;
}

void AastarPlayerController::RightClickPressed()
{
	bRightPressed = true;
}

void AastarPlayerController::RightClickReleased()
{
	bRightPressed = false;
}

void AastarPlayerController::SetBlock()
{
	if (bSpawnPath) return;
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, true, Hit);

	if (Hit.bBlockingHit)
	{
		const FVector HitLocation = Hit.ImpactPoint;
		const float LocationX = HitLocation.X;
		const float LocationY = HitLocation.Y;
		int32 SpawnLocX = FMath::FloorToFloat((LocationX / 100)); SpawnLocX *= 100;  SpawnLocX += 50;
		int32 SpawnLocY = FMath::FloorToFloat((LocationY / 100)); SpawnLocY *= 100;  SpawnLocY += 50;
		
		if (IsExistBlock(SpawnLocX, SpawnLocY)) return;
			
		SpawnedBlockLoc.Add(FLoc{ SpawnLocX, SpawnLocY });
		if (Block)
		{
			FActorSpawnParameters ActorSpawnParameters;
			GetWorld()->SpawnActor<AActor>(Block, FVector(SpawnLocX, SpawnLocY, 50.f), FRotator::ZeroRotator, ActorSpawnParameters);
			InitPath();
		}
	}
}

void AastarPlayerController::DestroyBlock()
{
	FHitResult Hit;
	GetHitResultUnderCursor(ECC_Visibility, true, Hit);
	
	if (Hit.bBlockingHit)
	{
		const FVector HitLocation = Hit.ImpactPoint;
		const float LocationX = HitLocation.X;
		const float LocationY = HitLocation.Y;
		int32 SpawnLocX = FMath::FloorToFloat((LocationX / 100)); SpawnLocX *= 100;  SpawnLocX += 50;
		int32 SpawnLocY = FMath::FloorToFloat((LocationY / 100)); SpawnLocY *= 100;  SpawnLocY += 50;
		
		
		if (IsExistBlock(SpawnLocX, SpawnLocY))
		{
			SpawnedBlockLoc.Remove(FLoc{ SpawnLocX, SpawnLocY });
			Hit.GetActor()->Destroy();
			InitPath();
		}	
	}
}

void AastarPlayerController::SButtonPressed()
{
	if (bSpawnPath) return;
	if (bIsExistSBlock)
	{
		bIsExistSBlock = !bIsExistSBlock;
		if (StartBlockActor)
			StartBlockActor->Destroy();
		SBlockLoc = FLoc{ 0, 0 };
		InitPath();
		return;
	}
	else
	{
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, true, Hit);

		if (Hit.bBlockingHit)
		{
			const FVector HitLocation = Hit.ImpactPoint;
			const float LocationX = HitLocation.X;
			const float LocationY = HitLocation.Y;
			int32 SpawnLocX = FMath::FloorToFloat((LocationX / 100)); SpawnLocX *= 100;  SpawnLocX += 50;
			int32 SpawnLocY = FMath::FloorToFloat((LocationY / 100)); SpawnLocY *= 100;  SpawnLocY += 50;
			SBlockLoc = FLoc{ SpawnLocX, SpawnLocY };

			if (IsExistBlock(SpawnLocX, SpawnLocY) || (SBlockLoc == QBlockLoc)) return;

			if (StartBlock)
			{
				FActorSpawnParameters ActorSpawnParameters;
				StartBlockActor = GetWorld()->SpawnActor<AActor>(StartBlock, FVector(SpawnLocX, SpawnLocY, 50.f), FRotator::ZeroRotator, ActorSpawnParameters);
				bIsExistSBlock = true;
				InitPath();
			}
		}
	}
}

void AastarPlayerController::QButtonPressed()
{
	if (bSpawnPath) return;
	if (bIsExistQBlock)
	{
		bIsExistQBlock = !bIsExistQBlock;
		if (ExitBlockActor)
			ExitBlockActor->Destroy();
		QBlockLoc = FLoc{ 0, 0 };
		InitPath();
		return;
	}
	else
	{
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, true, Hit);

		if (Hit.bBlockingHit)
		{
			const FVector HitLocation = Hit.ImpactPoint;
			const float LocationX = HitLocation.X;
			const float LocationY = HitLocation.Y;
			int32 SpawnLocX = FMath::FloorToFloat((LocationX / 100)); SpawnLocX *= 100;  SpawnLocX += 50;
			int32 SpawnLocY = FMath::FloorToFloat((LocationY / 100)); SpawnLocY *= 100;  SpawnLocY += 50;

			QBlockLoc = FLoc{ SpawnLocX, SpawnLocY };

			if (IsExistBlock(SpawnLocX, SpawnLocY) || (SBlockLoc == QBlockLoc)) return;

			if (ExitBlock)
			{
				FActorSpawnParameters ActorSpawnParameters;
				ExitBlockActor = GetWorld()->SpawnActor<AActor>(ExitBlock, FVector(SpawnLocX, SpawnLocY, 50.f), FRotator::ZeroRotator, ActorSpawnParameters);
				bIsExistQBlock = true;
				InitPath();
			}
		}
	}
}

bool AastarPlayerController::IsExistBlock(int32 X, int32 Y)
{
	if (X == 50 || X == 2950 || Y == 50 || Y == 3450) return true;
	
	for (auto Loc : SpawnedBlockLoc)
	{
		if (Loc.X == X && Loc.Y == Y)
		{
			return true;
		}
	}
	return false;
}

void AastarPlayerController::PathFind()
{
	InitPath(); 
	TMap<FLoc, FLoc> Parent;
	if (AStar(Parent))//���ã��
		CalcPath(Parent);//��� �籸�� �� ��ġ
	else
		if(OnNotifyFail.IsBound())
			OnNotifyFail.Broadcast();

}

bool AastarPlayerController::AStar(OUT TMap<FLoc, FLoc>& Parent)
{
	if (!bIsExistSBlock || !bIsExistQBlock) return false;
	int DirCount = 0;
	if (bCanCrossMove)
		DirCount = 8;
	else
		DirCount = 4;
	

	FLoc Forward[] =
	{
		FLoc { 100, 0 }, //��
		FLoc { 0, -100 }, //����
		FLoc { -100, 0 }, //�Ʒ�
		FLoc { 0, 100 }, //������
		FLoc { 100, -100 }, //������
		FLoc { -100, -100 }, //���ʾƷ�
		FLoc { -100, 100 }, //�����ʾƷ�
		FLoc { 100, 100 }, //��������
	};

	int32 Cost[] =
	{
		100, //��
		100, //����
		100, //�Ʒ�
		100, //������
		140, //�밢
		140,
		140,
		140
	};

	TArray<TArray<bool>> Closed;
	TArray<bool> ClosedInner;
	ClosedInner.Init(false, 36);
	Closed.Init(ClosedInner, 36);

	TArray<TArray<int32>> Best;
	TArray<int32> BestInner;
	BestInner.Init(TNumericLimits<int32>::Max(), 36);
	Best.Init(BestInner, 36);

	TArray<LocInfo> OpenList;
	OpenList.Heapify();
	{
		int32 G = 0;
		int32 H = (FMath::Abs(QBlockLoc.Y - SBlockLoc.Y)) + 
			(FMath::Abs(QBlockLoc.X - SBlockLoc.X));
		
		OpenList.HeapPush(LocInfo{ G + H, G, SBlockLoc });
		Best[SBlockLoc.Y / 100][SBlockLoc.X / 100] = G + H;
		Parent.Add(FLoc{ SBlockLoc.X, SBlockLoc.Y }, FLoc{ SBlockLoc.X, SBlockLoc.Y });
	}
	
	while (!OpenList.IsEmpty())
	{
		LocInfo DiscLocInfo; //DiscoverLocInfo
		OpenList.HeapPop(DiscLocInfo); //�߰��� ��ġ �� F�� ���� �ͺ��� ������ �ȴ�.

		if (Closed[DiscLocInfo.Loc.Y / 100][DiscLocInfo.Loc.X / 100]) continue;
		if (Best[DiscLocInfo.Loc.Y / 100][DiscLocInfo.Loc.X / 100] < DiscLocInfo.F) continue;

		Closed[DiscLocInfo.Loc.Y / 100][DiscLocInfo.Loc.X / 100] = true; //�湮

		if (DiscLocInfo.Loc == QBlockLoc) return true;

		TArray<int32> PassDirs;

		for (int32 Dir = 0; Dir < DirCount; ++Dir) //�밢 �̵� ���(DirCount:8), �����(4)
		{
			FLoc Next = DiscLocInfo.Loc + Forward[Dir];
			if (IsExistBlock(Next.X, Next.Y)) continue; //������ ���⿡ ���� ������
			
			if (Dir >= 4)
			{
				if (!bCanCornerCrossMove)//�ڳ� �밢�̵� ����� �ƴϸ�(�ڳʿ��� ���ư��� �Ѵ�.)
				{
					if (!CanGoCornerCrossAway(Dir, PassDirs)) continue; //�ڳʿ� ���� ������
				}
				if (!CanGoCrossAway(Dir, PassDirs)) continue; //�밢 �̵��� �� �� ������ 
			}
			else
			{
				PassDirs.Add(Dir);
			}

			if (Closed[Next.Y / 100][Next.X / 100]) continue; //�̹� �湮�� ��
			
			int32 G = DiscLocInfo.G + Cost[Dir]; //��������� ��� + �� ���������� ��� = ���������� Next������ ���
			int32 H = (FMath::Abs(QBlockLoc.Y - Next.Y) + FMath::Abs(QBlockLoc.X - Next.X)); //����ư �Ÿ�
			//�湮�� ��ġ���� �� �� �ִ� ���� F�� ��, ���� ����� F�� ����Ǿ� �ִ� F���� ������ 
			if (Best[Next.Y / 100][Next.X / 100] > G + H) 
			{
				Best[Next.Y / 100][Next.X / 100] = G + H; //����
				OpenList.HeapPush(LocInfo{ G + H, G, Next });//OpenList�� �߰�
				Parent.Add(Next, DiscLocInfo.Loc); //��� ������ ����
			}
		}
	}
	return false;
}

bool AastarPlayerController::CanGoCrossAway(int32 Dir, TArray<int32> PassDirs)
{
	switch (Dir)
	{
	case 4:
		for (int32 Pass : PassDirs)
		{
			if (Pass == 0 || Pass == 1) //�� �� �ϳ� ������ �밢���� ���� ����
			{
				return true;
			}
		}
		break;
	case 5:
		for (int32 Pass : PassDirs)
		{
			if (Pass == 2 || Pass == 1)
			{
				return true;
			}
		}
		break;

	case 6:
		for (int32 Pass : PassDirs)
		{
			if (Pass == 2 || Pass == 3)
			{
				return true;
			}
		}
		break;
	case 7:
		for (int32 Pass : PassDirs)
		{
			if (Pass == 0 || Pass == 3)
			{
				return true;
			}
		}
		break;
	}
	return false;
}
	
bool AastarPlayerController::CanGoCornerCrossAway(int32 Dir, TArray<int32> PassDirs)
{
	int32* Value = nullptr;
	switch (Dir)
	{
	case 4:
		Value = PassDirs.FindByKey(0);
		if (!Value) return false;
		Value = PassDirs.FindByKey(1);
		if (!Value) return false;
		break;
	case 5:
		Value = PassDirs.FindByKey(1);
		if (!Value) return false;
		Value = PassDirs.FindByKey(2);
		if (!Value) return false;
		break;
	case 6:
		Value = PassDirs.FindByKey(2);
		if (!Value) return false;
		Value = PassDirs.FindByKey(3);
		if (!Value) return false;
		break;
	case 7:
		Value = PassDirs.FindByKey(0);
		if (!Value) return false;
		Value = PassDirs.FindByKey(3);
		if (!Value) return false;
		break;
	}
	return true;
}

void AastarPlayerController::CalcPath(TMap<FLoc, FLoc> Parent)
{
	FLoc Loc = QBlockLoc;

	Path.Empty();
	int32 PathIndex = 0;

	while (true)
	{
		Path.Push(Loc);

		if (Loc == Parent[Loc]) break;

		Loc = Parent[Loc];
	}
	Path.Pop(); //�������� ����

	bSpawnPath = true;
}

void AastarPlayerController::SpawnPath()
{
	FLoc SpawnLoc = Path.Pop();
	if (SpawnLoc == QBlockLoc) //���� ����
	{
		bSpawnPath = false;
		return;
	}
	FActorSpawnParameters ActorSpawnParameters;

	PathBlocks.Add(GetWorld()->SpawnActor<AActor>(PathBlock, FVector(SpawnLoc.X, SpawnLoc.Y, 50.f), FRotator::ZeroRotator, ActorSpawnParameters));
}

void AastarPlayerController::InitPath()
{
	while (!PathBlocks.IsEmpty())
	{
		PathBlocks.Pop()->Destroy();
	}
}
