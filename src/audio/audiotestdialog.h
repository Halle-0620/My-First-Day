#ifndef AUDIOTESTDIALOG_H
#define AUDIOTESTDIALOG_H

#include "audiomanager.h"

#include <QDialog>

class AudioManager;
class QVBoxLayout;

class AudioTestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AudioTestDialog(AudioManager *audioManager, QWidget *parent = nullptr);

private:
    void buildUi();
    void addLoopingSection(QVBoxLayout *rootLayout, const QString &title, AudioCategory category, bool isBgm);
    void addOneShotSection(QVBoxLayout *rootLayout, const QString &title, AudioCategory category);

    AudioManager *m_audioManager;
};

#endif // AUDIOTESTDIALOG_H
