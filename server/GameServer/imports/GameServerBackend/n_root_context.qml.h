#ifndef __N_ROOT_CONTEXT_H__
#define __N_ROOT_CONTEXT_H__

#pragma once

#include <QString>
#include <QObject>
#include <QAbstractListModel>
#include <QList>
#include <QQmlListProperty>
#include <QtQml>

class NConsoleMessage : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString message READ getMessage WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(QString backgroundColor READ getBackgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QString fontColor READ getFontColor WRITE setFontColor NOTIFY fontColorChanged)
    Q_PROPERTY(QString selectedColor READ getSelectedColor WRITE setSelectedColor NOTIFY selectedColorChanged)
    Q_PROPERTY(QString highlightColor READ getHighlightColor WRITE setHighlightColor NOTIFY highlightColorChanged)

public:
    NConsoleMessage(QObject *parent = nullptr);
    NConsoleMessage(
        const QString message,
        const QString backgroundColor,
        const QString fontColor,
        const QString selectedColor,
        const QString highlightColor,
        QObject *parent = nullptr
    );

public:
    QString getMessage() const;
    QString getBackgroundColor() const;
    QString getFontColor() const;
    QString getSelectedColor() const;
    QString getHighlightColor() const;

    void setMessage(const QString &value);
    void setBackgroundColor(const QString &value);
    void setFontColor(const QString &value);
    void setSelectedColor(const QString &value);
    void setHighlightColor(const QString &value);

signals:
    void messageChanged();
    void backgroundColorChanged();
    void fontColorChanged();
    void selectedColorChanged();
    void highlightColorChanged();

private:
    QString Message;
    QString BackgroundColor;
    QString FontColor;
    QString SelectedColor;
    QString HighlightColor;
};

class NRootContext : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString performanceStatistics READ getPerformanceStatistics WRITE setPerformanceStatistics NOTIFY performanceStatisticsChanged)
    Q_PROPERTY(QQmlListProperty<NConsoleMessage> messages READ getMessages NOTIFY messagesChanged)

public:
    QString getPerformanceStatistics() const;
    void setPerformanceStatistics(const QString &value);
    QQmlListProperty<NConsoleMessage> getMessages();

public:
    QList<NConsoleMessage*> &getMessagesList();

signals:
    void performanceStatisticsChanged();
    void messagesChanged();
    void visibleMessagesChanged();

private:
    QString PerformanceStatistics;
    QList<NConsoleMessage*> Messages;
};

#endif
