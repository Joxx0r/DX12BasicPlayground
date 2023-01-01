#pragma once

#include <intrin.h>
#include <math.h>

class RevVector2
{
public:
	RevVector2() = default;
	RevVector2(float x, float y)
	{
		m_v[0] = x;
		m_v[1] = y;
	}


	float& operator[](int index)
	{
		return m_v[index];
	}

	float operator[](int index) const
	{
		return m_v[index];
	}

	float X() const
	{
		return m_v[0];
	}
	float Y() const
	{
		return m_v[1];
	}

	float m_v[2];

	static RevVector2 ZeroVector;

};
class RevVector3
{
public:
	RevVector3() = default;
	RevVector3(float x, float y, float z)
	{
		m_v[0] = x;
		m_v[1] = y;
		m_v[2] = z;
	}

	RevVector3(float* data)
	{
		memcpy(m_v, data, sizeof(RevVector3));
	}
	
	float& operator[](int index)
	{
		return m_v[index];
	}

	float operator[](int index) const
	{
		return m_v[index];
	}

	float LengthSquared() const
	{
		float returnLength = 0.0f;
		for (INT32 index = 0; index < ARRAYSIZE(m_v); index++)
		{
			returnLength += m_v[index] * m_v[index];
		}
		return returnLength;
	}

	float Length() const
	{
		return sqrtf(LengthSquared());
	}

	void NormalizeSelf()
	{
		const float desiredLength = Length();
		if (desiredLength > 0.f)
		{
			for (INT32 index = 0; index < ARRAYSIZE(m_v); index++)
			{
				m_v[index] = m_v[index] / desiredLength;
			}
		}
	}

	RevVector3 Normalize() const
	{
		const float desiredLength = Length();
		RevVector3 normalizeV = *this;
		if (desiredLength > 0.f)
		{
			for (INT32 index = 0; index < ARRAYSIZE(m_v); index++)
			{
				normalizeV.m_v[index] = m_v[index] / desiredLength;
			}
		}
		return normalizeV;
	}

	float Dot(const RevVector3& vector) const
	{
		float dot = 0.0f;
		for (INT32 index = 0; index < 3; index++)
		{
			dot += m_v[index] * vector[index];
		}

		return dot;
	}

	RevVector3 operator*(const RevVector3& vector) const
	{
		RevVector3 returnVector;
		for (INT32 index = 0; index < 3; index++)
		{
			returnVector[index] = m_v[index] * vector[index];
		}
		return returnVector;
	}

	RevVector3 operator-(const RevVector3& vector) const
	{
		RevVector3 returnVector = *this;
		for (uint32_t index = 0; index < ARRAYSIZE(m_v); index++)
		{
			returnVector[index] -= vector.m_v[index];
		}
		return returnVector;
	}

	RevVector3 operator-() const
	{
		RevVector3 returnVector;
		returnVector.m_v[0] = -m_v[0];
		returnVector.m_v[1] = -m_v[1];
		returnVector.m_v[2] = -m_v[2];
		return returnVector;
	}

	RevVector3 operator*(float scalar) const
	{
		RevVector3 returnVector;
		for (uint32_t index = 0; index < ARRAYSIZE(m_v); index++)
		{
			returnVector[index] = m_v[index] * scalar;
		}

		return returnVector;
	}

	RevVector3 operator/(float scalar) const
	{
		RevVector3 returnVector = *this;
		for (uint32_t index = 0; index < ARRAYSIZE(m_v); index++)
		{
			returnVector[index] /= scalar;
		}
		return returnVector;
	}

	RevVector3 operator+(const RevVector3& vector) const
	{
		RevVector3 returnVector = *this;
		for (uint32_t index = 0; index < ARRAYSIZE(m_v); index++)
		{
			returnVector[index] += vector.m_v[index];
		}
		return returnVector;
	}

