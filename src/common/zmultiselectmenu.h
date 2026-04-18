// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef ZMULTISELECTMENU_H
#define ZMULTISELECTMENU_H

#include <QMenu>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>

class ZMultiSelectMenu : public QMenu
{
    Q_OBJECT

public:
    explicit ZMultiSelectMenu(QWidget *parent = nullptr) : QMenu(parent) {}

protected:
    void mouseReleaseEvent(QMouseEvent *event) override
    {
        // Only respond to left-button releases to prevent accidental triggering.
        // When right-clicking the header to open this menu, the right-button
        // release event would otherwise toggle the checkable action under the cursor.
        if (event->button() != Qt::LeftButton) {
            QMenu::mouseReleaseEvent(event);
            return;
        }
        QAction *action = actionAt(event->pos());
        if (action && action->isCheckable()) {
            action->setChecked(!action->isChecked());
            emit action->toggled(action->isChecked());
            emit action->triggered(action->isChecked());
            return;
        }
        QMenu::mouseReleaseEvent(event);
    }

    void keyPressEvent(QKeyEvent *event) override
    {
        if (event->key() == Qt::Key_Escape) {
            close();
            return;
        }
        QMenu::keyPressEvent(event);
    }

    bool event(QEvent *event) override
    {
        if (event->type() == QEvent::Leave) {
            return true;
        }
        return QMenu::event(event);
    }
};

#endif // ZMULTISELECTMENU_H