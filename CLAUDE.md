# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## ⚠️ CRITICAL: Language Requirement
**ALWAYS respond in Korean (한국어) when working on this project.**

## Project Overview

**MYP** is an Unreal Engine 5.6 multi-variant game project featuring three distinct gameplay types within a single codebase:
- **Combat Variant**: Third-person melee combat with combo/charged attack system
- **Platforming Variant**: Advanced movement mechanics (double jump, wall jump, dash)
- **SideScrolling Variant**: 2D platformer with interaction systems

**Engine Version**: Unreal Engine 5.6
**Module Type**: Runtime (single module)
**Primary Language**: C++ with Blueprint integration

## Build & Development Commands

### Building the Project
```bash
# Generate project files (if .sln missing or outdated)
# Right-click MYP.uproject → "Generate Visual Studio project files"

# Build from Rider
# Open MYP.sln → Build → Build Solution (Ctrl+Shift+B)
# Configuration: Development Editor
# Platform: Win64
```

### Running the Project
```bash
# Launch from Unreal Editor
# Double-click MYP.uproject

# Or build and launch from Rider
# Set MYP as startup project → Debug → Start Without Debugging (Ctrl+F5)
```

### Common Development Tasks
- **Compile C++ Changes**: Hot Reload in editor (Ctrl+Alt+F11) or rebuild solution
- **Generate .sln**: Right-click .uproject → "Generate Visual Studio project files"
- **Clean Build**: Delete `Binaries/`, `Intermediate/`, `Saved/` folders, regenerate project

## Architecture Overview

### Multi-Variant System Design

The project uses a **template-based inheritance pattern** where three game variants share base classes but implement completely separate gameplay logic:

```
Base Classes (MYP module root)
├── MYPCharacter (abstract template)
├── MYPGameMode (abstract template)
└── MYPPlayerController (abstract template)

Variant Implementations
├── Variant_Combat/
│   ├── CombatCharacter, CombatGameMode, CombatPlayerController
│   ├── AI/ (CombatEnemy, CombatAIController, StateTree utilities)
│   ├── Animation/ (AnimNotify_DoAttackTrace, CheckCombo, CheckChargedAttack)
│   ├── Gameplay/ (ActivationVolume, CheckpointVolume, Dummy, LavaFloor)
│   ├── Interfaces/ (ICombatAttacker, ICombatDamageable)
│   └── UI/ (CombatLifeBar widget component)
│
├── Variant_Platforming/
│   ├── PlatformingCharacter, PlatformingGameMode, PlatformingPlayerController
│   └── Animation/ (AnimNotify_EndDash)
│
└── Variant_SideScrolling/
    ├── SideScrollingCharacter, SideScrollingGameMode, SideScrollingPlayerController
    ├── AI/ (SideScrollingNPC, SideScrollingAIController)
    ├── Gameplay/ (Pickup, MovingPlatform, SoftPlatform, JumpPad)
    ├── Interfaces/ (ISideScrollingInteractable)
    └── UI/ (SideScrollingUI HUD widget)
```

**Key Principle**: Each variant has complete subsystem isolation—no cross-variant dependencies at runtime.

### Character System

**Base Character (MYPCharacter)**: Abstract class providing third-person camera setup and Enhanced Input bindings. Never directly instantiated.

**Combat Character**:
- Combo attack system with input buffering (`CachedAttackInputTime`)
- Charged attack with hold/release detection
- Implements `ICombatAttacker` and `ICombatDamageable` interfaces
- Health system with respawn logic (checkpoint-based)
- Sphere trace collision for melee attacks

**Platforming Character**:
- Double jump, wall jump, coyote time (jump forgiveness)
- Dash mechanic with AnimNotify-driven state management
- Optimized `UCharacterMovementComponent` settings for responsive platforming

**SideScrolling Character**:
- 2D horizontal movement constraint
- Soft platform collision (drop-through platforms)
- Proximity-based interaction system (`ISideScrollingInteractable`)

### AI System (StateTree-Based)

The project uses **UE5 StateTree** (not Behavior Trees) for AI decision-making:

