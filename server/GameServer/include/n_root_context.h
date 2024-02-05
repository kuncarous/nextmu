#ifndef __N_ROOT_CONTEXT_H__
#define __N_ROOT_CONTEXT_H__

#pragma once

#include <QObject>
#include <QAbstractListModel>
#include <QList>
#include <QQmlListProperty>

class NConsoleMessage : QObject
{
    Q_OBJECT
    Q_PROPERTY(QString backgroundColor READ getBackgroundColor CONSTANT)
    Q_PROPERTY(QString fontColor READ getFontColor CONSTANT)
    Q_PROPERTY(QString selectedColor READ getSelectedColor CONSTANT)
    Q_PROPERTY(QString highlightColor READ getHighlightColor CONSTANT)
    Q_PROPERTY(QString message READ getMessage CONSTANT)

public:
    NConsoleMessage(const NLogMessage &message);

public:
    QString getBackgroundColor() const;
    QString getFontColor() const;
    QString getSelectedColor() const;
    QString getHighlightColor() const;
    QString getMessage() const;

private:
    QString BackgroundColor;
    QString FontColor;
    QString SelectedColor;
    QString HighlightColor;
    QString Message;
};

class NRootContext : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString performanceStatistics READ getPerformanceStatistics NOTIFY performanceStatisticsChanged)
    Q_PROPERTY(QQmlListProperty<NConsoleMessage> messages READ getQmlMessages NOTIFY messagesChanged)

public:
    virtual ~NRootContext() override;

public:
    QString getPerformanceStatistics() const;
    void setPerformanceStatistics(const QString &value);

    QList<NConsoleMessage*> &getMessages();
    QQmlListProperty<NConsoleMessage> getQmlMessages();

signals:
    void performanceStatisticsChanged();
    void messagesChanged();

private:
    QString PerformanceStatistics;
    QList<NConsoleMessage*> Messages;
};

#endif
