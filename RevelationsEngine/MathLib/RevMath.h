#pragma once

#include "RevMatrix.h"
#include "RevVector.h"
#include "RevQuat.h"

#define USE_D3D 0

#if USE_D3D
#include "DirectXMath.h"
using namespace DirectX;
#endif

inline RevVector operator*(const RevVector& vector, const RevMatrix& matrix)
{
	RevVector returnVector = {};

	returnVector[0] = vector[0] * matrix[0] + vector[1] * matrix[4] + vector[2] * matrix[8] + vector[3] * matrix[12];
	returnVector[1] = vector[0] * matrix[1] + vector[1] * matrix[5] + vector[2] * matrix[9] + vector[3] * matrix[13];
	returnVector[2] = vector[0] * matrix[2] + vector[1] * matrix[6] + vector[2] * matrix[10] + vector[3] * matrix[14];
	returnVector[3] = vector[0] * matrix[3] + vector[1] * matrix[7] + vector[2] * matrix[11] + vector[3] * matrix[15];
	return returnVector;
}

inline RevMatrix RevCreateLookAt(const RevVector3& towards)
{
	RevVector3 up;
	if (towards.Y() > 0.98)
	{
		up = RevVector3(0, 0, -1.0f);
	}
	else
		if (towards.Y() < -0.98f)
		{
			up = RevVector3(0, 0, 1.0f);
		}
		else
		{
			up = RevVector3(0, 1.0f, 0);
		}

	RevVector3 newRight = up.CrossProduct(towards);
	RevVector3 newUp = towards.CrossProduct(newRight);
	RevMatrix returnMatrix(newRight, newUp, towards);
	return returnMatrix;
}

inline XMMATRIX CreateXMMMatrix(const RevMatrix& inMatrix)
{
	XMMATRIX viewMatrixD3D;

	viewMatrixD3D.r[0] = XMVectorSet(inMatrix[0], inMatrix[1], inMatrix[2], inMatrix[3]);
	viewMatrixD3D.r[1] = XMVectorSet(inMatrix[4], inMatrix[5], inMatrix[6], inMatrix[7]);
	viewMatrixD3D.r[2] = XMVectorSet(inMatrix[8], inMatrix[9], inMatrix[10], inMatrix[11]);
	viewMatrixD3D.r[3] = XMVectorSet(inMatrix[12], inMatrix[13], inMatrix[14], inMatrix[15]);

	return viewMatrixD3D;
}


inline RevQuat RevRotationMatrixToQuaternion(const RevMatrix& matrix)
{
	RevQuat returnQuat;
#if USE_D3D
	XMMATRIX m = CreateXMMMatrix(matrix);
	XMVECTOR d3dQ = XMQuaternionRotationMatrix(m);
	XMFLOAT4 float4;
	XMStoreFloat4(&float4, d3dQ);
	returnQuat[0] = float4.x;
	returnQuat[1] = float4.y;
	returnQuat[2] = float4.z;
	returnQuat[3] = float4.w;
	return returnQuat;
#else
	//based around multiplication with quaternion that contains both parts and one with point
	//end result becomes close to rotation axis around same angle and 0 out real part
	//for us to construct a quaternion that is required to do this multiplication we have a final vector( see QuaternionToRotationMatrix)
	//there is some algerbic terms of figuring out these once, the simples one is where real part is not zero
	//most of them are relying on other terms explained in the other functions in revquat
	//with addition pow(q1, 2) + pow(q2, 2)+ pow(q3, 2)+ pow(q4, 2)= 1
	//which means also that 1 - pow(q4, 2) = pow(q1, 2) + pow(q2, 2) + pow(q3, 2)
	//so if you have the trace of a the rotation matrix of the qpq*
	//it can be written down in forms to trace(r) = 3-4(pow(q1, 2) + pow(q2, 2) + pow(q3, 2))
	//which we then know can also be trace(r) = 3-4(1-pow(q4,2))
	//which theb can be derived down to 3 - 4 - 4pow(q4,2)
	//-1 - pow(q4, 2)
	//square both side 
	// sqrtf(trace(r)) = -1 - 2q4
	//sqrtf(trace(r)) + 1 = 2q4
	//(sqrtf(trace(r)) + 1) / 2 = q4
	const float trace = matrix.TraceRotation();
	if (trace < 0.0f)
	{
		//if the real part is zero we cant simply rely on certain rotation matrix calculations but need to find another one to divide by
		//we need to do there a combination of certain rotation matrix elements to then produce our product
		//we use X as an example (there is an important rule afterwards) but the combiniation we will use for X is M[0] - M[5] - M[10] which in the qpq* can be derived to 1 + 4q2
		//This means we can write R1 - R2 - R3 = - 1 + 4q2
		//more arthemetic leads us to be able to do sqrtf((R1 - R2 - R3) + 1) / 2 = q2
		// the only issue is we can't square root a negative matrix so we need to find the biggest index for each given one and then we can calculate it
		// once we know one, we can just find out the other ones

		uint32_t biggestIndex = matrix.FindBiggestDiagonalAxis();
		uint32_t matrixIndex = biggestIndex * 5;
		uint32_t biggestIndexOne = biggestIndex + 1 > 2 ? 0 : biggestIndex + 1;
		uint32_t biggestIndexTwo = biggestIndexOne + 1 > 2 ? 0 : biggestIndexOne + 1;

		float a = matrix.m_m[matrixIndex];
		float b = matrix.m_m[biggestIndexOne * 5];
		float c = matrix.m_m[biggestIndexTwo * 5];

		returnQuat.m_v[biggestIndex] = (sqrtf((a - b - c)) + 1) / 2;
		DEBUG_ASSERT(returnQuat.m_v[biggestIndex] != 0);
		returnQuat.m_v[biggestIndexOne] = (matrix[0 + biggestIndexOne] + matrix[4 + (biggestIndex)]) / (4 * returnQuat.m_v[biggestIndex]);
		returnQuat.m_v[biggestIndexTwo] = (matrix[0 + biggestIndexTwo] + matrix[8 + biggestIndex]) / (4 * returnQuat.m_v[biggestIndex]);
		returnQuat.m_real = (matrix[4 + biggestIndexTwo] - matrix[8 + biggestIndexOne]) / (4 * returnQuat.m_v[biggestIndex]);
	}
	else
	{
		returnQuat.m_real = sqrtf(trace + 1) / 2;
		float invQ = 4.0f * returnQuat.m_real;
		returnQuat.m_v[0] = (matrix[6] - matrix[9]) / invQ;
		returnQuat.m_v[1] = (matrix[8] - matrix[2]) / invQ;
		returnQuat.m_v[2] = (matrix[1] - matrix[4]) / invQ;
	}
#endif
	return returnQuat;

}


