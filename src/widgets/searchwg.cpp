// SPDX-FileCopyrightText: 2025 zhang hongyuan <2063218120@qq.com>
// SPDX-License-Identifier: MIT

#include "searchwg.h"
#include "ui_searchwg.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

SearchWG::SearchWG(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SearchWG)
    , m_selectAllRadioBtn(nullptr)
    , m_selectNoneRadioBtn(nullptr)
    , m_visibleGroupBoxes(GroupBoxType::All)
{
    ui->setupUi(this);

    // Initialize the flow layout for search range
    m_floatLayout = new ZFlowLayout();
    ui->search_range_groupBox->setLayout(m_floatLayout);

    // Setup all group boxes and controls
    setupGroupBoxes();
    setupSearchRangeControls();
}

SearchWG::~SearchWG()
{
    delete ui;
}

void SearchWG::setupGroupBoxes()
{
    // Make all group boxes checkable
    ui->search_range_groupBox->setCheckable(true);
    ui->match_control_groupBox->setCheckable(true);
    ui->file_groupBox->setCheckable(true);
    ui->time_groupBox->setCheckable(true);

    // Store original heights
    m_originalHeights[ui->search_range_groupBox] = ui->search_range_groupBox->sizeHint().height();
    m_originalHeights[ui->match_control_groupBox] = ui->match_control_groupBox->sizeHint().height();
    m_originalHeights[ui->file_groupBox] = ui->file_groupBox->sizeHint().height();
    m_originalHeights[ui->time_groupBox] = ui->time_groupBox->sizeHint().height();
    m_originalHeights[ui->operation_groupBox] = ui->operation_groupBox->sizeHint().height();

    // Set initial states - only operation_groupBox is expanded
    ui->search_range_groupBox->setChecked(false);
    ui->match_control_groupBox->setChecked(false);
    ui->file_groupBox->setChecked(false);
    ui->time_groupBox->setChecked(false);

    // Connect group box toggle signals
    connect(ui->search_range_groupBox, &QGroupBox::toggled, this, &SearchWG::onGroupBoxToggled);
    connect(ui->match_control_groupBox, &QGroupBox::toggled, this, &SearchWG::onGroupBoxToggled);
    connect(ui->file_groupBox, &QGroupBox::toggled, this, &SearchWG::onGroupBoxToggled);
    connect(ui->time_groupBox, &QGroupBox::toggled, this, &SearchWG::onGroupBoxToggled);
    connect(ui->operation_groupBox, &QGroupBox::toggled, this, &SearchWG::onGroupBoxToggled);

    // Connect match control checkbox signals
    connect(ui->case_sensitive_cbx, &QCheckBox::stateChanged, this, &SearchWG::onMatchControlChanged);
    connect(ui->match_whole_word_cbx, &QCheckBox::stateChanged, this, &SearchWG::onMatchControlChanged);
    connect(ui->use_regular_express_cbx, &QCheckBox::stateChanged, this, &SearchWG::onMatchControlChanged);

    // Apply initial visibility
    updateGroupBoxDetail(ui->search_range_groupBox, false);
    updateGroupBoxDetail(ui->match_control_groupBox, false);
    updateGroupBoxDetail(ui->file_groupBox, false);
    updateGroupBoxDetail(ui->time_groupBox, false);
    updateGroupBoxDetail(ui->operation_groupBox, true);
}

void SearchWG::setupSearchRangeControls()
{
    // Create select all and select none checkboxes
    m_selectAllRadioBtn = new QRadioButton(tr("Select All"), this);
    m_selectNoneRadioBtn = new QRadioButton(tr("Select None"), this);

    // Style the control checkboxes differently
    m_selectAllRadioBtn->setObjectName("selectAllCheckBox");
    m_selectNoneRadioBtn->setObjectName("selectNoneCheckBox");

    // Add them to the layout first
    m_floatLayout->addWidget(m_selectAllRadioBtn);
    m_floatLayout->addWidget(m_selectNoneRadioBtn);

    // Connect signals
    connect(m_selectAllRadioBtn, &QCheckBox::toggled, this, &SearchWG::onSelectAllClicked);
    connect(m_selectNoneRadioBtn, &QCheckBox::clicked, this, &SearchWG::onSelectNoneClicked);
}

