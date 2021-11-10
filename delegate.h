#ifndef DELEGATE_H
#define DELEGATE_H

#include <QObject>
#include <QProgressBar>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QPainter>
#include <QModelIndex>
#include <QPushButton>
#include <QAbstractItemDelegate>

// 添加动态数据 setItemWidget
class delegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    delegate(QWidget *parent = nullptr);

    // painting
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;


    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

signals:
    void open(const QModelIndex &index);
    void delData(const QModelIndex &index);


private:
    QPoint m_mousePoint;    // 鼠标位置
    QScopedPointer<QPushButton> m_openButton;       // 智能指针
    QScopedPointer<QPushButton> m_delButton;
    QStringList m_list;
    int m_spacing;          // 按钮之间的宽度
    int m_width;            // 按钮宽度
    int m_height;           // 按钮高度
    int m_type;             // 按钮状态 1:划过 2:按下

};

#endif // DELEGATE_H