inline RevMatrix RevQuaternionToRotationMatrix(const RevQuat& quat)
{
	RevMatrix returnMatrix;
#if USE_D3D
	XMVECTOR d3dQ = XMVectorSet(quat[0], quat[1], quat[2], quat[3]);
	XMMATRIX d3dM = XMMatrixRotationQuaternion(d3dQ);
	XMFLOAT4X4 uncomrpess;
	XMStoreFloat4x4(&uncomrpess, d3dM);
	returnMatrix.Load(&uncomrpess._11);
#else

	//the end result of qpq* is a rotation matrix with specific elements
	//we take each element here and derive it

	returnMatrix.m_m[0] = 1 - 2 * quat[1] * quat[1] - 2 * quat[2] * quat[2];
	returnMatrix.m_m[1] = 2 * quat[0] * quat[1] + 2 * quat[2] * quat[3];
	returnMatrix.m_m[2] = 2 * quat[0] * quat[2] - 2 * quat[1] * quat[3];
	returnMatrix.m_m[3] = 0.0f;

	returnMatrix.m_m[4] = 2 * quat[0] * quat[1] - 2 * quat[2] * quat[3];
	returnMatrix.m_m[5] = 1 - 2 * quat[0] * quat[0] - 2 * quat[2] * quat[2];
	returnMatrix.m_m[6] = 2 * quat[1] * quat[2] + 2 * quat[0] * quat[3];
	returnMatrix.m_m[7] = 0.0f;

	returnMatrix.m_m[8] = 2 * quat[0] * quat[2] + 2 * quat[1] * quat[3];
	returnMatrix.m_m[9] = 2 * quat[1] * quat[2] - 2 * quat[0] * quat[3];
	returnMatrix.m_m[10] = 1 - 2 * quat[0] * quat[0] - 2 * quat[1] * quat[1];
	returnMatrix.m_m[11] = 0.0f;

	returnMatrix.m_m[12] = 0.0f;
	returnMatrix.m_m[13] = 0.0f;
	returnMatrix.m_m[14] = 0.0f;
	returnMatrix.m_m[15] = 1.0f;

#endif
	return returnMatrix;
}

