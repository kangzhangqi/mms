#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>

#include "ConfigDialogField.h"

namespace mms {

class ConfigDialog : public QDialog {

    Q_OBJECT

public:

    ConfigDialog(
        const QString& action,
        const QString& object,
        const QVector<ConfigDialogField>& fields,
        bool includeRemoveButton);

    bool removeButtonPressed();
    QString getValue(const QString& key);

private:

    bool m_removeButtonPressed;
    QDialogButtonBox* m_buttons;
    QMap<QString, QLineEdit*> m_lineEdits;

};

} // namespace mms
