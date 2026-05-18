#include "audiotestdialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

AudioTestDialog::AudioTestDialog(AudioManager *audioManager, QWidget *parent)
    : QDialog(parent),
      m_audioManager(audioManager)
{
    buildUi();
}

void AudioTestDialog::buildUi()
{
    setWindowTitle(QString::fromUtf8(u8"音频测试"));
    resize(760, 560);

    auto *rootLayout = new QVBoxLayout(this);

    auto *hintLabel = new QLabel(QString::fromUtf8(
        u8"临时测试入口：用于验证 BGM / AMB 的循环、切换、淡入淡出，以及 SFX / UI / EMO 的一次性播放。"), this);
    hintLabel->setWordWrap(true);
    rootLayout->addWidget(hintLabel);

    auto *manifestLabel = new QLabel(this);
    const QString manifestPath = m_audioManager ? m_audioManager->loadedManifestPath() : QString();
    manifestLabel->setText(QString::fromUtf8(u8"清单：%1").arg(manifestPath.isEmpty() ? QString::fromUtf8(u8"未加载") : manifestPath));
    manifestLabel->setWordWrap(true);
    rootLayout->addWidget(manifestLabel);

    auto *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    auto *content = new QWidget(scrollArea);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(8, 8, 8, 8);
    contentLayout->setSpacing(12);

    addLoopingSection(contentLayout, QStringLiteral("BGM"), AudioCategory::Bgm, true);
    addLoopingSection(contentLayout, QStringLiteral("AMB"), AudioCategory::Amb, false);
    addOneShotSection(contentLayout, QStringLiteral("SFX"), AudioCategory::Sfx);
    addOneShotSection(contentLayout, QStringLiteral("UI"), AudioCategory::Ui);
    addOneShotSection(contentLayout, QStringLiteral("EMO"), AudioCategory::Emo);
    contentLayout->addStretch();

    scrollArea->setWidget(content);
    rootLayout->addWidget(scrollArea, 1);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    rootLayout->addWidget(buttonBox);
}

void AudioTestDialog::addLoopingSection(QVBoxLayout *rootLayout,
                                        const QString &title,
                                        AudioCategory category,
                                        bool isBgm)
{
    auto *groupBox = new QGroupBox(title, this);
    auto *layout = new QGridLayout(groupBox);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(8);

    const QStringList ids = m_audioManager ? m_audioManager->availableIds(category) : QStringList();
    int row = 0;
    for (const QString &id : ids) {
        auto *label = new QLabel(id, groupBox);
        auto *button = new QPushButton(QString::fromUtf8(u8"播放 / 切换"), groupBox);
        connect(button, &QPushButton::clicked, this, [this, id, isBgm]() {
            if (!m_audioManager) {
                return;
            }
            if (isBgm) {
                m_audioManager->playBgm(id);
            } else {
                m_audioManager->playAmb(id);
            }
        });

        layout->addWidget(label, row, 0);
        layout->addWidget(button, row, 1);
        ++row;
    }

    auto *stopButton = new QPushButton(QString::fromUtf8(u8"停止"), groupBox);
    connect(stopButton, &QPushButton::clicked, this, [this, isBgm]() {
        if (!m_audioManager) {
            return;
        }
        if (isBgm) {
            m_audioManager->stopBgm();
        } else {
            m_audioManager->stopAmb();
        }
    });
    layout->addWidget(stopButton, row, 1);

    rootLayout->addWidget(groupBox);
}

void AudioTestDialog::addOneShotSection(QVBoxLayout *rootLayout, const QString &title, AudioCategory category)
{
    auto *groupBox = new QGroupBox(title, this);
    auto *layout = new QGridLayout(groupBox);
    layout->setHorizontalSpacing(8);
    layout->setVerticalSpacing(8);

    const QStringList ids = m_audioManager ? m_audioManager->availableIds(category) : QStringList();
    int row = 0;
    for (const QString &id : ids) {
        auto *label = new QLabel(id, groupBox);
        auto *button = new QPushButton(QString::fromUtf8(u8"播放一次"), groupBox);
        connect(button, &QPushButton::clicked, this, [this, category, id]() {
            if (!m_audioManager) {
                return;
            }
            switch (category) {
            case AudioCategory::Sfx:
                m_audioManager->playSfx(id);
                break;
            case AudioCategory::Ui:
                m_audioManager->playUi(id);
                break;
            case AudioCategory::Emo:
                m_audioManager->playEmo(id);
                break;
            case AudioCategory::Bgm:
            case AudioCategory::Amb:
                break;
            }
        });

        layout->addWidget(label, row, 0);
        layout->addWidget(button, row, 1);
        ++row;
    }

    rootLayout->addWidget(groupBox);
}
