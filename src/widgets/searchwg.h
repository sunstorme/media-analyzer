// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#ifndef SEARCHWG_H
#define SEARCHWG_H

#include <QMap>
#include <QWidget>
#include <QCheckBox>
#include <QStringList>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QLineEdit>

#include <common/zflowlayout.h>


namespace Ui {
class SearchWG;
}

class SearchWG : public QWidget
{
    Q_OBJECT

public:
    // Enum for controlling which group boxes to show
    enum GroupBoxType {
        None = 0x00,
        SearchRange = 0x01,
        MatchControl = 0x02,
        Files = 0x04,
        Time = 0x08,
        Operation = 0x10,
        All = SearchRange | MatchControl | Files | Time | Operation
    };
    Q_DECLARE_FLAGS(GroupBoxTypes, GroupBoxType)

    explicit SearchWG(QWidget *parent = nullptr);
    ~SearchWG();

    // Interface for managing search range checkboxes
    void setSearchRangeOptions(const QStringList &options);
    void clearSearchRangeOptions();
    QStringList getSelectedSearchRanges() const;
    void setSelectedSearchRanges(const QStringList &selectedOptions);

    // GroupBox visibility control
    void setVisibleGroupBoxes(GroupBoxTypes types);
    GroupBoxTypes getVisibleGroupBoxes() const;
    void showGroupBox(GroupBoxType type, bool show = true);
    void hideGroupBox(GroupBoxType type);

    // Match Control
    bool isCaseSensitive();
    bool isMatchWholewords();
    bool isUseRegularExpression();

    void setSearchText(const QString &text);
    QString getSearchText();

    void setSearchStatus(const QString &text);

    QLineEdit *getSearchLE();
signals:
    void searchRangeSelectionChanged(const QStringList &selectedOptions);
    void searchReady();
    void searchTextChanged(QString text);
    void searchNext();
    void searchBefore();
    void searchClear();
    void matchControlChanged();

private slots:
    void onSelectAllClicked(bool checked);
    void onSelectNoneClicked();
    void onSearchRangeCheckboxToggled();
    void onGroupBoxToggled(bool checked);
    void onMatchControlChanged();

    void on_search_btn_clicked();
    void on_search_le_textChanged(const QString &arg1);

    void on_search_le_editingFinished();

    void on_before_btn_clicked();

    void on_next_btn_clicked();

    void on_clear_btn_clicked();

private:
    void setupGroupBoxes();
    void setupSearchRangeControls();
    void updateGroupBoxDetail(QGroupBox *groupBox, bool visible);
    void connectCheckboxSignals();
    QGroupBox* getGroupBoxByType(GroupBoxType type) const;

private:
    Ui::SearchWG *ui;
    ZFlowLayout *m_floatLayout = nullptr;
    
    // Search range controls
    QRadioButton *m_selectAllRadioBtn = nullptr;
    QRadioButton *m_selectNoneRadioBtn = nullptr;
    QList<QCheckBox*> m_searchRangeCheckBoxes;
    
    // Group box visibility control
    GroupBoxTypes m_visibleGroupBoxes;
    QMap<QGroupBox*, int> m_originalHeights;
    
    // Search state tracking
    bool m_isSearching = false;
    QString m_lastSearchText;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(SearchWG::GroupBoxTypes)

#endif // SEARCHWG_H
