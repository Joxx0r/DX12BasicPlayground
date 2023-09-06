#pragma once

class RevInstance;
class RevObject
{
public:
    RevObject();
    
    void Initialize(RevWorld* InWorld, const RevMatrix& transform, uint32_t index, const char* modelPath);
    void Update(struct RevFrameResource* InResource, float InDeltaTime);
    void Draw(RevModelFrameRender& param);

    RevWorld* m_world;
    RevInstance* m_instance;
};

