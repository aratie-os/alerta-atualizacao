#pragma once

#include <QDialog>

class UpdateNotification final : public QDialog
{
    Q_OBJECT

public:
    explicit UpdateNotification(bool waylandSession, QWidget *parent = nullptr);

    void showInPrimaryScreenCorner();

signals:
    void updateRequested();

private:
    bool m_waylandSession;
};
