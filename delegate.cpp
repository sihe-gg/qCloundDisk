#include "delegate.h"
#include <QDebug>

delegate::delegate(QWidget *parent)
{
    this->setParent(parent);
}

void delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.isValid() && index.column() == 3)
    {
        QStyleOptionProgressBar bar;

        bar.rect = option.rect;

        bar.progress = index.data().toInt();
        bar.maximum = 100;
        bar.minimum = 0;
        bar.text = QString::number(bar.progress) + "%";
        bar.textVisible = true;
        bar.textAlignment = Qt::AlignCenter;

        // 设置样式
        //这里需要QStyle在绘制的时候设置drawControl的最后一个参数，是一个QWidget *，这里我们使用QProgressBar即可
        QProgressBar progressBar;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &bar, painter, &progressBar);
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

