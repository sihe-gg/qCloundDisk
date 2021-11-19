#include "delegate.h"
#include <QDebug>
#include <QMouseEvent>
#include <QToolTip>
#include <QIcon>

delegate::delegate(int flag, QWidget *parent)
    : QStyledItemDelegate(parent),
      m_openButton(new QPushButton()),
      m_delButton(new QPushButton()),
      m_spacing(5),
      m_width(25),
      m_height(20)
{
    this->setParent(parent);
    // 1 是 downloadList
    m_flag = flag;

    if(m_flag == 1)
    {
        // 设置按钮正常、划过、按下
        m_openButton->setStyleSheet("QPushButton {\
                                        border: none; \
                                        background-color: transparent;\
                                        image: url(:/fileIcon/open.png);\
                                    }\
                                    QPushButton:hover {image:url(:/fileIcon/openhover.png);} \
                                    QPushButton:pressed {image: url(:/fileIcon/openpressed.png);}");

        m_delButton->setStyleSheet("QPushButton {\
                                        border: none; \
                                        background-color: transparent;\
                                        image: url(:/fileIcon/del.png);\
                                    }\
                                    QPushButton:hover {image:url(:/fileIcon/delhover.png);} \
                                    QPushButton:pressed {image: url(:/fileIcon/delpressed.png);}");

        m_list << QStringLiteral("打开") << QStringLiteral("删除");
    }
    else
    {
        m_openButton->setStyleSheet("QPushButton {\
                                        border: none; \
                                        background-color: transparent;\
                                        image: url(:/fileIcon/shareInfo.png);\
                                    }\
                                    QPushButton:hover {image:url(:/fileIcon/shareInfo1.png);}");

        m_delButton->setStyleSheet("QPushButton {\
                                        border: none; \
                                        background-color: transparent;\
                                        image: url(:/fileIcon/delShare.png);\
                                    }\
                                    QPushButton:hover {image:url(:/fileIcon/delShare1.png);}");

        m_list << QStringLiteral("详细信息") << QStringLiteral("删除记录");
    }

    qApp->setStyleSheet("QPushButton m_openButton, m_delButton{}");
}

void delegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem viewOption(option);
    initStyleOption(&viewOption, index);
    if (option.state.testFlag(QStyle::State_HasFocus))
        viewOption.state = viewOption.state ^ QStyle::State_HasFocus;


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
    else if(index.column() == 4)
    {
        // 计算按钮显示区域
        int count = m_list.count();
        int half = (option.rect.width() - m_width * count - m_spacing * (count - 1)) / 2;
        int top = (option.rect.height() - m_height) / 2;

        for(int i = 0; i < count; i++)
        {
            // 绘制按钮
            QStyleOptionButton button;
            button.rect = QRect(option.rect.left() + half + m_width * i + m_spacing * i,
                                option.rect.top() + top, m_width, m_height);

            button.state |= QStyle::State_Enabled;

            if(button.rect.contains(m_mousePoint))
            {
                if(m_type == 0)
                {
                    // hover
                    button.state |= QStyle::State_MouseOver;
                }
                else if(m_type == 1)
                {
                    // pressed
                    button.state |= QStyle::State_Sunken;
                }
            }

            QWidget *pWidget = (i == 0) ? m_openButton.data() : m_delButton.data();
            QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter, pWidget);
        }
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool delegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.column() != 4)
        return false;

    m_type = -1;
    bool bRepaint = false;
    QMouseEvent *pEvent = static_cast<QMouseEvent *> (event);
    m_mousePoint = pEvent->pos();

    int nCount = m_list.count();
    int nHalf = (option.rect.width() - m_width * nCount - m_spacing * (nCount - 1)) / 2;
    int nTop = (option.rect.height() - m_height) / 2;

    // 还原鼠标样式
    QApplication::restoreOverrideCursor();

    for (int i = 0; i < nCount; ++i)
    {
        QStyleOptionButton button;
        button.rect = QRect(option.rect.left() + nHalf + m_width * i + m_spacing * i,
                            option.rect.top() + nTop,  m_width, m_height);

        // 鼠标位于按钮之上
        if (!button.rect.contains(m_mousePoint))
            continue;

        bRepaint = true;
        switch (event->type())
        {
        // 鼠标滑过
        case QEvent::MouseMove:
        {
            // 设置鼠标样式为手型
            QApplication::setOverrideCursor(Qt::PointingHandCursor);

            m_type = 0;
            QToolTip::showText(pEvent->globalPos(), m_list.at(i));
            break;
        }
        // 鼠标按下
        case QEvent::MouseButtonPress:
        {
            m_type = 1;
            break;
        }
        // 鼠标释放
        case QEvent::MouseButtonRelease:
        {
            if (i == 0)
            {
                emit open(index);
            }
            else
            {
                emit delData(index);
            }
            break;
        }
        default:
            break;
        }
    }

    return bRepaint;

}


