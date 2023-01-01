#pragma once

class RevCamera;
class RevInstance;

class RevController
{
public:

	void Initialize();
	void SetInstance();
	void Update(float deltaTime);
	void Draw(float deltaTime, struct RevModelFrameRender& renderEntry);

	RevCamera* m_camera = nullptr;
	RevInstance* m_instance = nullptr;

;};