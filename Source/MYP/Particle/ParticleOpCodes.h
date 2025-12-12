#pragma once

#include "CoreMinimal.h"

/*
 * GPU Compute Shader에서 실행할 바이트코드 OpCode 정의
 * 스택 기반 VM 방식
 */

enum class EParticleOpCode : uint8
{
	//	스택 조작
	PUSH_CONST = 0,				//	상수 풀에서 값을 가져와 스택에 푸시
								//	다음 바이트: 상수 인덱스
	PUSH_VAR = 1,				//	변수를 스택에 푸시
								//	다음 바이트: 변수 ID (0=particleId, 1=time)
	
	//	산술연산 (스택 top 2개 사용)
	ADD = 10,					//	Pop 2개, 더하기, Push 결과
	SUB = 11,					//	Pop 2개, 빼기, Push 결과
	MUL = 12,					//	Pop 2개, 곱하기, Push 결과
	DIV = 13,					//	Pop 2개, 나누기, Push 결과
	
	//	수학함수 (스택 top 1개 사용)
	SIN = 20,					//	Pop 1개, sin() 계산, Push 결과
	COS = 21,					//	Pop 1개, cos() 계산, Push 결과
	
	//	결과 저장 (스택 top 1개를 결과 변수에 저장 )
	STORE_POS_X = 30,			//	Pop 1개, 파티클 Position.X에 저장
	STORE_POS_Y = 31,			//	Pop 1개, 파티클 Position.Y에 저장
	STORE_POS_Z = 32,			//	Pop 1개, 파티클 Position.Z에 저장
	
	STORE_COLOR_R = 40,			//	Pop 1개, 파티클 Color.R에 저장
	STORE_COLOR_G = 41,			//	Pop 1개, 파티클 Color.G에 저장
	STORE_COLOR_B = 42,			//	Pop 1개, 파티클 Color.B에 저장
	
	//	제어 흐름
	HALT = 255					//	실행 종료
};

/*
 *	PUSH_VAR에서 사용할 변수 ID
 */
enum class EParticleVariable : uint8
{
	PARTICLE_ID = 0,			//	현재 파티클 인덱스 (0, 1, 2, ...)
	TIME = 1					//	경과 시간 (sec)
};