**Core Components**:
- `ACombatAIController` / `ASideScrollingAIController`: Inherit from `AAIController`, contain `UStateTreeAIComponent`
- `CombatStateTreeUtility.h`: Custom StateTree nodes (tasks and conditions)

**StateTree Tasks** (examples):
- `FStateTreeComboAttackTask`: Execute combo attacks with delegate binding
- `FStateTreeFaceActorTask`: Rotate AI to face target
- `FStateTreeGetPlayerInfoTask`: Track player position/distance in real-time
- `FStateTreeWaitForLandingTask`: Pause execution until character lands

**Delegates for Synchronization**:
- `FOnEnemyAttackCompleted`: Fires when AnimMontage ends
- `FOnEnemyLanded`: Custom landing event (not using built-in `OnLanded`)

**Environmental Query System**:
- `EnvQueryContext_Player`: Provides player location context for EQS-based decision-making

### Animation System

The animation pipeline uses **custom AnimNotify classes** as event markers in AnimMontages:

**Combat AnimNotifies**:
- `AnimNotify_DoAttackTrace`: Triggers melee collision detection at specific frames
- `AnimNotify_CheckCombo`: Validates combo input buffer, advances to next attack section
- `AnimNotify_CheckChargedAttack`: Loops charge animation until button release

**Platforming AnimNotifies**:
- `AnimNotify_EndDash`: Restores player control after dash animation completes

All AnimNotify classes read animation context (`USkeletalMeshComponent`, `UAnimSequenceBase`) and call game logic on the owning character.

### Interface-Based Design

**Combat Interfaces**:
- `ICombatAttacker`: Contract for executing attacks (`DoAttackTrace()`, `CheckCombo()`)
- `ICombatDamageable`: Contract for receiving damage (`ApplyDamage()`, `HandleDeath()`, `ApplyHealing()`)

**SideScrolling Interfaces**:
- `ISideScrollingInteractable`: Contract for interactive objects (`Interaction(AActor*)`)

**Usage**: Characters and enemies implement these interfaces, allowing decoupled communication (StateTree tasks, collision handlers, etc. interact via interface pointers).

### Module Dependencies

**Required Modules** (from MYP.Build.cs):
```
Core, CoreUObject, Engine, InputCore, EnhancedInput,
AIModule, StateTreeModule, GameplayStateTreeModule,
UMG, Slate
```

**Include Path Structure**:
- All variant subdirectories are added to `PublicIncludePaths` (flat namespace)
- Allows cross-variant includes if needed (currently unused)
- Private code in `Private/` folder, public headers in variant subdirectories

### Logging System

**Central Log Category**: `MYPLOG` (declared in `MYP.h`)

**Convenience Macros**:
```cpp
SHOWLOG(Format, ...)     // Info level with file:line:function
SHOWLOGF(Format, ...)    // Info with formatted args
SHOWWARN(Format, ...)    // Warning level
SHOWERROR(Format, ...)   // Error level
```

**Usage**: Prefer these macros over `UE_LOG` for consistency—they automatically include source location.

## Development Guidelines

### Code Conventions

**Naming**:
- Classes: `A` prefix for Actors, `U` prefix for UObject-derived, `F` prefix for structs, `I` prefix for interfaces
- Variant prefix: `Combat`, `Platforming`, `SideScrolling` (e.g., `CombatCharacter`, `PlatformingGameMode`)
- StateTree tasks: `FStateTree[TaskName]Task` (e.g., `FStateTreeComboAttackTask`)
- AnimNotifies: `AnimNotify_[ActionName]` (e.g., `AnimNotify_DoAttackTrace`)

**File Organization**:
- Place variant-specific code in corresponding subdirectory (`Variant_Combat/AI/`, etc.)
- Use `Public/` for shared utility headers (currently only `ForBuildTest.h`)
- Keep interfaces separate from implementation (e.g., `Interfaces/CombatAttacker.h`)

**Interface Implementation**:
- Always implement both header declaration and C++ definition for interface methods
- Use `UINTERFACE`/`IInterface` pattern (UE4/5 standard)

### Animation Integration

