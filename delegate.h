#ifndef DELEGATE_H
#define DELEGATE_H

#include <QObject>
#include <QProgressBar>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QPainter>
#include <QModelIndex>

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



//    QWidget *createEditor(QWidget *parent,
//                          const QStyleOptionViewItem &option,
//                          const QModelIndex &index) const;


//    virtual void
//    setEditorData(QWidget *editor, const QModelIndex &index) const;


//    virtual void
//    setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;





private:


};

#endif // DELEGATE_H