void SearchWG::setSearchRangeOptions(const QStringList &options)
{
    // Clear existing checkboxes (except select all/none)
    clearSearchRangeOptions();

    // Create new checkboxes for each option
    for (const QString &option : options) {
        auto *checkBox = new QCheckBox(option, this);
        checkBox->setChecked(true);
        m_searchRangeCheckBoxes.append(checkBox);
        m_floatLayout->addWidget(checkBox);

        // Connect to the toggle signal
        connect(checkBox, &QCheckBox::toggled, this, &SearchWG::onSearchRangeCheckboxToggled);
    }

    // Update the layout
    m_floatLayout->update();
}

void SearchWG::clearSearchRangeOptions()
{
    // Remove and delete all search range checkboxes
    for (auto *checkBox : m_searchRangeCheckBoxes) {
        m_floatLayout->removeWidget(checkBox);
        checkBox->deleteLater();
    }
    m_searchRangeCheckBoxes.clear();
}

QStringList SearchWG::getSelectedSearchRanges() const
{
    QStringList selectedOptions;
    for (const auto *checkBox : m_searchRangeCheckBoxes) {
        if (checkBox->isChecked()) {
            selectedOptions.append(checkBox->text());
        }
    }
    return selectedOptions;
}

void SearchWG::setSelectedSearchRanges(const QStringList &selectedOptions)
{
    for (auto *checkBox : m_searchRangeCheckBoxes) {
        checkBox->setChecked(selectedOptions.contains(checkBox->text()));
    }
    
    // Update select all checkbox state
    bool allSelected = !m_searchRangeCheckBoxes.isEmpty() && 
                       m_searchRangeCheckBoxes.size() == getSelectedSearchRanges().size();
    m_selectAllRadioBtn->setChecked(allSelected);
    
    emit searchRangeSelectionChanged(getSelectedSearchRanges());
}

void SearchWG::onSelectAllClicked(bool checked)
{
    // Block signals temporarily to avoid recursive calls
    const bool blocked = blockSignals(true);
    
    for (auto *checkBox : m_searchRangeCheckBoxes) {
        checkBox->setChecked(checked);
    }
    
    blockSignals(blocked);
    emit searchRangeSelectionChanged(getSelectedSearchRanges());
}

void SearchWG::onSelectNoneClicked()
{
    // Block signals temporarily to avoid recursive calls
    const bool blocked = blockSignals(true);
    
    for (auto *checkBox : m_searchRangeCheckBoxes) {
        checkBox->setChecked(false);
    }
    
    blockSignals(blocked);
    emit searchRangeSelectionChanged(getSelectedSearchRanges());
}

void SearchWG::onSearchRangeCheckboxToggled()
{
    // Update select all/none states based on current selection
    const QStringList selected = getSelectedSearchRanges();
    const bool allSelected = !m_searchRangeCheckBoxes.isEmpty() && 
                             selected.size() == m_searchRangeCheckBoxes.size();
    const bool noneSelected = selected.isEmpty();
    
    // Block signals to avoid recursive calls
    const bool blocked = blockSignals(true);

    if (!allSelected && !allSelected) {
        m_selectAllRadioBtn->setAutoExclusive(false);
        m_selectNoneRadioBtn->setAutoExclusive(false);

        m_selectAllRadioBtn->setChecked(false);
        m_selectNoneRadioBtn->setChecked(false);
    } else {
        m_selectAllRadioBtn->setAutoExclusive(true);
        m_selectNoneRadioBtn->setAutoExclusive(true);
        m_selectAllRadioBtn->setChecked(allSelected);
        m_selectNoneRadioBtn->setChecked(noneSelected);
    }
    blockSignals(blocked);
    
    emit searchRangeSelectionChanged(selected);
}

void SearchWG::onGroupBoxToggled(bool checked)
{
    auto *groupBox = qobject_cast<QGroupBox*>(sender());
    if (groupBox) {
        updateGroupBoxDetail(groupBox, checked);
    }
}

void SearchWG::onMatchControlChanged()
{
    emit matchControlChanged();
}

void SearchWG::updateGroupBoxDetail(QGroupBox *groupBox, bool visible)
{
    if (!groupBox) return;
    
    if (visible) {
        // Expand: restore original height and show content
        groupBox->setMaximumHeight(QWIDGETSIZE_MAX);
        groupBox->setMinimumHeight(0);
        
        // Show all child widgets
        const auto children = groupBox->findChildren<QWidget*>();
        for (auto *child : children) {
            if (child->parent() == groupBox) {
                child->setVisible(true);
            }
        }
    } else {
        // Collapse: hide content and minimize height
        const auto children = groupBox->findChildren<QWidget*>();
        for (auto *child : children) {
            if (child->parent() == groupBox) {
                child->setVisible(false);
            }
        }
        
        // Set a minimal height to show only the title
        groupBox->setMaximumHeight(30);
        groupBox->setMinimumHeight(30);
    }
}

