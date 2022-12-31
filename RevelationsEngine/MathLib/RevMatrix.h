#pragma once
#include "RevAssert.h"
#include "RevVector.h"
#include <math.h>
#include <intrin.h>

#define MakeShuffleMask(x,y,z,w)           (x | (y<<2) | (z<<4) | (w<<6))

// vec(0, 1, 2, 3) -> (vec[x], vec[y], vec[z], vec[w])
#define VecSwizzle(vec, x,y,z,w)           _mm_shuffle_ps(vec, vec, MakeShuffleMask(x,y,z,w))
#define VecSwizzle1(vec, x)                _mm_shuffle_ps(vec, vec, MakeShuffleMask(x,x,x,x))
// special swizzle
#define VecSwizzle_0101(vec)               _mm_movelh_ps(vec, vec)
#define VecSwizzle_2323(vec)               _mm_movehl_ps(vec, vec)
#define VecSwizzle_0022(vec)               _mm_moveldup_ps(vec)
#define VecSwizzle_1133(vec)               _mm_movehdup_ps(vec)

// return (vec1[x], vec1[y], vec2[z], vec2[w])
#define VecShuffle(vec1, vec2, x,y,z,w)    _mm_shuffle_ps(vec1, vec2, MakeShuffleMask(x,y,z,w))
// special shuffle
#define VecShuffle_0101(vec1, vec2)        _mm_movelh_ps(vec1, vec2)
#define VecShuffle_2323(vec1, vec2)        _mm_movehl_ps(vec2, vec1)
#define REV_SMALL_NUMBER		(1.e-8f)

enum class RevMatrixAxis : uint8_t
{
	Right,
	Up,
	Forward,
	MAX
};

