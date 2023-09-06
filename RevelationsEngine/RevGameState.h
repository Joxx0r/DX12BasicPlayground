#pragma once

class RevObject;
struct RevFrameResource;

class RevGameState
{
public:
    void Initialize();
    void AddGameObject(RevObject* InObject);
    void Update(RevFrameResource* InResource, float InDeltaTime);
    void Draw(RevModelFrameRender& param);
    bool ReplaceObject(UINT index, RevObject* newInstance);
    void Destroy();

    std::vector<RevObject*> m_gameObjects; 

};
