📋 전체 흐름도

  게임 시작
     ↓
  Steam 로그인 확인
     ↓
  Steam ID 추출
     ↓
  서버에 HTTP 요청 (Steam ID로 닉네임 조회)
     ↓
     ├─→ [닉네임 존재] → PlayerState에 설정 → 게임 진입
     └─→ [닉네임 없음] → 닉네임 입력 UI 표시
                           ↓
                      서버에 등록 요청
                           ↓
                      PlayerState에 설정 → 게임 진입

  ---
  🔧 1단계: Steam ID 추출

  필요한 지식

  - OnlineSubsystem Steam: UE의 Steam 통합 인터페이스
  - UniqueNetId: 플랫폼별 고유 ID를 추상화한 클래스
  - IOnlineIdentity: 사용자 인증 정보 관리 인터페이스

  사용할 함수들

  // PlayerController 또는 GameInstance에서 구현
  void AYourPlayerController::GetSteamID()
  {
      // 1. OnlineSubsystem 가져오기
      IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
      if (!OnlineSubsystem)
      {
          UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem not found"));
          return;
      }

      // 2. Identity Interface 가져오기
      IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
      if (!IdentityInterface.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("IdentityInterface not valid"));
          return;
      }

      // 3. 로컬 유저 번호 (보통 0)
      int32 LocalUserNum = GetLocalPlayer()->GetControllerId();

      // 4. Steam ID 가져오기
      FUniqueNetIdPtr UniqueNetId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
      if (!UniqueNetId.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("UniqueNetId not valid"));
          return;
      }

      // 5. String으로 변환 (Steam ID 64비트 숫자)
      FString SteamIDString = UniqueNetId->ToString();
      UE_LOG(LogTemp, Log, TEXT("Steam ID: %s"), *SteamIDString);

      // 6. 서버에 조회 요청
      CheckNicknameOnServer(SteamIDString);
  }

  핵심 API

  | 인터페이스            | 메서드                      | 반환값               | 설명                    |
  |------------------|--------------------------|-------------------|-----------------------|
  | IOnlineSubsystem | Get()                    | IOnlineSubsystem* | Steam OSS 인스턴스        |
  | IOnlineIdentity  | GetUniquePlayerId(int32) | FUniqueNetIdPtr   | Steam ID              |
  | FUniqueNetId     | ToString()               | FString           | "765611XXXXXXXXXX" 형식 |
  | FUniqueNetId     | GetBytes()               | TArray<uint8>     | 바이너리 형태 (선택)          |

  ---
  🌐 2단계: 서버 조회 (HTTP 요청)

  필요한 지식

  - HTTP Module: UE의 HTTP 통신 모듈 (이미 사용 중)
  - JSON 파싱: 서버 응답 처리
  - 델리게이트: 비동기 콜백 처리

  사용할 함수들

  void AYourPlayerController::CheckNicknameOnServer(const FString& SteamID)
  {
      FHttpModule& HttpModule = FHttpModule::Get();
      TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

      // GET 방식 예시
      FString URL = FString::Printf(
          TEXT("http://100.115.252.51:8000/api/v1/user/nickname?steam_id=%s"),
          *SteamID
      );

      Request->SetURL(URL);
      Request->SetVerb(TEXT("GET"));
      Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

      // 콜백 바인딩
      Request->OnProcessRequestComplete().BindUObject(
          this,
          &AYourPlayerController::OnNicknameCheckComplete
      );

      Request->ProcessRequest();
  }

  void AYourPlayerController::OnNicknameCheckComplete(
      FHttpRequestPtr Request,
      FHttpResponsePtr Response,
      bool bWasSuccessful)
  {
      if (!bWasSuccessful || !Response.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));
          ShowNicknameInputUI(); // 실패 시 새 닉네임 입력
          return;
      }

      int32 ResponseCode = Response->GetResponseCode();
      FString ResponseContent = Response->GetContentAsString();

      if (ResponseCode == 200) // 닉네임 존재
      {
          // JSON 파싱
          TSharedPtr<FJsonObject> JsonObject;
          TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

          if (FJsonSerializer::Deserialize(Reader, JsonObject))
          {
              FString Nickname = JsonObject->GetStringField(TEXT("nickname"));
              SetPlayerNickname(Nickname);
          }
      }
      else if (ResponseCode == 404) // 닉네임 없음
      {
          ShowNicknameInputUI();
      }
  }

  서버 API 예상 스펙

  1. 닉네임 조회

  GET /api/v1/user/nickname?steam_id=76561198XXXXXXXX

  응답 (200 OK):
  {
      "steam_id": "76561198XXXXXXXX",
      "nickname": "PlayerName",
      "created_at": "2025-12-01T10:30:00Z"
  }

  응답 (404 Not Found):
  {
      "error": "Nickname not found"
  }

  2. 닉네임 등록

  POST /api/v1/user/nickname
  Content-Type: application/json

  {
      "steam_id": "76561198XXXXXXXX",
      "nickname": "NewPlayerName"
  }

  응답 (201 Created):
  {
      "steam_id": "76561198XXXXXXXX",
      "nickname": "NewPlayerName",
      "created_at": "2025-12-04T14:20:00Z"
  }

  ---
  👤 3단계: PlayerState에 닉네임 설정

  필요한 지식

  - PlayerState: 네트워크 복제되는 플레이어 정보 클래스
  - Replication: 멀티플레이어 동기화
  - Server RPC: 클라이언트→서버 함수 호출

  사용할 함수들

  // PlayerController에서 호출
  void AYourPlayerController::SetPlayerNickname(const FString& Nickname)
  {
      APlayerState* PS = GetPlayerState<APlayerState>();
      if (!PS)
      {
          UE_LOG(LogTemp, Error, TEXT("PlayerState is null"));
          return;
      }

      // PlayerState의 SetPlayerName은 자동으로 복제됨
      PS->SetPlayerName(Nickname);

      UE_LOG(LogTemp, Log, TEXT("Nickname set to: %s"), *Nickname);

      // 게임 진입 허용
      OnNicknameSetupComplete();
  }

  void AYourPlayerController::OnNicknameSetupComplete()
  {
      // UI 닫기
      if (NicknameInputWidget)
      {
          NicknameInputWidget->RemoveFromParent();
      }

      // 입력 모드 게임으로 전환
      FInputModeGameOnly InputMode;
      SetInputMode(InputMode);
      SetShowMouseCursor(false);

      // 게임 로직 시작 (예: 스폰 포인트 이동 등)
      ServerRPC_NotifyNicknameReady();
  }

  // 서버에 닉네임 설정 완료 알림
  void AYourPlayerController::ServerRPC_NotifyNicknameReady_Implementation()
  {
      // 서버 측 로직 (필요 시)
      UE_LOG(LogTemp, Log, TEXT("Player %s is ready"), *GetPlayerState<APlayerState>()->GetPlayerName());
  }

  PlayerState 기본 함수

  | 함수                     | 설명             |
  |------------------------|----------------|
  | SetPlayerName(FString) | 닉네임 설정 (자동 복제) |
  | GetPlayerName()        | 현재 닉네임 반환      |
  | GetUniqueId()          | UniqueNetId 반환 |

  ---
  🎨 4단계: 닉네임 입력 UI

  필요한 지식

  - UMG Widget: UI 시스템
  - EditableTextBox: 텍스트 입력 위젯
  - Button 이벤트: 클릭 핸들링

  Widget Blueprint 구조

  Canvas Panel
  ├─ Border (배경)
  │   └─ Vertical Box
  │       ├─ Text Block ("닉네임을 입력하세요")
  │       ├─ Editable Text Box (입력 필드)
  │       └─ Button ("확인")

  C++ Widget 클래스

  // Rene_NicknameInput_Widget.h
  UCLASS()
  class UE_RENE_API URene_NicknameInput_Widget : public UUserWidget
  {
      GENERATED_BODY()

  protected:
      virtual void NativeConstruct() override;

  private:
      UPROPERTY(meta=(BindWidget))
      TObjectPtr<UEditableTextBox> TextBox_Nickname;

      UPROPERTY(meta=(BindWidget))
      TObjectPtr<UButton> Btn_Confirm;

      UFUNCTION()
      void OnConfirmClicked();

      UFUNCTION()
      void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
  };

  // Rene_NicknameInput_Widget.cpp
  void URene_NicknameInput_Widget::NativeConstruct()
  {
      Super::NativeConstruct();

      if (Btn_Confirm)
      {
          Btn_Confirm->OnClicked.AddDynamic(this, &URene_NicknameInput_Widget::OnConfirmClicked);
      }

      if (TextBox_Nickname)
      {
          TextBox_Nickname->OnTextCommitted.AddDynamic(this, &URene_NicknameInput_Widget::OnTextCommitted);
      }
  }

  void URene_NicknameInput_Widget::OnConfirmClicked()
  {
      if (!TextBox_Nickname) return;

      FString Nickname = TextBox_Nickname->GetText().ToString().TrimStartAndEnd();

      // 유효성 검증
      if (Nickname.IsEmpty())
      {
          UE_LOG(LogTemp, Warning, TEXT("Nickname is empty"));
          return;
      }

      if (Nickname.Len() > 20)
      {
          UE_LOG(LogTemp, Warning, TEXT("Nickname too long"));
          return;
      }

      // PlayerController에 알림
      APlayerController* PC = GetOwningPlayer();
      if (AYourPlayerController* YourPC = Cast<AYourPlayerController>(PC))
      {
          YourPC->RegisterNicknameToServer(Nickname);
      }
  }

  void URene_NicknameInput_Widget::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
  {
      if (CommitMethod == ETextCommit::OnEnter)
      {
          OnConfirmClicked();
      }
  }

  ---
  🔄 5단계: 닉네임 서버 등록

  사용할 함수들

  void AYourPlayerController::RegisterNicknameToServer(const FString& Nickname)
  {
      // Steam ID 가져오기
      IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
      if (!OnlineSubsystem) return;

      IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
      if (!IdentityInterface.IsValid()) return;

      FUniqueNetIdPtr UniqueNetId = IdentityInterface->GetUniquePlayerId(0);
      if (!UniqueNetId.IsValid()) return;

      FString SteamID = UniqueNetId->ToString();

      // HTTP POST 요청
      FHttpModule& HttpModule = FHttpModule::Get();
      TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

      Request->SetURL(TEXT("http://100.115.252.51:8000/api/v1/user/nickname"));
      Request->SetVerb(TEXT("POST"));
      Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

      // JSON 페이로드 생성
      TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
      JsonObject->SetStringField(TEXT("steam_id"), SteamID);
      JsonObject->SetStringField(TEXT("nickname"), Nickname);

      FString OutputString;
      TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
      FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

      Request->SetContentAsString(OutputString);

      Request->OnProcessRequestComplete().BindUObject(
          this,
          &AYourPlayerController::OnNicknameRegisterComplete
      );

      Request->ProcessRequest();
  }

  void AYourPlayerController::OnNicknameRegisterComplete(
      FHttpRequestPtr Request,
      FHttpResponsePtr Response,
      bool bWasSuccessful)
  {
      if (!bWasSuccessful || !Response.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("Nickname registration failed"));
          // 에러 UI 표시
          return;
      }

      if (Response->GetResponseCode() == 201) // Created
      {
          FString ResponseContent = Response->GetContentAsString();
          TSharedPtr<FJsonObject> JsonObject;
          TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

          if (FJsonSerializer::Deserialize(Reader, JsonObject))
          {
              FString Nickname = JsonObject->GetStringField(TEXT("nickname"));
              SetPlayerNickname(Nickname);
          }
      }
      else
      {
          UE_LOG(LogTemp, Error, TEXT("Server returned code: %d"), Response->GetResponseCode());
      }
  }

  ---
  📦 6단계: 모듈 의존성 추가

  UE_ReNe.Build.cs 수정

  PublicDependencyModuleNames.AddRange(new string[]
  {
      "Core",
      "CoreUObject",
      "Engine",
      "InputCore",
      "SlateCore",
      "OnlineSubsystem",        // 이미 있음
      "OnlineSubsystemSteam",   // 이미 있음
      "OnlineSubsystemUtils",   // 추가 필요 (헬퍼 유틸리티)
      "Voice",
      "HTTP",
      "Json",                   // JSON 파싱
      "JsonUtilities",          // JSON 직렬화
  });

  ---
  🎯 전체 구현 순서

  1. GameInstance에서 초기화 (권장)

  // Rene_GameInstance.h
  UCLASS()
  class URene_GameInstance : public UGameInstance
  {
      GENERATED_BODY()

  public:
      virtual void Init() override;
      void CheckSteamLogin();

  private:
      void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString&
  Error);
  };

  // Rene_GameInstance.cpp
  void URene_GameInstance::Init()
  {
      Super::Init();
      CheckSteamLogin();
  }

  void URene_GameInstance::CheckSteamLogin()
  {
      IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
      if (!OnlineSubsystem) return;

      IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
      if (!IdentityInterface.IsValid()) return;

      // Steam은 자동 로그인되지만 명시적 확인
      if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
      {
          FUniqueNetIdPtr UniqueNetId = IdentityInterface->GetUniquePlayerId(0);
          UE_LOG(LogTemp, Log, TEXT("Steam Auto-Login Success: %s"), *UniqueNetId->ToString());
      }
      else
      {
          // Steam 로그인 시도
          IdentityInterface->OnLoginCompleteDelegates->AddUObject(this, &URene_GameInstance::OnLoginComplete);
          IdentityInterface->Login(0, FOnlineAccountCredentials());
      }
  }

  void URene_GameInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
  const FString& Error)
  {
      if (bWasSuccessful)
      {
          UE_LOG(LogTemp, Log, TEXT("Steam Login Success: %s"), *UserId.ToString());
      }
      else
      {
          UE_LOG(LogTemp, Error, TEXT("Steam Login Failed: %s"), *Error);
      }
  }

  2. PlayerController에서 닉네임 체크

  void ARene_PlayerController::BeginPlay()
  {
      Super::BeginPlay();

      if (IsLocalController())
      {
          // 약간의 지연 후 체크 (PlayerState 초기화 대기)
          FTimerHandle TimerHandle;
          GetWorld()->GetTimerManager().SetTimer(
              TimerHandle,
              this,
              &ARene_PlayerController::GetSteamID,
              0.5f,
              false
          );
      }
  }

  ---
  📚 필요한 사전 지식 요약

  1. OnlineSubsystem 기초

  - Steam OSS 설정 (DefaultEngine.ini)
  - Identity Interface 개념
  - UniqueNetId 구조

  2. HTTP 통신

  - 비동기 요청/응답 패턴
  - 델리게이트 바인딩
  - JSON 직렬화/역직렬화

  3. UMG & Widget

  - Widget Blueprint 바인딩
  - 동적 위젯 생성/제거
  - 입력 모드 전환

  4. 네트워크 기초

  - PlayerState 복제
  - Server RPC
  - 클라이언트/서버 구분 (IsLocalController())

  ---
  🛠️ 구현 체크리스트

  - UE_ReNe.Build.cs에 Json, JsonUtilities 모듈 추가
  - Steam ID 추출 함수 구현
  - 서버 닉네임 조회 HTTP 요청 구현
  - 서버 닉네임 등록 HTTP 요청 구현
  - 닉네임 입력 Widget 제작
  - PlayerState 닉네임 설정 함수 구현
  - GameInstance 또는 PlayerController에 초기화 로직 추가
  - 에러 핸들링 (네트워크 실패, 중복 닉네임 등)
  - 서버 API 엔드포인트 확정 및 테스트

  ---
  💡 추가 고려사항

  1. 중복 닉네임 처리: 서버에서 409 Conflict 반환 시 재입력 요청
  2. 닉네임 검증: 욕설 필터, 길이 제한, 특수문자 제한
  3. 오프라인 대응: HTTP 요청 실패 시 Steam 닉네임을 임시 사용
  4. 캐싱: 한 번 조회한 닉네임을 GameInstance에 저장
  5. Steam 닉네임 기본값: 서버에 없을 경우 Steam 프로필 이름을 제안

  // Steam 프로필 닉네임 가져오기
  IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
  FString SteamNickname = IdentityInterface->GetPlayerNickname(0);

  이 정보들을 바탕으로 구현하시면 됩니다! 추가 질문 있으시면 말씀해주세요.

