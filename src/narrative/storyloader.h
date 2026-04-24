#ifndef STORYLOADER_H
#define STORYLOADER_H

#include "scene.h"

#include <QString>

struct StoryLoadResult
{
    bool success = false;
    QString filePath;
    QString startSceneId;
    NarrativeSceneMap scenes;
    QString errorMessage;
    int sceneCount = 0;
    int interactionCount = 0;
};

class StoryLoader
{
public:
    static StoryLoadResult loadFromFile(const QString &filePath);
};

#endif // STORYLOADER_H
