#ifndef HALLDIALOG_H
#define HALLDIALOG_H

#include <QDialog>
#include <QTableWidget>

class MainWindow;

class HallDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HallDialog(MainWindow *main_window);
    void refreshRooms(int page);
    void joinRoom(int room_id);

    void roomBegin(int total, int pagelimit);
    void room(int room_id, int joined, const QString &setup_string);
    void roomEnd();

private:
    MainWindow *main_window;
    QTableWidget *table;
    int current_page;
    int room_row;

private slots:
    void pageUp();
    void pageDown();
    void join();
    void createRoom();
    void toggleDisplay(bool only_nonful);
};

#endif // HALLDIALOG_H
