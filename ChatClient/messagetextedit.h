#ifndef MESSAGETEXTEDIT_H
#define MESSAGETEXTEDIT_H

#include <QObject>
#include <QTextEdit>
#include <QMouseEvent>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMimeType>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QPainter>
#include <QVector>
#include "global.h"


class MessageTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MessageTextEdit(QWidget *parent = nullptr);

    using QTextEdit::QTextEdit;

    void setOverlayMargins(int rightMargin, int bottomMargin)
    {
        setViewportMargins(0, 0, rightMargin, bottomMargin);
    }

    ~MessageTextEdit();

    QVector<MsgInfo> getMsgList();

    void insertFileFromUrl(const QStringList &urls);
signals:
    void send();
    //当 textEdit 尺寸变化时发出信号，连接此信号重新定位按钮
    void resized();
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void keyPressEvent(QKeyEvent *e);
    //当控件大小变化时会被调用，让外面更新按钮位置
    void resizeEvent(QResizeEvent *e) override{
            QTextEdit::resizeEvent(e);
            emit resized();
    }
private:
    void insertImages(const QString &url);
    void insertTextFile(const QString &url);
    bool canInsertFromMimeData(const QMimeData *source) const;
    void insertFromMimeData(const QMimeData *source);
    bool isImage(QString url);//判断文件是否为图片
    void insertMsgList(QVector<MsgInfo> &list,QString flag, QString text, QPixmap pix);

    QStringList getUrl(QString text);
    QPixmap getFileIconPixmap(const QString &url);//获取文件图标及大小信息，并转化成图片
    QString getFileSize(qint64 size);//获取文件大小
    QVector<MsgInfo> mMsgList;
    QVector<MsgInfo> mGetMsgList;
private slots:
    void textEditChanged();
};

#endif // MESSAGETEXTEDIT_H
