// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef TABLEFMTWG_H
#define TABLEFMTWG_H

#include <QWidget>
#include <QAction>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QShortcut>
#include <QClipboard>
#include <QThread>
#include <QTimer>
#include <QHeaderView>
#include <QtConcurrent>
#include <QMessageBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QProcess>
#include <QProgressBar>
#include <QLabel>
#include <QElapsedTimer>

#include "common/common.h"
#include "common/ztableheadermanager.h"
#include "common/zwindowhelper.h"
#include "common/qtcompat.h"

#include "model/mediainfotabelmodel.h"
#include "model/multicolumnsearchproxymodel.h"

#include "widgets/searchwg.h"
#include "widgets/helpquerywg.h"
#include "widgets/basefmtwg.h"

// Forward declarations
class ProgressDialog;

namespace Ui {
class TableFormatWG;
}

class TableFormatWG : public BaseFormatWG
{
    Q_OBJECT

public:
    // Copy operation type enumeration
    enum CopyOperation {
        CopySelectedText,
        CopySelectedTextWithHeader,
        CopySelectedRows,
        CopySelectedRowsWithHeader,
        CopySelectedColumns,
        CopySelectedColumnsWithHeader,
        CopyAllData,
        CopyAllDataWithHeader
    };

    explicit TableFormatWG(QWidget *parent = nullptr);
    ~TableFormatWG();

    void setHelpInfoKey(const QString& key);

    void setSearchTitleVisiable(const bool& visiable);

    /**
     * @brief Start streaming JSON loading from a command
     * @param program The program to execute (e.g., "ffprobe")
     * @param args The arguments for the program
     * @param arrayKey The JSON array key to parse ("frames" or "packets")
     */
    void startStreamingLoad(const QString &program, const QStringList &args, const QString &arrayKey = "frames");

    /**
     * @brief Check if streaming loading is active
     */
    bool isStreaming() const;

    /**
     * @brief Stop the streaming loading
     */
    void stopStreaming();

    QList <QStringList> getSelectLines();

    QList <int> getSelectRows();

    const QList<QStringList> *getTableData();

signals:
    void dataChanged(QStringList line);
    void contextMenuAboutToShow();
    void streamingFinished(int totalRows);

public slots:
    void initDetailTb(const QString& data, const QString &format_key);

    void initHeaderDetailTb(const QStringList &headers, QString format_join = "");

    void updateDataDetailTb(const QList<QStringList> &data_tb, QString format_join = "");

    void updateDataDetailTb(const QMap<QString, QList<QStringList>>&data_tb, QString format_join = "");

    void removeDataFromRowIndexs(const QList<int>& indexs);

    void appendDataDetailTb(const QList<QStringList> &data_tb, QString format_join = "");

    void removeSelectedRow();

    void clearDetailTb();

    void copySelectedText();

    void copySelectedTextWithHeader();

    void copySelectedRows();

    void copySelectedRowsWithHeader();

    void copySelectedColumns();

    void copySelectedColumnsWithHeader();

    void copyAllData();

    void copyAllDataWithHeader();

    void copyCmd();

    void fitTableColumnToContent();

    void showDetailInfo();

private slots:
    // Search functionality
    void onSearchReady();
    void onSearchTextChanged(const QString &text);
    void onSearchClear();
    
    // Column width management slots
    void onHeaderSectionResized(int logicalIndex, int oldSize, int newSize);
    void onResizeTimerTimeout();

    void previewImage();
    void saveImage();
    void onContextMenuAboutToShow();
    void onStreamingDataReady();
    void onStreamingFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onStreamingError(QProcess::ProcessError error);

private:
    void updateCurrentModel(); // Helper method to update the current active model
    QString getSelectedText(bool includeHeader = false);
    void setupTableModel(); // Setup table model and view
    
    // Universal copy function
    void copyData(CopyOperation operation);
    QString prepareCopyData(CopyOperation operation);

private:
    void formatData(const QString& data, QList<QStringList> &data_tb, QStringList &headers, QString format_key);

private:
    Ui::TableFormatWG *ui;

