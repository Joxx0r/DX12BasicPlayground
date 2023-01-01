#pragma once


class RevInstance;
class RevCamera;

class RevController
{
public:

	void Initialize();
	void SetInstance(RevInstance* inInstance);
	void Update(float deltaTime);
	void Draw(float deltaTime, struct RevModelFrameRender& renderEntry);

	RevCamera* m_camera = nullptr;
	RevInstance* m_instance = nullptr;

private:

	void RefreshInitialization();

;};


