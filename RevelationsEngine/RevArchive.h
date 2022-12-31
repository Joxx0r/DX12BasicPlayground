#pragma once

#define REV_FILE_VERSION 0
class RevArchive
{
public:

	virtual void Alloc(uint32_t size)
	{
		unsigned char* newSize = (unsigned char*)malloc(m_currentAllocatedSize + size);
		if (m_byteArray)
		{
			memcpy(newSize, m_byteArray, m_currentAllocatedSize);
			delete m_byteArray;
		}

		m_byteArray = newSize;
		m_currentAllocatedSize += size;
	}

	void Delete()
	{
		delete m_byteArray;
	}

	unsigned char* GetMemory(uint32_t size)
	{
		if (m_memoryPointer + size > (m_currentAllocatedSize))
		{
			Alloc(256 + size);
		}

		unsigned char* memoryStart = &m_byteArray[m_memoryPointer];
		m_memoryPointer += size;
		return memoryStart;
	}

	uint32_t Tell() { return m_memoryPointer;  }

	virtual bool IsLoading() = 0;
	virtual bool IsSaving() = 0;
	virtual void Serialize(void* inputData, uint32_t size) = 0;
	virtual void SerializeBinaryData(void** inputData, uint32_t size) = 0;


	template<class T>
	void operator<<(T& element)
	{
		Serialize((void*)&element, sizeof(T));
	}
	template<>
	void operator<<(std::string& element)
	{
		uint32_t size = (uint32_t)element.size();
		*this << size;
		if (IsSaving())
		{
			for (uint32_t index = 0; index < size; index++)
			{
				char elem = element.at(index);
				*this << elem;
			}
		}
		else
		{
			for (UINT index = 0; index < size; index++)
			{
				char buffer;
				*this << buffer;
				element.push_back(buffer);
			}
		}
	}
	template<class T>
	void operator<<(std::vector<T>& elements)
	{
		UINT nElements = (UINT)elements.size();
		*this << nElements;

		if (IsSaving())
		{
			for (T& element : elements)
			{
				element.Serialize(*this);
			}
		}
		else
		if (IsLoading())
		{
			for (UINT keyFrameIndex = 0; keyFrameIndex < nElements; keyFrameIndex++)
			{
				T element = {};
				element.Serialize(*this);
				elements.push_back(element);
			}
		}
	}
	unsigned char* m_byteArray = nullptr;

	UINT m_currentAllocatedSize = 0;
	UINT m_memoryPointer = 0;
	UINT m_version;
};


class RevArchiveLoader : public RevArchive
{
public:

	RevArchiveLoader(UINT size)
		:RevArchive()
	{

		unsigned char* newSize = (unsigned char*)malloc(m_currentAllocatedSize + size);
		if (m_byteArray)
		{
			memcpy(newSize, m_byteArray, m_currentAllocatedSize);
			delete m_byteArray;
		}

		m_byteArray = newSize;
		m_currentAllocatedSize += size;
	}


	virtual void Alloc(UINT size)
	{
		REV_ASSERT(0 && "NO ALLOC");
	}

	virtual void Serialize(void* inputData, UINT size);
	virtual void SerializeBinaryData(void** inputData, UINT size);
	
	virtual bool IsLoading() { return true;  }
	virtual bool IsSaving() { return false;  }
};

class RevArchiveSaver : public RevArchive
{
public:

	
	virtual void Serialize(void* inputData, UINT size);

	virtual void SerializeBinaryData(void** inputData, UINT size);

	template<class T>
	void operator<<(const T element)
	{
		Serialize((void*)&element, sizeof(T));
	}

	virtual bool IsLoading() { return false; }
	virtual bool IsSaving() { return true; }
};