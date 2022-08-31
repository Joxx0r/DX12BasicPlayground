#pragma once

#include "RevVector.h"

class RevQuat
{
public:

	RevQuat()
	{
		m_v = RevVector3(0,0,0);
		m_real = 1.0f;
	}

	RevQuat(float radian, RevVector3 axis)
	{

		//this creates a unit quaternion and can be proven in this way for UNIT quatenerions
		//the length is always 1  with simple ||v||2 + ||real||2 == 1;
		//if we plot the real part as y = cos(x) then we can use trigmetric identities
		//sin(x)2 + cos(x)2 = 1
		//sin(x)2 = 1 - cos(x)2
		//sin(x)2 = 1 - ||real||2
		//which also leads sin(x)2 == ||v||2
		//and then sin(x) == ||v||
		//so then if you think of normalization 
		// n = v / ||v|| 
		// n = v / sin(x)
		// n*sin(x) = v
		//this is how we are able to create this formula and apply the quaternion in these two angles

		//why we multiply by half is that quaternions are primarly used during rotaiton multiplication 
		m_v = axis;
		m_real = radian * 0.5f;
	}

	RevQuat(float x, float y, float z, float w)
	{
		m_v = RevVector3(x,y,z);
		m_real = w;
	}

	RevQuat operator*(const RevQuat& quat)
	{
		//can be rewritten if break down in components(x,y,z,w)
		//which makes it then possible to get vector * matrix form
		//which can also prove the identity 0,0,0,1 and ijk = -1 realtionship
		RevQuat returnQuat;
		returnQuat.m_v = returnQuat.m_v * m_real + m_v * returnQuat.m_real + m_v.CrossProduct(returnQuat.m_v);
		returnQuat.m_real = m_real * quat.m_real - m_v.Dot(returnQuat.m_v);
		return returnQuat;
	}

	RevQuat Conjugate() const
	{
		RevQuat returnQuat;
		returnQuat.m_v = -m_v;
		returnQuat.m_real = m_real;
		return returnQuat;
	}

	RevQuat Inverse() const
	{
		//the regular inverse for a quaternion is as follows
		// q-1 =  q* / ||q||2
		// can easily be shown by adding q to both side
		//This leads to q-1  q to be 1 
		//and qq* / ||q||2 == ( qq* is equal to ||v||2 + real2 which is the magnitude anyway raised by two.
		//as we only deal in unit quaternions the magnitude is 1 which means that the forumla becomes even easier
		//q-1 == q*

		return Conjugate();
	}

	RevQuat Identity()
	{
		//see * operator about explaining how this identity work.
		RevQuat returnQuat;
		returnQuat.m_v = RevVector3(0, 0, 0);
		returnQuat.m_real = m_real;
		return returnQuat;
	}


	RevQuat Normalize() const
	{
		RevQuat returnQuat = *this;

		const float d = returnQuat.Length();
		return returnQuat / d;
	}

	float LengthSquared() const
	{
		//for Unit quatenerions the length is always 1
		//otherwise the formula is sqrtf( powf(||v||, 2) + m_real pow 2 ) 
		return m_v.LengthSquared() + m_real * m_real;
	}

	float Length() const
	{
		return sqrtf(LengthSquared());
	}

	RevQuat operator-(const RevQuat& otherQuat) const
	{
		RevQuat returnQuat;
		returnQuat.m_v = m_v - otherQuat.m_v;
		returnQuat.m_real = m_real - otherQuat.m_real;
		return returnQuat;
	}

	RevQuat operator+(const RevQuat& otherQuat) const
	{
		RevQuat returnQuat;
		returnQuat.m_v = m_v + otherQuat.m_v;
		returnQuat.m_real = m_real + otherQuat.m_real;
		return returnQuat;
	}

	RevQuat operator-() const
	{
		RevQuat returnQuat;
		returnQuat.m_v = -m_v;
		returnQuat.m_real = -m_real;
		return returnQuat;
	}
	RevQuat operator*(float scalar) const
	{
		RevQuat returnQuat;
		returnQuat.m_v = m_v * scalar;
		returnQuat.m_real = m_real * scalar;
		return returnQuat;
	}
	RevQuat operator/(float scalar) const
	{
		RevQuat returnQuat;
		returnQuat.m_v = m_v / scalar;
		returnQuat.m_real = m_real / scalar;
		return returnQuat;
	}

	float Dot(const RevQuat& otherQuat) const
	{
		return m_v.Dot(otherQuat.m_v) + m_real * otherQuat.m_real;
	}


	float& operator[](uint32_t index)
	{
		if (index >= 0 || index < 3)
		{
			return m_v[index];
		}
		else
			if (index == 3)
			{
				return m_real;
			}
			else
			{
				static float invalidOperator = 0.0f;
				assert(0 && "Invalid operator");
				return invalidOperator;
			}
	}

	float operator[](UINT index) const
	{
		if (index >= 0 || index < 3)
		{
			return m_v[index];
		}
		else
			if (index == 3)
			{
				return m_real;
			}
			else
			{
				static float invalidOperator = 0.0f;
				assert(0 && "Invalid operator");
				return invalidOperator;
			}
	}
	RevVector3 m_v;
	float m_real;

};