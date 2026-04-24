#include "gamewindow.h"

#include "narrative/samplestory.h"
#include "narrative/storyloader.h"
#include "ui/dialoguepanel.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent),
      m_backgroundWidget(nullptr),
      m_dialoguePanel(nullptr),
      m_headerLabel(nullptr),
      m_narrativeEngine(new NarrativeEngine(this))
{
    buildUi();

    connect(m_narrativeEngine, &NarrativeEngine::stateChanged, this, &GameWindow::applyNarrativeState);

    loadStoryContent();
    m_narrativeEngine->start();
}

void GameWindow::setSpeaker(const QString &speaker)
{
    m_dialoguePanel->setSpeaker(speaker);
}

void GameWindow::clearSpeaker()
{
    m_dialoguePanel->clearSpeaker();
}

void GameWindow::setText(const QString &text)
{
    m_dialoguePanel->setText(text);
}

void GameWindow::setContinueVisible(bool visible)
{
    m_dialoguePanel->setContinueVisible(visible);
}

void GameWindow::setDialogueVisible(bool visible)
{
    m_dialoguePanel->setDialogueVisible(visible);
}

void GameWindow::setBackgroundStyle(BackgroundStyle style)
{
    m_backgroundWidget->setBackgroundStyle(style);
}

void GameWindow::setInteractionItems(const InteractionItems &items)
{
    m_dialoguePanel->setInteractionItems(items);
}

void GameWindow::setInteractionMode(InteractionMode mode)
{
    m_dialoguePanel->setInteractionMode(mode);
}

void GameWindow::showNarrationMode(const QString &text, bool showContinue)
{
    setDialogueVisible(true);
    clearSpeaker();
    setText(text);
    setInteractionMode(InteractionMode::None);
    setInteractionItems({});
    m_dialoguePanel->setInteractionVisible(false);
    setContinueVisible(showContinue);
}

void GameWindow::showDialogueMode(const QString &speaker, const QString &text, bool showContinue)
{
    setDialogueVisible(true);
    setSpeaker(speaker);
    setText(text);
    setInteractionMode(InteractionMode::None);
    setInteractionItems({});
    m_dialoguePanel->setInteractionVisible(false);
    setContinueVisible(showContinue);
}

void GameWindow::showInteractionMode(const QString &speaker,
                                     const QString &text,
                                     InteractionMode mode,
                                     const InteractionItems &items,
                                     bool showContinue)
{
    setDialogueVisible(true);

    if (speaker.trimmed().isEmpty()) {
        clearSpeaker();
    } else {
        setSpeaker(speaker);
    }

    setText(text);
    setInteractionMode(mode);
    setInteractionItems(items);
    m_dialoguePanel->setInteractionVisible(!items.isEmpty());
    setContinueVisible(showContinue);

    if (!items.isEmpty()) {
        m_dialoguePanel->focusFirstInteraction();
    }
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return
         || event->key() == Qt::Key_Enter
         || event->key() == Qt::Key_Space)
        && m_dialoguePanel->isContinueVisible()) {
        advanceNarrative();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void GameWindow::advanceNarrative()
{
    if (!m_dialoguePanel->isContinueVisible()) {
        return;
    }

    m_narrativeEngine->continueNarrative();
}

void GameWindow::handleInteractionTriggered(const QString &id)
{
    if (!m_dialoguePanel->hasVisibleInteractions()) {
        return;
    }

    m_narrativeEngine->handleInteraction(id);
}

void GameWindow::applyNarrativeState()
{
    const NarrativeViewState state = m_narrativeEngine->currentViewState();

    setHeaderText(state.header);
    setBackgroundStyle(state.backgroundStyle);
    setDialogueVisible(true);

    if (state.displayMode == NarrativeDisplayMode::Interaction && !state.interactionItems.isEmpty()) {
        showInteractionMode(state.showSpeaker ? state.speaker : QString(),
                            state.text,
                            state.interactionMode,
                            state.interactionItems,
                            state.showContinue);
        return;
    }

    if (state.showSpeaker) {
        showDialogueMode(state.speaker, state.text, state.showContinue);
    } else {
        showNarrationMode(state.text, state.showContinue);
    }
}

void GameWindow::buildUi()
{
    resize(1280, 720);
    setMinimumSize(960, 600);
    setWindowTitle(QStringLiteral("第一天 - 最小数据驱动原型"));

    m_backgroundWidget = new BackgroundWidget(this);
    setCentralWidget(m_backgroundWidget);

    auto *layout = new QVBoxLayout(m_backgroundWidget);
    layout->setContentsMargins(40, 28, 40, 28);
    layout->setSpacing(16);

    m_headerLabel = new QLabel(m_backgroundWidget);
    m_headerLabel->setStyleSheet(
        "QLabel {"
        "    color: rgba(255, 255, 255, 180);"
        "    font-size: 14px;"
        "    letter-spacing: 1px;"
        "    font-weight: 600;"
        "}"
    );
    layout->addWidget(m_headerLabel, 0, Qt::AlignLeft | Qt::AlignTop);

    layout->addStretch();

    m_dialoguePanel = new DialoguePanel(m_backgroundWidget);
    layout->addWidget(m_dialoguePanel, 0, Qt::AlignBottom);

    connect(m_dialoguePanel, &DialoguePanel::continueRequested, this, &GameWindow::advanceNarrative);
    connect(m_dialoguePanel, &DialoguePanel::interactionTriggered, this, &GameWindow::handleInteractionTriggered);
}

void GameWindow::loadStoryContent()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidatePaths = {
        QDir(appDir).filePath(QStringLiteral("data/story_main.json")),
        QDir(appDir).filePath(QStringLiteral("../data/story_main.json"))
    };

    for (const QString &candidatePath : candidatePaths) {
        const StoryLoadResult result = StoryLoader::loadFromFile(candidatePath);
        if (!result.success) {
            qWarning().noquote() << QStringLiteral("[StoryLoader] Failed to load %1: %2")
                                    .arg(candidatePath, result.errorMessage);
            continue;
        }

        qInfo().noquote() << QStringLiteral("[StoryLoader] Loaded %1 (%2 scenes, %3 interactions)")
                              .arg(result.filePath)
                              .arg(result.sceneCount)
                              .arg(result.interactionCount);

        m_narrativeEngine->loadScenes(result.scenes, result.startSceneId);
        return;
    }

    qWarning().noquote() << QStringLiteral("[StoryLoader] Falling back to built-in sample story.");
    m_narrativeEngine->loadScenes(createSampleStory(), sampleStoryStartSceneId());
}

void GameWindow::setHeaderText(const QString &text)
{
    m_headerLabel->setText(text);
}