    QStringList m_headers;
    QList<QStringList> m_data_tb;

    MediaInfoTabelModel *m_model = nullptr;
    MultiColumnSearchProxyModel *multiColumnSearchModel = nullptr;
    ZTableHeaderManager *m_headerManager = nullptr;
    
    // Search component
    SearchWG *m_searchWG = nullptr;

    // Copy
    QMenu *m_copyMenu = nullptr;
    QAction *m_copySelectedTextAction = nullptr;
    QAction *m_copySelectedTextWithHeaderAction = nullptr;
    QAction *m_copySelectedRowsAction = nullptr;
    QAction *m_copySelectedRowsWithHeaderAction = nullptr;
    QAction *m_copySelectedColumnsAction = nullptr;
    QAction *m_copySelectedColumnsWithHeaderAction = nullptr;
    QAction *m_copyAllDataAction = nullptr;
    QAction *m_copyAllDataWithHeaderAction = nullptr;
    QAction *m_copyCmdAction = nullptr;

    // image menu
    QMenu *m_imageMenu = nullptr;
    QAction *m_previewImageAction = nullptr;
    QAction *m_saveImageAction = nullptr;

    QString valueToString(const QJsonValue &value);
    QString extractSideData(const QJsonObject &frameObj, const QString &key);

    QStringList getSelectedMediaTypes();

    void updateImageMenuVisibility();

    int m_currentRow;
    int m_currentColumn;
    QString m_helpKey;
    QAction *m_detailAction = nullptr;
    QAction *m_restoreOrderAction = nullptr;
    QAction *m_fitTableColumnAction = nullptr;
    QAction *m_switchViewAction = nullptr;
    QAction *m_searchAction = nullptr;
    QShortcut *m_searchShortcut = nullptr;
    
    // Column width management
    QVector<double> m_columnWidthRatios;
    bool m_isUserAdjusted;
    QTimer *m_resizeTimer = nullptr;
    int m_lastTableWidth;
    
    // Copy operations
    ProgressDialog *m_copyProgressDialog;
    
    // Streaming JSON loading state
    QProcess *m_streamingProcess = nullptr;
    QByteArray m_streamingBuffer;
    bool m_streamingActive = false;
    bool m_streamingArrayFound = false;
    int m_streamingRowCount = 0;
    QString m_streamingArrayKey;
    QElapsedTimer m_streamingTimer;
    
    // Streaming JSON field classification (same as in loadJson)
    struct StreamingFieldCategory {
        QStringList common;
        QStringList video;
        QStringList audio;
        QSet<QString> allFields;
    };
    StreamingFieldCategory m_streamingCategories;
    QHash<QString, QStringList> m_streamingHeaderTemplates;
    QString m_streamingCurrentMediaType;
    bool m_streamingHeadersInitialized = false;
    
    // Streaming helpers
    void processStreamingBuffer();
    void processStreamingObject(const QJsonObject &obj);
    void initStreamingHeaders();
    void finalizeStreaming();
    void showStreamingProgress();
    void hideStreamingProgress();
    void updateStreamingProgress();
    
    void setupColumnWidthManagement();
    void saveColumnWidthRatios();
    void restoreColumnWidthRatios();
    void resizeColumnsProportionally();
    void setupInitialColumnWidths();
    bool eventFilter(QObject *obj, QEvent *event) override;

protected:
    // Implement pure virtual functions from base class
    void initUI() override;
    void initMenu() override;
    void initConnection() override;
    void initShortCut() override;
    void initExtra() override;
    
    bool loadJson(const QByteArray &json) override;

    void showContextMenu(const QPoint &pos) override;

    QModelIndexList getSelectedIndexes() override;
    bool hasValidSelection() override;

    // Override base class getting all data functions
    QString getAllData() override;
};

#endif // TABLEFMTWG_H
