#pragma once

class RevInstance;
class RevCamera;
class RevObject;

class RevController
{
public:

	void Initialize();
	void SetObject(RevObject* inInstance);
	void Update(float deltaTime);
	void Draw(float deltaTime, struct RevModelFrameRender& renderEntry);

	RevCamera* m_camera = nullptr;
	RevObject* m_object = nullptr;

private:

	void RefreshInitialization();

;};