QGroupBox* SearchWG::getGroupBoxByType(GroupBoxType type) const
{
    switch (type) {
    case SearchRange: return ui->search_range_groupBox;
    case MatchControl: return ui->match_control_groupBox;
    case Files: return ui->file_groupBox;
    case Time: return ui->time_groupBox;
    case Operation: return ui->operation_groupBox;
    default: return nullptr;
    }
}

void SearchWG::setVisibleGroupBoxes(GroupBoxTypes types)
{
    m_visibleGroupBoxes = types;
    
    // Hide all group boxes first
    ui->search_range_groupBox->setVisible(false);
    ui->match_control_groupBox->setVisible(false);
    ui->file_groupBox->setVisible(false);
    ui->time_groupBox->setVisible(false);
    ui->operation_groupBox->setVisible(false);

    const auto children = this->findChildren<QGroupBox*>();
    for (auto *child : children) {
        if (child) {
            child->setVisible(false);
            child->setChecked(false);
        }
    }
    
    // Show only the requested group boxes
    if (types & SearchRange) {
        ui->search_range_groupBox->setVisible(true);
        ui->search_range_groupBox->setChecked(true);
    }
    if (types & MatchControl){
        ui->match_control_groupBox->setVisible(true);
        ui->match_control_groupBox->setChecked(true);
    }
    if (types & Files){
        ui->file_groupBox->setVisible(true);
        ui->file_groupBox->setChecked(true);
    }
    if (types & Time) {
        ui->time_groupBox->setVisible(true);
        ui->time_groupBox->setChecked(true);
    }
    if (types & Operation) {
        ui->operation_groupBox->setVisible(true);
        ui->operation_groupBox->setChecked(true);
    }
    
    // Update the layout
    adjustSize();
}

SearchWG::GroupBoxTypes SearchWG::getVisibleGroupBoxes() const
{
    return m_visibleGroupBoxes;
}

void SearchWG::showGroupBox(GroupBoxType type, bool show)
{
    QGroupBox* groupBox = getGroupBoxByType(type);
    if (groupBox) {
        groupBox->setVisible(show);
        
        // Update the flags
        if (show) {
            m_visibleGroupBoxes |= type;
        } else {
            m_visibleGroupBoxes &= ~type;
        }
        
        adjustSize();
    }
}

void SearchWG::hideGroupBox(GroupBoxType type)
{
    showGroupBox(type, false);
}

bool SearchWG::isCaseSensitive()
{
    return ui->case_sensitive_cbx->isChecked();
}

bool SearchWG::isMatchWholewords()
{
    return ui->match_whole_word_cbx->isChecked();
}

bool SearchWG::isUseRegularExpression()
{
    return ui->use_regular_express_cbx->isChecked();
}

void SearchWG::setSearchText(const QString &text)
{
    ui->search_le->setText(text);
}

QString SearchWG::getSearchText()
{
    return ui->search_le->text();
}

void SearchWG::setSearchStatus(const QString &text)
{
    ui->status_lb->setText(text);
}

QLineEdit *SearchWG::getSearchLE()
{
    return ui->search_le;
}

void SearchWG::on_search_btn_clicked()
{
    m_isSearching = true;
    m_lastSearchText = ui->search_le->text();
    emit searchReady();
}


void SearchWG::on_search_le_textChanged(const QString &arg1)
{
    emit searchTextChanged(arg1);
}


void SearchWG::on_search_le_editingFinished()
{
    QString text = ui->search_le->text();
    if (!text.isEmpty()) {
        if (m_isSearching && text == m_lastSearchText) {
            emit searchNext();
        } else {
            m_isSearching = true;
            m_lastSearchText = text;
            emit searchReady();
        }
    }
}


void SearchWG::on_before_btn_clicked()
{
    emit searchBefore();
}


void SearchWG::on_next_btn_clicked()
{
    emit searchNext();
}


void SearchWG::on_clear_btn_clicked()
{
    m_isSearching = false;
    m_lastSearchText.clear();
    emit searchClear();
}