inline RevQuat RevQuatenerionSlerp(const RevQuat& q1, const RevQuat& q2, float t)
{

#if USE_D3D
	RevQuat returnQuat;
	XMVECTOR d3dQ1, d3dQ2;
	d3dQ1 = XMVectorSet(q1[0], q1[1], q1[2], q1[3]);
	d3dQ2 = XMVectorSet(q2[0], q2[1], q2[2], q2[3]);

	XMVECTOR d3dQResult = XMQuaternionSlerp(d3dQ1, d3dQ2, t);
	XMFLOAT4 float4;
	XMStoreFloat4(&float4, d3dQResult);
	returnQuat[0] = float4.x;
	returnQuat[1] = float4.y;
	returnQuat[2] = float4.z;
	returnQuat[3] = float4.w;
	return returnQuat;
#else
	//the basic is we have an initial rotation
	//we want to face another rotation a certain amount (define by t)
	//we want it to be spherical though and not linear so the final result should have the same magnitude as both other quats
	// ||p|| = ||a|| = ||b||
	// a = this
	// b = otherquat
	// p = result
	// we know that this dot otherquat == cos(x); by previous formula
	//this dot quat = this_vdot this_q + s*t = ||q||||v|| sin(x)
	//as magnitude is 1 for both V it just becomes cos(x) = pdotv
	//so we know basically that dot gives us an angle, what we need to do now is able to modify the dot to give us the amount to be spherical
	//we introduce two constant variables(c1, c2) which will give us this that we currently do not know
	//p = c1 *a dot b * c2
	//p will be at a t0 so...
	//a dot p = c1 *a dot a + c2 * b dot a
	// we know a dot a is 1 as they are both magnitude and that a dot b give us the full angle
	// a dot p = c1 + c2*cos(0)
	//this dot will obviously give us the same (cos(0t)) = c1 + c2*cos(0)
	// we can do the same for p dot b  (cos((1 - t)*0)) = c2*cos(0) + c2
	// we can build a matrix for this calulcation with vector
	// [ 1		cos(0)	][C1]	= [(cos(0t))]
	// [ cos(0)	1		][C2]	= [(cos((1 - t)*0))]
	//There is a rule in MatrixMath which is Ax = B for invertable matixes (something that this matrix is)
	//Ax = B == xi = Ai/Adet
	//So for c1
	//c1 =	[ cos(0t)	(cos0)	]  /	[	1		cos(0)	]
	//		[ cos(0)	1		]		[	cos(0)	1		]
	//		
	//same for c2 however instead of first element on diagonal, the second one is exchanged.
	// with this we find out both c1 and c2 and forumula in the end become:
	//slerp (a, b, t) = sin((1-t)0)a + sin(t0)b / sin(0)
	// 0(angle) is obviously the arcos of p dot v

	//there is one more thing as well we need to decide
	//which is a funky thing with quatenerions
	//-q = q represent the exact same rotation however if we always want to rotate the fastest way towards a target we need to pick the one which will give us the shortest path
	//to determine this we calculate the difference between those two
	// pow(||a - b||, 2) vs pow(||a - (-b) ||, 2) pick the one that has the smallest magnitude


	const RevQuat b = (q1 - q2).LengthSquared() > (q1 + q2).LengthSquared() ? -q2 : q2;

	const float dotAB = q1.Dot(b);
	if (dotAB > (1.0f - 0.001f))
	{
		//lerp instead then
		return (q1 * (1 - t) + b * t).Normalize();
	}
	const float angle = acosf(dotAB);
	const float sinAngle = sinf(angle);
	const float invSinAngle = 1 / sinAngle;
	return (q1*(sinf(angle*(1 - t)) * invSinAngle) + b*(sinf(angle * t) * invSinAngle));

#endif
}


inline RevMatrix RevLocationAndQuatenerionToMatrix(const RevQuat& quat, const RevVector3& location)
{
	RevMatrix returnMatrix = RevQuaternionToRotationMatrix(quat);
	returnMatrix.SetLocation(location);
	return returnMatrix;
}

inline RevMatrix RevCreateRotationFromX(float xAngle)
{
	RevMatrix returnMatrix;
	returnMatrix.Identity();

	const float xRadianAngle = REV_ANGLE_TO_RADIAN * xAngle;

	const float cosAngle = cosf(xRadianAngle);
	const float sinAngle = sinf(xRadianAngle);

	returnMatrix.m_m[5] = cosAngle;
	returnMatrix.m_m[9] = -sinAngle;
	returnMatrix.m_m[6] = sinAngle;
	returnMatrix.m_m[10] = cosAngle;

	return returnMatrix;
}

inline RevMatrix RevCreateRotationFromY(float xAngle)
{
	RevMatrix returnMatrix;
	returnMatrix.Identity();

	const float xRadianAngle = REV_ANGLE_TO_RADIAN * xAngle;

	const float cosAngle = cosf(xRadianAngle);
	const float sinAngle = sinf(xRadianAngle);

	returnMatrix.m_m[10] = cosAngle;
	returnMatrix.m_m[2] = -sinAngle;
	returnMatrix.m_m[8] = sinAngle;
	returnMatrix.m_m[0] = cosAngle;
	return returnMatrix;
}

inline RevMatrix RevCreateRotationFromZ(float xAngle)
{
	RevMatrix returnMatrix;
	returnMatrix.Identity();

	const float xRadianAngle = REV_ANGLE_TO_RADIAN * xAngle;

	const float cosAngle = cosf(xRadianAngle);
	const float sinAngle = sinf(xRadianAngle);

	returnMatrix.m_m[0] = cosAngle;
	returnMatrix.m_m[4] = -sinAngle;
	returnMatrix.m_m[1] = sinAngle;
	returnMatrix.m_m[5] = cosAngle;
	return returnMatrix;
}