	bool operator==(const RevVector3& vector) const
	{
		for (uint32_t index = 0; index < ARRAYSIZE(m_v); index++)
		{
			if(m_v[index] != vector.m_v[index])
			{
				return false;
			}
		}
		return true;
	}

	
	void operator-=(const RevVector3 vector)
	{
		*this = *this - vector;
	}
	void operator+=(const RevVector3 vector)
	{
		*this = *this + vector;
	}

	float X() const
	{
		return m_v[0];
	}
	float Y() const
	{
		return m_v[1];
	}
	float Z() const
	{
		return m_v[2];
	}

	RevVector3 CrossProduct(const RevVector3& otherCrossProduct) const
	{
		RevVector3 returnVector;
		returnVector[0] = Y() * otherCrossProduct.Z() - Z() * otherCrossProduct.Y();
		returnVector[1] = Z() * otherCrossProduct.X() - X() * otherCrossProduct.Z();
		returnVector[2] = X() * otherCrossProduct.Y() - Y() * otherCrossProduct.X();
		return returnVector;
	}

	__m128 LoadInteristicFloat()
	{
		return _mm_setr_ps(m_v[0], m_v[1], m_v[2], 0);
	}
	
	float m_v[3];

	static RevVector3 ZeroVector;

};

class RevVector
{
public:

	RevVector(float x, float y, float z)
	{
		m_v[0] = x;
		m_v[1] = y;
		m_v[2] = z;
		m_v[3] = 1.0f;
	}

	RevVector(float x, float y, float z, float w)
	{
		m_v[0] = x;
		m_v[1] = y;
		m_v[2] = z;
		m_v[3] = w;
	}


	RevVector()
	{
		memset(this, 0, sizeof(RevVector));
	}

	float Dot(const RevVector& vector) const
	{
		return X() * vector.X() + Y() * vector.Y() + Z()* vector.Z();
	}


	float& operator[](int index)
	{
		return m_v[index];
	}

	float operator[](int index) const
	{
		return m_v[index];
	}

	RevVector CrossProduct(const RevVector& otherVector) const
	{
		RevVector returnVector;
		returnVector[0] = Y() * otherVector.Z() - Z() * otherVector.Y();
		returnVector[1] = Z() * otherVector.X() - X() * otherVector.Z();
		returnVector[2] = X() * otherVector.Y() - Y() * otherVector.X();
		returnVector[3] = 0.0f;
		return returnVector;
	}


	float Length()
	{
		float returnLength = 0.0f;
		for (INT32 index = 0; index < 3; index++)
		{
			returnLength += m_v[index] * m_v[index];
		}
		return sqrtf(returnLength);
	}

	void Normalize()
	{
		const float desiredLength = Length();
		if (desiredLength > 0.f)
		{
			for (INT32 index = 0; index <3; index++)
			{
				m_v[index] = m_v[index] / desiredLength;
			}
			m_v[3] = 1.0f;
		}
	}

	RevVector operator-(const RevVector& vector)
	{
		RevVector returnVector;
		for (uint32_t index = 0; index < 3; index++)
		{
			returnVector.m_v[index] = m_v[index] - vector.m_v[index];
		}
		returnVector[3] = 1.0f;
		return returnVector;
	}

	RevVector operator*(const RevVector3& vector) const
	{
		RevVector returnVector = {};
		for (INT32 index = 0; index < 3; index++)
		{
			returnVector[index] = m_v[index] * vector[index];
		}
		returnVector.m_v[3] = m_v[3];
		return returnVector;
	}

	RevVector operator+=(const RevVector3& vector) const
	{
		RevVector returnVector = {};
		for (INT32 index = 0; index < 3; index++)
		{
			returnVector[index] = m_v[index] + vector[index];
		}
		returnVector.m_v[3] = m_v[3];
		return returnVector;
	}

	float X() const
	{
		return m_v[0];
	}
	float Y() const
	{
		return m_v[1];
	}
	float Z() const
	{
		return m_v[2];
	}

	float W() const
	{
		return m_v[3];
	}

	float m_v[4];

	static RevVector ZeroVector;
};