> 마지막에 줬던 답변 다시 줘

● Steam 기반 닉네임 시스템 구현 가이드

  📋 전체 흐름도

  게임 시작
     ↓
  Steam 로그인 확인
     ↓
  Steam ID 추출
     ↓
  서버에 HTTP 요청 (Steam ID로 닉네임 조회)
     ↓
     ├─→ [닉네임 존재] → PlayerState에 설정 → 게임 진입
     └─→ [닉네임 없음] → 닉네임 입력 UI 표시
                           ↓
                      서버에 등록 요청
                           ↓
                      PlayerState에 설정 → 게임 진입

  ---
  🔧 1단계: Steam ID 추출

  필요한 지식

  - OnlineSubsystem Steam: UE의 Steam 통합 인터페이스
  - UniqueNetId: 플랫폼별 고유 ID를 추상화한 클래스
  - IOnlineIdentity: 사용자 인증 정보 관리 인터페이스

  사용할 함수들

  // PlayerController 또는 GameInstance에서 구현
  void AYourPlayerController::GetSteamID()
  {
      // 1. OnlineSubsystem 가져오기
      IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
      if (!OnlineSubsystem)
      {
          UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem not found"));
          return;
      }

      // 2. Identity Interface 가져오기
      IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
      if (!IdentityInterface.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("IdentityInterface not valid"));
          return;
      }

      // 3. 로컬 유저 번호 (보통 0)
      int32 LocalUserNum = GetLocalPlayer()->GetControllerId();

      // 4. Steam ID 가져오기
      FUniqueNetIdPtr UniqueNetId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
      if (!UniqueNetId.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("UniqueNetId not valid"));
          return;
      }

      // 5. String으로 변환 (Steam ID 64비트 숫자)
      FString SteamIDString = UniqueNetId->ToString();
      UE_LOG(LogTemp, Log, TEXT("Steam ID: %s"), *SteamIDString);

      // 6. 서버에 조회 요청
      CheckNicknameOnServer(SteamIDString);
  }

  핵심 API

  | 인터페이스            | 메서드                      | 반환값               | 설명                    |
  |------------------|--------------------------|-------------------|-----------------------|
  | IOnlineSubsystem | Get()                    | IOnlineSubsystem* | Steam OSS 인스턴스        |
  | IOnlineIdentity  | GetUniquePlayerId(int32) | FUniqueNetIdPtr   | Steam ID              |
  | FUniqueNetId     | ToString()               | FString           | "765611XXXXXXXXXX" 형식 |
  | FUniqueNetId     | GetBytes()               | TArray<uint8>     | 바이너리 형태 (선택)          |

  ---
  🌐 2단계: 서버 조회 (HTTP 요청)

  필요한 지식

  - HTTP Module: UE의 HTTP 통신 모듈 (이미 사용 중)
  - JSON 파싱: 서버 응답 처리
  - 델리게이트: 비동기 콜백 처리

  사용할 함수들

  void AYourPlayerController::CheckNicknameOnServer(const FString& SteamID)
  {
      FHttpModule& HttpModule = FHttpModule::Get();
      TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

      // GET 방식 예시
      FString URL = FString::Printf(
          TEXT("http://100.115.252.51:8000/api/v1/user/nickname?steam_id=%s"),
          *SteamID
      );

      Request->SetURL(URL);
      Request->SetVerb(TEXT("GET"));
      Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

      // 콜백 바인딩
      Request->OnProcessRequestComplete().BindUObject(
          this,
          &AYourPlayerController::OnNicknameCheckComplete
      );

      Request->ProcessRequest();
  }

  void AYourPlayerController::OnNicknameCheckComplete(
      FHttpRequestPtr Request,
      FHttpResponsePtr Response,
      bool bWasSuccessful)
  {
      if (!bWasSuccessful || !Response.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));
          ShowNicknameInputUI(); // 실패 시 새 닉네임 입력
          return;
      }

      int32 ResponseCode = Response->GetResponseCode();
      FString ResponseContent = Response->GetContentAsString();

      if (ResponseCode == 200) // 닉네임 존재
      {
          // JSON 파싱
          TSharedPtr<FJsonObject> JsonObject;
          TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

          if (FJsonSerializer::Deserialize(Reader, JsonObject))
          {
              FString Nickname = JsonObject->GetStringField(TEXT("nickname"));
              SetPlayerNickname(Nickname);
          }
      }
      else if (ResponseCode == 404) // 닉네임 없음
      {
          ShowNicknameInputUI();
      }
  }

  서버 API 예상 스펙

  1. 닉네임 조회

  GET /api/v1/user/nickname?steam_id=76561198XXXXXXXX

  응답 (200 OK):
  {
      "steam_id": "76561198XXXXXXXX",
      "nickname": "PlayerName",
      "created_at": "2025-12-01T10:30:00Z"
  }

  응답 (404 Not Found):
  {
      "error": "Nickname not found"
  }

  2. 닉네임 등록

  POST /api/v1/user/nickname
  Content-Type: application/json

  {
      "steam_id": "76561198XXXXXXXX",
      "nickname": "NewPlayerName"
  }

  응답 (201 Created):
  {
      "steam_id": "76561198XXXXXXXX",
      "nickname": "NewPlayerName",
      "created_at": "2025-12-04T14:20:00Z"
  }

  ---
  👤 3단계: PlayerState에 닉네임 설정

  필요한 지식

  - PlayerState: 네트워크 복제되는 플레이어 정보 클래스
  - Replication: 멀티플레이어 동기화
  - Server RPC: 클라이언트→서버 함수 호출

  사용할 함수들

  // PlayerController에서 호출
  void AYourPlayerController::SetPlayerNickname(const FString& Nickname)
  {
      APlayerState* PS = GetPlayerState<APlayerState>();
      if (!PS)
      {
          UE_LOG(LogTemp, Error, TEXT("PlayerState is null"));
          return;
      }

      // PlayerState의 SetPlayerName은 자동으로 복제됨
      PS->SetPlayerName(Nickname);

      UE_LOG(LogTemp, Log, TEXT("Nickname set to: %s"), *Nickname);

      // 게임 진입 허용
      OnNicknameSetupComplete();
  }

  void AYourPlayerController::OnNicknameSetupComplete()
  {
      // UI 닫기
      if (NicknameInputWidget)
      {
          NicknameInputWidget->RemoveFromParent();
      }

      // 입력 모드 게임으로 전환
      FInputModeGameOnly InputMode;
      SetInputMode(InputMode);
      SetShowMouseCursor(false);

      // 게임 로직 시작 (예: 스폰 포인트 이동 등)
      ServerRPC_NotifyNicknameReady();
  }

  // 서버에 닉네임 설정 완료 알림
  void AYourPlayerController::ServerRPC_NotifyNicknameReady_Implementation()
  {
      // 서버 측 로직 (필요 시)
      UE_LOG(LogTemp, Log, TEXT("Player %s is ready"), *GetPlayerState<APlayerState>()->GetPlayerName());
  }

  PlayerState 기본 함수

  | 함수                     | 설명             |
  |------------------------|----------------|
  | SetPlayerName(FString) | 닉네임 설정 (자동 복제) |
  | GetPlayerName()        | 현재 닉네임 반환      |
  | GetUniqueId()          | UniqueNetId 반환 |

  ---
  🎨 4단계: 닉네임 입력 UI

  필요한 지식

  - UMG Widget: UI 시스템
  - EditableTextBox: 텍스트 입력 위젯
  - Button 이벤트: 클릭 핸들링

  Widget Blueprint 구조

  Canvas Panel
  ├─ Border (배경)
  │   └─ Vertical Box
  │       ├─ Text Block ("닉네임을 입력하세요")
  │       ├─ Editable Text Box (입력 필드)
  │       └─ Button ("확인")

  C++ Widget 클래스

  // Rene_NicknameInput_Widget.h
  UCLASS()
  class UE_RENE_API URene_NicknameInput_Widget : public UUserWidget
  {
      GENERATED_BODY()

  protected:
      virtual void NativeConstruct() override;

  private:
      UPROPERTY(meta=(BindWidget))
      TObjectPtr<UEditableTextBox> TextBox_Nickname;

      UPROPERTY(meta=(BindWidget))
      TObjectPtr<UButton> Btn_Confirm;

      UFUNCTION()
      void OnConfirmClicked();

      UFUNCTION()
      void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
  };

  // Rene_NicknameInput_Widget.cpp
  void URene_NicknameInput_Widget::NativeConstruct()
  {
      Super::NativeConstruct();

      if (Btn_Confirm)
      {
          Btn_Confirm->OnClicked.AddDynamic(this, &URene_NicknameInput_Widget::OnConfirmClicked);
      }

      if (TextBox_Nickname)
      {
          TextBox_Nickname->OnTextCommitted.AddDynamic(this, &URene_NicknameInput_Widget::OnTextCommitted);
      }
  }

  void URene_NicknameInput_Widget::OnConfirmClicked()
  {
      if (!TextBox_Nickname) return;

      FString Nickname = TextBox_Nickname->GetText().ToString().TrimStartAndEnd();

      // 유효성 검증
      if (Nickname.IsEmpty())
      {
          UE_LOG(LogTemp, Warning, TEXT("Nickname is empty"));
          return;
      }

      if (Nickname.Len() > 20)
      {
          UE_LOG(LogTemp, Warning, TEXT("Nickname too long"));
          return;
      }

      // PlayerController에 알림
      APlayerController* PC = GetOwningPlayer();
      if (AYourPlayerController* YourPC = Cast<AYourPlayerController>(PC))
      {
          YourPC->RegisterNicknameToServer(Nickname);
      }
  }

  void URene_NicknameInput_Widget::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
  {
      if (CommitMethod == ETextCommit::OnEnter)
      {
          OnConfirmClicked();
      }
  }

  ---
  🔄 5단계: 닉네임 서버 등록

  사용할 함수들

  void AYourPlayerController::RegisterNicknameToServer(const FString& Nickname)
  {
      // Steam ID 가져오기
      IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
      if (!OnlineSubsystem) return;

      IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
      if (!IdentityInterface.IsValid()) return;

      FUniqueNetIdPtr UniqueNetId = IdentityInterface->GetUniquePlayerId(0);
      if (!UniqueNetId.IsValid()) return;

      FString SteamID = UniqueNetId->ToString();

      // HTTP POST 요청
      FHttpModule& HttpModule = FHttpModule::Get();
      TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule.CreateRequest();

      Request->SetURL(TEXT("http://100.115.252.51:8000/api/v1/user/nickname"));
      Request->SetVerb(TEXT("POST"));
      Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

      // JSON 페이로드 생성
      TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
      JsonObject->SetStringField(TEXT("steam_id"), SteamID);
      JsonObject->SetStringField(TEXT("nickname"), Nickname);

      FString OutputString;
      TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
      FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

      Request->SetContentAsString(OutputString);

      Request->OnProcessRequestComplete().BindUObject(
          this,
          &AYourPlayerController::OnNicknameRegisterComplete
      );

      Request->ProcessRequest();
  }

  void AYourPlayerController::OnNicknameRegisterComplete(
      FHttpRequestPtr Request,
      FHttpResponsePtr Response,
      bool bWasSuccessful)
  {
      if (!bWasSuccessful || !Response.IsValid())
      {
          UE_LOG(LogTemp, Error, TEXT("Nickname registration failed"));
          // 에러 UI 표시
          return;
      }

      if (Response->GetResponseCode() == 201) // Created
      {
          FString ResponseContent = Response->GetContentAsString();
          TSharedPtr<FJsonObject> JsonObject;
          TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseContent);

          if (FJsonSerializer::Deserialize(Reader, JsonObject))
          {
              FString Nickname = JsonObject->GetStringField(TEXT("nickname"));
              SetPlayerNickname(Nickname);
          }
      }
      else
      {
          UE_LOG(LogTemp, Error, TEXT("Server returned code: %d"), Response->GetResponseCode());
      }
  }

  ---
  📦 6단계: 모듈 의존성 추가

  UE_ReNe.Build.cs 수정

  PublicDependencyModuleNames.AddRange(new string[]
  {
      "Core",
      "CoreUObject",
      "Engine",
      "InputCore",
      "SlateCore",
      "OnlineSubsystem",        // 이미 있음
      "OnlineSubsystemSteam",   // 이미 있음
      "OnlineSubsystemUtils",   // 추가 필요 (헬퍼 유틸리티)
      "Voice",
      "HTTP",
      "Json",                   // JSON 파싱
      "JsonUtilities",          // JSON 직렬화
  });

  ---
  🎯 전체 구현 순서

  1. GameInstance에서 초기화 (권장)

  // Rene_GameInstance.h
  UCLASS()
  class URene_GameInstance : public UGameInstance
  {
      GENERATED_BODY()

  public:
      virtual void Init() override;
      void CheckSteamLogin();

  private:
      void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString&
  Error);
  };

  // Rene_GameInstance.cpp
  void URene_GameInstance::Init()
  {
      Super::Init();
      CheckSteamLogin();
  }

  void URene_GameInstance::CheckSteamLogin()
  {
      IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
      if (!OnlineSubsystem) return;

      IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
      if (!IdentityInterface.IsValid()) return;

      // Steam은 자동 로그인되지만 명시적 확인
      if (IdentityInterface->GetLoginStatus(0) == ELoginStatus::LoggedIn)
      {
          FUniqueNetIdPtr UniqueNetId = IdentityInterface->GetUniquePlayerId(0);
          UE_LOG(LogTemp, Log, TEXT("Steam Auto-Login Success: %s"), *UniqueNetId->ToString());
      }
      else
      {
          // Steam 로그인 시도
          IdentityInterface->OnLoginCompleteDelegates->AddUObject(this, &URene_GameInstance::OnLoginComplete);
          IdentityInterface->Login(0, FOnlineAccountCredentials());
      }
  }

  void URene_GameInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
  const FString& Error)
  {
      if (bWasSuccessful)
      {
          UE_LOG(LogTemp, Log, TEXT("Steam Login Success: %s"), *UserId.ToString());
      }
      else
      {
          UE_LOG(LogTemp, Error, TEXT("Steam Login Failed: %s"), *Error);
      }
  }

  2. PlayerController에서 닉네임 체크

  void ARene_PlayerController::BeginPlay()
  {
      Super::BeginPlay();

      if (IsLocalController())
      {
          // 약간의 지연 후 체크 (PlayerState 초기화 대기)
          FTimerHandle TimerHandle;
          GetWorld()->GetTimerManager().SetTimer(
              TimerHandle,
              this,
              &ARene_PlayerController::GetSteamID,
              0.5f,
              false
          );
      }
  }

  ---
  📚 필요한 사전 지식 요약

  1. OnlineSubsystem 기초

  - Steam OSS 설정 (DefaultEngine.ini)
  - Identity Interface 개념
  - UniqueNetId 구조

  2. HTTP 통신

  - 비동기 요청/응답 패턴
  - 델리게이트 바인딩
  - JSON 직렬화/역직렬화

  3. UMG & Widget

  - Widget Blueprint 바인딩
  - 동적 위젯 생성/제거
  - 입력 모드 전환

  4. 네트워크 기초

  - PlayerState 복제
  - Server RPC
  - 클라이언트/서버 구분 (IsLocalController())

  ---
  🛠️ 구현 체크리스트

  - UE_ReNe.Build.cs에 Json, JsonUtilities 모듈 추가
  - Steam ID 추출 함수 구현
  - 서버 닉네임 조회 HTTP 요청 구현
  - 서버 닉네임 등록 HTTP 요청 구현
  - 닉네임 입력 Widget 제작
  - PlayerState 닉네임 설정 함수 구현
  - GameInstance 또는 PlayerController에 초기화 로직 추가
  - 에러 핸들링 (네트워크 실패, 중복 닉네임 등)
  - 서버 API 엔드포인트 확정 및 테스트

  ---
  💡 추가 고려사항

  1. 중복 닉네임 처리: 서버에서 409 Conflict 반환 시 재입력 요청
  2. 닉네임 검증: 욕설 필터, 길이 제한, 특수문자 제한
  3. 오프라인 대응: HTTP 요청 실패 시 Steam 닉네임을 임시 사용
  4. 캐싱: 한 번 조회한 닉네임을 GameInstance에 저장
  5. Steam 닉네임 기본값: 서버에 없을 경우 Steam 프로필 이름을 제안

  // Steam 프로필 닉네임 가져오기
  IOnlineIdentityPtr IdentityInterface = OnlineSubsystem->GetIdentityInterface();
  FString SteamNickname = IdentityInterface->GetPlayerNickname(0);