__declspec(align(16)) class RevMatrix
{
public:
	RevMatrix() {};

	RevMatrix(const float* data)
	{
		Load(data);
	}

	RevMatrix(RevVector right, RevVector up, RevVector forward)
	{
		memset(this, 0, sizeof(RevMatrix));
		mVec[0] = _mm_load_ps(&right.m_v[0]);
		mVec[1] = _mm_load_ps(&up.m_v[0]);
		mVec[2] = _mm_load_ps(&forward.m_v[0]);
		m_m[15] = 1.0f;
	}

	RevMatrix(RevVector3 right, RevVector3 up, RevVector3 forward)
	{

		memset(this, 0, sizeof(RevMatrix));
		m_m[0] = right[0];
		m_m[1] = right[1];
		m_m[2] = right[2];

		m_m[4] = up[0];
		m_m[5] = up[1];
		m_m[6] = up[2];

		m_m[8] = forward[0];
		m_m[9] = forward[1];
		m_m[10] = forward[2];
		m_m[15] = 1.0f;
	}


	void Identity()
	{
		memset(this, 0, sizeof(RevMatrix));
		m_m[0] = 1.0f;
		m_m[5] = 1.0f;
		m_m[10] = 1.0f;
		m_m[15] = 1.0f;
	}

	void operator=(const RevMatrix& otherMatrix)
	{
		memcpy(m_m, otherMatrix.m_m, sizeof(otherMatrix));
	}

	RevMatrix operator*(const RevMatrix& otherMatrix) const
	{
		RevMatrix returnMatrix;
		returnMatrix.m_m[0] = m_m[0] * otherMatrix.m_m[0] + m_m[1] * otherMatrix.m_m[4] + m_m[2] * otherMatrix.m_m[8] + m_m[3] * otherMatrix.m_m[12];
		returnMatrix.m_m[1] = m_m[0] * otherMatrix.m_m[1] + m_m[1] * otherMatrix.m_m[5] + m_m[2] * otherMatrix.m_m[9] + m_m[3] * otherMatrix.m_m[13];
		returnMatrix.m_m[2] = m_m[0] * otherMatrix.m_m[2] + m_m[1] * otherMatrix.m_m[6] + m_m[2] * otherMatrix.m_m[10] + m_m[3] * otherMatrix.m_m[11];
		returnMatrix.m_m[3] = m_m[0] * otherMatrix.m_m[3] + m_m[1] * otherMatrix.m_m[7] + m_m[2] * otherMatrix.m_m[11] + m_m[3] * otherMatrix.m_m[15];


		returnMatrix.m_m[4] = m_m[4] * otherMatrix.m_m[0] + m_m[5] * otherMatrix.m_m[4] + m_m[6] * otherMatrix.m_m[8] + m_m[7] * otherMatrix.m_m[12];
		returnMatrix.m_m[5] = m_m[4] * otherMatrix.m_m[1] + m_m[5] * otherMatrix.m_m[5] + m_m[6] * otherMatrix.m_m[9] + m_m[7] * otherMatrix.m_m[13];
		returnMatrix.m_m[6] = m_m[4] * otherMatrix.m_m[2] + m_m[5] * otherMatrix.m_m[6] + m_m[6] * otherMatrix.m_m[10] + m_m[7] * otherMatrix.m_m[14];
		returnMatrix.m_m[7] = m_m[4] * otherMatrix.m_m[3] + m_m[5] * otherMatrix.m_m[7] + m_m[6] * otherMatrix.m_m[11] + m_m[7] * otherMatrix.m_m[15];



		returnMatrix.m_m[8] = m_m[8] * otherMatrix.m_m[0] + m_m[9] * otherMatrix.m_m[4] + m_m[10] * otherMatrix.m_m[8] + m_m[11] * otherMatrix.m_m[12];
		returnMatrix.m_m[9] = m_m[8] * otherMatrix.m_m[1] + m_m[9] * otherMatrix.m_m[5] + m_m[10] * otherMatrix.m_m[9] + m_m[11] * otherMatrix.m_m[13];
		returnMatrix.m_m[10] = m_m[8] * otherMatrix.m_m[2] + m_m[9] * otherMatrix.m_m[6] + m_m[10] * otherMatrix.m_m[10] + m_m[11] * otherMatrix.m_m[14];
		returnMatrix.m_m[11] = m_m[8] * otherMatrix.m_m[3] + m_m[9] * otherMatrix.m_m[7] + m_m[10] * otherMatrix.m_m[11] + m_m[11] * otherMatrix.m_m[15];



		returnMatrix.m_m[12] = m_m[12] * otherMatrix.m_m[0] + m_m[13] * otherMatrix.m_m[4] + m_m[14] * otherMatrix.m_m[8] + m_m[15] * otherMatrix.m_m[12];
		returnMatrix.m_m[13] = m_m[12] * otherMatrix.m_m[1] + m_m[13] * otherMatrix.m_m[5] + m_m[14] * otherMatrix.m_m[9] + m_m[15] * otherMatrix.m_m[13];
		returnMatrix.m_m[14] = m_m[12] * otherMatrix.m_m[2] + m_m[13] * otherMatrix.m_m[6] + m_m[14] * otherMatrix.m_m[10] + m_m[15] * otherMatrix.m_m[14];
		returnMatrix.m_m[15] = m_m[12] * otherMatrix.m_m[3] + m_m[13] * otherMatrix.m_m[7] + m_m[14] * otherMatrix.m_m[11] + m_m[15] * otherMatrix.m_m[15];

		return returnMatrix;
	}

	void Load(const float* data)
	{
		mVec[0] = _mm_load_ps(&data[0]);
		mVec[1] = _mm_load_ps(&data[4]);
		mVec[2] = _mm_load_ps(&data[8]);
		mVec[3] = _mm_load_ps(&data[12]);
	}

	RevMatrix Transpose() const
	{
		//we do not transpose location
		RevMatrix returnMatrix = *this;
		returnMatrix.m_m[1] = m_m[4];
		returnMatrix.m_m[2] = m_m[8];

		returnMatrix.m_m[4] = m_m[1];
		returnMatrix.m_m[6] = m_m[9];

		returnMatrix.m_m[8] = m_m[2];
		returnMatrix.m_m[9] = m_m[6];


		returnMatrix.m_m[3] = m_m[12];
		returnMatrix.m_m[7] = m_m[13];
		returnMatrix.m_m[11] = m_m[14];


		returnMatrix.m_m[12] = m_m[3];
		returnMatrix.m_m[13] = m_m[7];
		returnMatrix.m_m[14] = m_m[11];

		return returnMatrix;
	}

	RevMatrix TransposeSimd() const
	{
		RevMatrix r;

		// transpose 3x3, we know m03 = m13 = m23 = 0
		__m128 t0 = VecShuffle_0101(mVec[0], mVec[1]); // 00, 01, 10, 11
		__m128 t1 = VecShuffle_2323(mVec[0], mVec[1]); // 02, 03, 12, 13
		r.mVec[0] = VecShuffle(t0, mVec[2], 0, 2, 0, 3); // 00, 10, 20, 23(=0)
		r.mVec[1] = VecShuffle(t0, mVec[2], 1, 3, 1, 3); // 01, 11, 21, 23(=0)
		r.mVec[2] = VecShuffle(t1, mVec[2], 0, 2, 2, 3); // 02, 12, 22, 23(=0)

														 // last line
		r.mVec[3] = _mm_mul_ps(r.mVec[0], VecSwizzle1(mVec[3], 0));
		r.mVec[3] = _mm_add_ps(r.mVec[3], _mm_mul_ps(r.mVec[1], VecSwizzle1(mVec[3], 1)));
		r.mVec[3] = _mm_add_ps(r.mVec[3], _mm_mul_ps(r.mVec[2], VecSwizzle1(mVec[3], 2)));
		r.mVec[3] = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), r.mVec[3]);
		return r;
	}


	float& operator[](int index)
	{
		return m_m[index];
	}

	float operator[](int index) const
	{
		return m_m[index];
	}

	RevVector3 GetLocation() const
	{
		return RevVector3(m_m[12], m_m[13], m_m[14]);
	}

	RevVector3 GetAxis(RevMatrixAxis axis)
	{
		switch (axis)
		{
		case RevMatrixAxis::Right:
			return RevVector3(m_m[0], m_m[1], m_m[2]);
		case RevMatrixAxis::Up:
			return RevVector3(m_m[4], m_m[5], m_m[6]);
		case RevMatrixAxis::Forward:
			return RevVector3(m_m[8], m_m[9], m_m[10]);
		default:
			static RevVector3 returnVector(0.0f, 0.0f, 0.0f);
			DEBUG_ASSERT(false);
			return returnVector;
		};
	}

	void SetLocation(const RevVector3& vector)
	{
		m_m[12] = vector[0];
		m_m[13] = vector[1];
		m_m[14] = vector[2];
	}

	void SetLocation(const RevVector& vector)
	{
		m_m[12] = vector[0];
		m_m[13] = vector[1];
		m_m[14] = vector[2];
	}

	/** 
	IF IN DOUBT THIS FUNCTION WORKS, WHICH IT SHOULD. USE GLU OPTIMIZED

	double inv[16];
	inv[0] = m_m[5] * m_m[10] * m_m[15] -
	m_m[5] * m_m[11] * m_m[14] -
	m_m[9] * m_m[6] * m_m[15] +
	m_m[9] * m_m[7] * m_m[14] +
	m_m[13] * m_m[6] * m_m[11] -
	m_m[13] * m_m[7] * m_m[10];

	inv[1] = -m_m[1] * m_m[10] * m_m[15] +
	m_m[1] * m_m[11] * m_m[14] +
	m_m[9] * m_m[2] * m_m[15] -
	m_m[9] * m_m[3] * m_m[14] -
	m_m[13] * m_m[2] * m_m[11] +
	m_m[13] * m_m[3] * m_m[10];

	inv[2] = m_m[1] * m_m[6] * m_m[15] -
	m_m[1] * m_m[7] * m_m[14] -
	m_m[5] * m_m[2] * m_m[15] +
	m_m[5] * m_m[3] * m_m[14] +
	m_m[13] * m_m[2] * m_m[7] -
	m_m[13] * m_m[3] * m_m[6];


	inv[3] = -m_m[1] * m_m[6] * m_m[11] +
	m_m[1] * m_m[7] * m_m[10] +
	m_m[5] * m_m[2] * m_m[11] -
	m_m[5] * m_m[3] * m_m[10] -
	m_m[9] * m_m[2] * m_m[7] +
	m_m[9] * m_m[3] * m_m[6];

	inv[4] = -m_m[4] * m_m[10] * m_m[15] +
	m_m[4] * m_m[11] * m_m[14] +
	m_m[8] * m_m[6] * m_m[15] -
	m_m[8] * m_m[7] * m_m[14] -
	m_m[12] * m_m[6] * m_m[11] +
	m_m[12] * m_m[7] * m_m[10];
	inv[5] = m_m[0] * m_m[10] * m_m[15] -
	m_m[0] * m_m[11] * m_m[14] -
	m_m[8] * m_m[2] * m_m[15] +
	m_m[8] * m_m[3] * m_m[14] +
	m_m[12] * m_m[2] * m_m[11] -
	m_m[12] * m_m[3] * m_m[10];
	inv[6] = -m_m[0] * m_m[6] * m_m[15] +
	m_m[0] * m_m[7] * m_m[14] +
	m_m[4] * m_m[2] * m_m[15] -
	m_m[4] * m_m[3] * m_m[14] -
	m_m[12] * m_m[2] * m_m[7] +
	m_m[12] * m_m[3] * m_m[6];

	inv[7] = m_m[0] * m_m[6] * m_m[11] -
	m_m[0] * m_m[7] * m_m[10] -
	m_m[4] * m_m[2] * m_m[11] +
	m_m[4] * m_m[3] * m_m[10] +
	m_m[8] * m_m[2] * m_m[7] -
	m_m[8] * m_m[3] * m_m[6];

	inv[8] = m_m[4] * m_m[9] * m_m[15] -
	m_m[4] * m_m[11] * m_m[13] -
	m_m[8] * m_m[5] * m_m[15] +
	m_m[8] * m_m[7] * m_m[13] +
	m_m[12] * m_m[5] * m_m[11] -
	m_m[12] * m_m[7] * m_m[9];

	inv[9] = -m_m[0] * m_m[9] * m_m[15] +
	m_m[0] * m_m[11] * m_m[13] +
	m_m[8] * m_m[1] * m_m[15] -
	m_m[8] * m_m[3] * m_m[13] -
	m_m[12] * m_m[1] * m_m[11] +
	m_m[12] * m_m[3] * m_m[9];

	inv[10] = m_m[0] * m_m[5] * m_m[15] -
	m_m[0] * m_m[7] * m_m[13] -
	m_m[4] * m_m[1] * m_m[15] +
	m_m[4] * m_m[3] * m_m[13] +
	m_m[12] * m_m[1] * m_m[7] -
	m_m[12] * m_m[3] * m_m[5];

	inv[11] = -m_m[0] * m_m[5] * m_m[11] +
	m_m[0] * m_m[7] * m_m[9] +
	m_m[4] * m_m[1] * m_m[11] -
	m_m[4] * m_m[3] * m_m[9] -
	m_m[8] * m_m[1] * m_m[7] +
	m_m[8] * m_m[3] * m_m[5];


	inv[12] = -m_m[4] * m_m[9] * m_m[14] +
	m_m[4] * m_m[10] * m_m[13] +
	m_m[8] * m_m[5] * m_m[14] -
	m_m[8] * m_m[6] * m_m[13] -
	m_m[12] * m_m[5] * m_m[10] +
	m_m[12] * m_m[6] * m_m[9];


	inv[13] = m_m[0] * m_m[9] * m_m[14] -
	m_m[0] * m_m[10] * m_m[13] -
	m_m[8] * m_m[1] * m_m[14] +
	m_m[8] * m_m[2] * m_m[13] +
	m_m[12] * m_m[1] * m_m[10] -
	m_m[12] * m_m[2] * m_m[9];


	inv[14] = -m_m[0] * m_m[5] * m_m[14] +
	m_m[0] * m_m[6] * m_m[13] +
	m_m[4] * m_m[1] * m_m[14] -
	m_m[4] * m_m[2] * m_m[13] -
	m_m[12] * m_m[1] * m_m[6] +
	m_m[12] * m_m[2] * m_m[5];

	inv[15] = m_m[0] * m_m[5] * m_m[10] -
	m_m[0] * m_m[6] * m_m[9] -
	m_m[4] * m_m[1] * m_m[10] +
	m_m[4] * m_m[2] * m_m[9] +
	m_m[8] * m_m[1] * m_m[6] -
	m_m[8] * m_m[2] * m_m[5];

	det = m_m[0] * inv[0] + m_m[1] * inv[4] + m_m[2] * inv[8] + m_m[3] * inv[12];

	if (det == 0)
	return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
	invOut[i] = inv[i] * det;

	*/
	float minor(float* minorMatrix) const
	{
		return
			(minorMatrix[0] * minorMatrix[4] * minorMatrix[8]) +
			(minorMatrix[1] * minorMatrix[5] * minorMatrix[6]) +
			(minorMatrix[2] * minorMatrix[3] * minorMatrix[7])
			-
			(minorMatrix[2] * minorMatrix[4] * minorMatrix[6]) -
			(minorMatrix[1] * minorMatrix[3] * minorMatrix[8]) -
			(minorMatrix[0] * minorMatrix[5] * minorMatrix[7]);

	}

	RevMatrix operator/(float scalar) const
	{
		RevMatrix returnMatrix;
		for (INT32 index = 0; index < ARRAYSIZE(m_m); index++)
		{
			returnMatrix.m_m[index] = m_m[index] / scalar;
		}

		return returnMatrix;
	}

	float Determant() const
	{

		float detPos =
			m_m[0] * m_m[5] * m_m[10] * m_m[15]
			+ m_m[1] * m_m[6] * m_m[11] * m_m[12]
			+ m_m[2] * m_m[7] * m_m[8] * m_m[13]
			+ m_m[3] * m_m[4] * m_m[9] * m_m[14];
		float detNeg =
			m_m[3] * m_m[6] * m_m[9] * m_m[12]
			+ m_m[2] * m_m[5] * m_m[8] * m_m[15]
			+ m_m[1] * m_m[4] * m_m[11] * m_m[14]
			+ m_m[0] * m_m[7] * m_m[10] * m_m[13];
		return detPos - detNeg;
	}

	RevMatrix Inverse() const
	{


		// transpose 3x3, we know m03 = m13 = m23 = 0
		/*
		RevMatrix r;
		__m128 t0 = VecShuffle_0101(mVec[0], mVec[1]); // 00, 01, 10, 11
		__m128 t1 = VecShuffle_2323(mVec[0], mVec[1]); // 02, 03, 12, 13
		r.mVec[0] = VecShuffle(t0, mVec[2], 0, 2, 0, 3); // 00, 10, 20, 23(=0)
		r.mVec[1] = VecShuffle(t0, mVec[2], 1, 3, 1, 3); // 01, 11, 21, 23(=0)
		r.mVec[2] = VecShuffle(t1, mVec[2], 0, 2, 2, 3); // 02, 12, 22, 23(=0)

		// last line
		r.mVec[3] = _mm_mul_ps(r.mVec[0], VecSwizzle1(mVec[3], 0));
		r.mVec[3] = _mm_add_ps(r.mVec[3], _mm_mul_ps(r.mVec[1], VecSwizzle1(mVec[3], 1)));
		r.mVec[3] = _mm_add_ps(r.mVec[3], _mm_mul_ps(r.mVec[2], VecSwizzle1(mVec[3], 2)));
		r.mVec[3] = _mm_sub_ps(_mm_setr_ps(0.f, 0.f, 0.f, 1.f), r.mVec[3]);
		*/

		float cofactor[16];
		{
			float minorMatrix[] = { m_m[5], m_m[6], m_m[7], m_m[9], m_m[10], m_m[11], m_m[13], m_m[14], m_m[15] };
			cofactor[0] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[4], m_m[6], m_m[7], m_m[8], m_m[10], m_m[11], m_m[12], m_m[14], m_m[15] };
			cofactor[1] = minor(&minorMatrix[0]);
		}
		{
			float minorMatrix[] = { m_m[4], m_m[5], m_m[7], m_m[8], m_m[9],  m_m[11], m_m[12], m_m[13], m_m[15] };
			cofactor[2] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[4], m_m[5], m_m[6], m_m[8], m_m[9],  m_m[10], m_m[12], m_m[13], m_m[14] };
			cofactor[3] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[1], m_m[2], m_m[3], m_m[9], m_m[10], m_m[11], m_m[13], m_m[14], m_m[15] };
			cofactor[4] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[2], m_m[3], m_m[8], m_m[10], m_m[11], m_m[12], m_m[14], m_m[15] };
			cofactor[5] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[1], m_m[3], m_m[8], m_m[9], m_m[11], m_m[12], m_m[13], m_m[15] };
			cofactor[6] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[1], m_m[2], m_m[8], m_m[9], m_m[10], m_m[12], m_m[13], m_m[14] };
			cofactor[7] = minor(&minorMatrix[0]);
		}


		{
			float minorMatrix[] = { m_m[1], m_m[2], m_m[3], m_m[5], m_m[6], m_m[7], m_m[13], m_m[14], m_m[15] };
			cofactor[8] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[2], m_m[3], m_m[4], m_m[6], m_m[7], m_m[12], m_m[14], m_m[15] };
			cofactor[9] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[1], m_m[3], m_m[4], m_m[5], m_m[7], m_m[12], m_m[13], m_m[15] };
			cofactor[10] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[1], m_m[2], m_m[4], m_m[5], m_m[6], m_m[12], m_m[13], m_m[14] };
			cofactor[11] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[1], m_m[2], m_m[3], m_m[5], m_m[6], m_m[7], m_m[9], m_m[10], m_m[11] };
			cofactor[12] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[2], m_m[3], m_m[4], m_m[6], m_m[7], m_m[8], m_m[10], m_m[11] };
			cofactor[13] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[1], m_m[3], m_m[4], m_m[5], m_m[7], m_m[8], m_m[9], m_m[11] };
			cofactor[14] = minor(&minorMatrix[0]);
		}

		{
			float minorMatrix[] = { m_m[0], m_m[1], m_m[2], m_m[4], m_m[5], m_m[6], m_m[8], m_m[9], m_m[10] };
			cofactor[15] = minor(&minorMatrix[0]);
		}


		float positiveNumber[] =
		{
			1, -1, 1, -1,
			-1, 1, -1, 1,
			1, -1, 1, -1,
			- 1, 1, -1, 1

		};
		for (int index = 0; index < ARRAYSIZE(positiveNumber); index++)
		{
			cofactor[index] = positiveNumber[index] * cofactor[index];
		}

		RevMatrix m(&cofactor[0]);
		m = m.Transpose();


		//pick row, multiply by cofactor
		float det = m_m[0] * m.m_m[0] + m_m[1] * m.m_m[4] + m_m[2] * m.m_m[8] + m_m[3] * m.m_m[12];
		RevMatrix inverse = m / det;
		return inverse;


		/*float inv[16];
		inv[0] = m_m[5] * m_m[10] * m_m[15] -
		m_m[5] * m_m[11] * m_m[14] -
		m_m[9] * m_m[6] * m_m[15] +
		m_m[9] * m_m[7] * m_m[14] +
		m_m[13] * m_m[6] * m_m[11] -
		m_m[13] * m_m[7] * m_m[10];

		inv[1] = -m_m[1] * m_m[10] * m_m[15] +
		m_m[1] * m_m[11] * m_m[14] +
		m_m[9] * m_m[2] * m_m[15] -
		m_m[9] * m_m[3] * m_m[14] -
		m_m[13] * m_m[2] * m_m[11] +
		m_m[13] * m_m[3] * m_m[10];

		inv[2] = m_m[1] * m_m[6] * m_m[15] -
		m_m[1] * m_m[7] * m_m[14] -
		m_m[5] * m_m[2] * m_m[15] +
		m_m[5] * m_m[3] * m_m[14] +
		m_m[13] * m_m[2] * m_m[7] -
		m_m[13] * m_m[3] * m_m[6];


		inv[3] = -m_m[1] * m_m[6] * m_m[11] +
		m_m[1] * m_m[7] * m_m[10] +
		m_m[5] * m_m[2] * m_m[11] -
		m_m[5] * m_m[3] * m_m[10] -
		m_m[9] * m_m[2] * m_m[7] +
		m_m[9] * m_m[3] * m_m[6];

		inv[4] = -m_m[4] * m_m[10] * m_m[15] +
		m_m[4] * m_m[11] * m_m[14] +
		m_m[8] * m_m[6] * m_m[15] -
		m_m[8] * m_m[7] * m_m[14] -
		m_m[12] * m_m[6] * m_m[11] +
		m_m[12] * m_m[7] * m_m[10];
		inv[5] = m_m[0] * m_m[10] * m_m[15] -
		m_m[0] * m_m[11] * m_m[14] -
		m_m[8] * m_m[2] * m_m[15] +
		m_m[8] * m_m[3] * m_m[14] +
		m_m[12] * m_m[2] * m_m[11] -
		m_m[12] * m_m[3] * m_m[10];
		inv[6] = -m_m[0] * m_m[6] * m_m[15] +
		m_m[0] * m_m[7] * m_m[14] +
		m_m[4] * m_m[2] * m_m[15] -
		m_m[4] * m_m[3] * m_m[14] -
		m_m[12] * m_m[2] * m_m[7] +
		m_m[12] * m_m[3] * m_m[6];

		inv[7] = m_m[0] * m_m[6] * m_m[11] -
		m_m[0] * m_m[7] * m_m[10] -
		m_m[4] * m_m[2] * m_m[11] +
		m_m[4] * m_m[3] * m_m[10] +
		m_m[8] * m_m[2] * m_m[7] -
		m_m[8] * m_m[3] * m_m[6];

		inv[8] = m_m[4] * m_m[9] * m_m[15] -
		m_m[4] * m_m[11] * m_m[13] -
		m_m[8] * m_m[5] * m_m[15] +
		m_m[8] * m_m[7] * m_m[13] +
		m_m[12] * m_m[5] * m_m[11] -
		m_m[12] * m_m[7] * m_m[9];

		inv[9] = -m_m[0] * m_m[9] * m_m[15] +
		m_m[0] * m_m[11] * m_m[13] +
		m_m[8] * m_m[1] * m_m[15] -
		m_m[8] * m_m[3] * m_m[13] -
		m_m[12] * m_m[1] * m_m[11] +
		m_m[12] * m_m[3] * m_m[9];

		inv[10] = m_m[0] * m_m[5] * m_m[15] -
		m_m[0] * m_m[7] * m_m[13] -
		m_m[4] * m_m[1] * m_m[15] +
		m_m[4] * m_m[3] * m_m[13] +
		m_m[12] * m_m[1] * m_m[7] -
		m_m[12] * m_m[3] * m_m[5];

		inv[11] = -m_m[0] * m_m[5] * m_m[11] +
		m_m[0] * m_m[7] * m_m[9] +
		m_m[4] * m_m[1] * m_m[11] -
		m_m[4] * m_m[3] * m_m[9] -
		m_m[8] * m_m[1] * m_m[7] +
		m_m[8] * m_m[3] * m_m[5];


		inv[12] = -m_m[4] * m_m[9] * m_m[14] +
		m_m[4] * m_m[10] * m_m[13] +
		m_m[8] * m_m[5] * m_m[14] -
		m_m[8] * m_m[6] * m_m[13] -
		m_m[12] * m_m[5] * m_m[10] +
		m_m[12] * m_m[6] * m_m[9];


		inv[13] = m_m[0] * m_m[9] * m_m[14] -
		m_m[0] * m_m[10] * m_m[13] -
		m_m[8] * m_m[1] * m_m[14] +
		m_m[8] * m_m[2] * m_m[13] +
		m_m[12] * m_m[1] * m_m[10] -
		m_m[12] * m_m[2] * m_m[9];


		inv[14] = -m_m[0] * m_m[5] * m_m[14] +
		m_m[0] * m_m[6] * m_m[13] +
		m_m[4] * m_m[1] * m_m[14] -
		m_m[4] * m_m[2] * m_m[13] -
		m_m[12] * m_m[1] * m_m[6] +
		m_m[12] * m_m[2] * m_m[5];

		inv[15] = m_m[0] * m_m[5] * m_m[10] -
		m_m[0] * m_m[6] * m_m[9] -
		m_m[4] * m_m[1] * m_m[10] +
		m_m[4] * m_m[2] * m_m[9] +
		m_m[8] * m_m[1] * m_m[6] -
		m_m[8] * m_m[2] * m_m[5];

		float det = m_m[0] * inv[0] + m_m[1] * inv[4] + m_m[2] * inv[8] + m_m[3] * inv[12];

		assert(det != 0);

		det = 1.0 / det;
		RevMatrix returnMatrix;

		for (int i = 0; i < 16; i++)
			returnMatrix.m_m[i] = inv[i] * det;
		return returnMatrix;*/
	}

	float Trace() const
	{
		return m_m[0] + m_m[5] + m_m[10] + m_m[15];
	}

	float TraceRotation() const
	{
		return m_m[0] + m_m[5] + m_m[10];
	}

	uint32_t FindBiggestDiagonalAxis() const
	{
		if (m_m[0] > m_m[5] || m_m[0] > m_m[10])
		{
			return 0;
		}
		else
		if (m_m[5] > m_m[0] || m_m[5] > m_m[10])
		{
			return 1;
		}
		else
		if (m_m[10] > m_m[0] || m_m[10] > m_m[5])
		{
			return 2;
		}

		return 0;
	}

	union
	{
		float m_m[16];
		__m128 mVec[4];
	};
};