**When Creating AnimNotifies**:
1. Inherit from `UAnimNotify`
2. Override `Notify(USkeletalMeshComponent*, UAnimSequenceBase*)`
3. Override `GetNotifyName_Implementation()` for editor display
4. Get owning character via `MeshComp->GetOwner()`
5. Cast to interface or specific character class, call game logic

**When Using StateTree Tasks**:
1. Define task struct inheriting from `FStateTreeTaskCommonBase` or `FStateTreeTaskBase`
2. Implement `EnterState()`, `ExitState()`, `Tick()` as needed
3. Use `FStateTreeExecutionContext` to access AI controller/blackboard
4. Bind delegates if task requires async completion (e.g., animation montage end)

### Common Patterns

**Checkpoint System (Combat Variant)**:
- `ACombatCheckpointVolume` updates `ACombatPlayerController->RespawnTransform`
- On death, `CombatCharacter` calls `PlayerController->Respawn()`

**Activation Volumes (Combat Variant)**:
- `ACombatActivationVolume` holds `TArray<AActor*> TargetsToActivate`
- On player overlap, calls `Activate()` on all targets (typically `CombatEnemySpawner`)

**Soft Platforms (SideScrolling Variant)**:
- `ASideScrollingSoftPlatform` uses custom collision traces
- Allows downward pass-through when player presses down+jump

**Widget Components**:
- `UCombatLifeBar` attached to `CombatCharacter`/`CombatEnemy` as 3D widget
- Update via `SetLifePercentage(float)` Blueprint implementable event

## Testing

**No automated tests currently implemented.**
Manual testing required for all changes. Focus areas:
- Combat: Combo timing, charged attack hold/release, enemy AI aggro
- Platforming: Coyote time window, dash collision, wall jump detection
- SideScrolling: Soft platform drop-through, pickup collection, NPC interaction

## Git Workflow

**Branches**:
- `master`: Main stable branch (for PRs)
- `Dev`: Active development branch

**Commit Conventions**:
- Follow existing style (check `git log --oneline -10`)
- Use prefixes: `[CONFIG]`, `[FIX]`, `[DOCS]`, `[FEATURE]`, etc.

## Key Files Reference

| File | Purpose |
|------|---------|
| `MYP.Build.cs` | Module dependencies and include paths |
| `MYP.h` | Log category definitions and convenience macros |
| `MYPCharacter.h` | Base character template (abstract) |
| `Variant_Combat/CombatCharacter.h` | Combat variant player character |
| `Variant_Combat/AI/CombatStateTreeUtility.h` | StateTree task/condition definitions |
| `Config/DefaultEngine.ini` | Engine configuration (renderer, input, maps) |
| `Config/DefaultInput.ini` | Input action mappings |

## Project-Specific Notes

- **Enhanced Input System**: All variants use UE5 Enhanced Input (not legacy input)
- **StateTree over Behavior Trees**: Combat and SideScrolling AI use StateTree exclusively
- **No Plugins**: All code in single `MYP` module (no custom plugins)
- **Blueprint Integration**: GameMode/Character classes intended for Blueprint child classes (e.g., `BP_CombatCharacter`)

## Troubleshooting

**Compile Errors with Include Paths**:
- Ensure `PublicIncludePaths` in `MYP.Build.cs` includes variant subdirectory
- Includes use flat namespace: `#include "CombatCharacter.h"` (not `#include "Variant_Combat/CombatCharacter.h"`)

**StateTree Tasks Not Executing**:
- Verify `UStateTreeAIComponent` is active on AIController
- Check delegate bindings in task `EnterState()` (must bind to valid UObject)
- Ensure task returns `EStateTreeRunStatus::Running` if async

**AnimNotify Not Firing**:
- Check AnimMontage has notify marker at correct frame
- Verify character's `USkeletalMeshComponent` has animation asset assigned
- Ensure character class implements required interface (for interface method calls)

**Hot Reload Failures**:
- Delete `Binaries/`, `Intermediate/` folders
- Regenerate project files (right-click .uproject)
- Rebuild solution from Rider

---

*This document is maintained for Claude Code efficiency. Update when architecture changes significantly.*
