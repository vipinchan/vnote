#include "newnotedialog.h"

#include <QtWidgets>

#include "notebook/notebook.h"
#include "notebook/node.h"
#include "../widgetsfactory.h"
#include <utils/pathutils.h>
#include "exception.h"
#include "nodeinfowidget.h"

using namespace vnotex;

const QString NewNoteDialog::c_defaultNoteName = "new_note.md";

NewNoteDialog::NewNoteDialog(Node *p_node, QWidget *p_parent)
    : ScrollDialog(p_parent)
{
    Q_ASSERT(p_node && p_node->isLoaded());
    setupUI(p_node);

    initDefaultValues();

    m_infoWidget->getNameLineEdit()->setFocus();
}

void NewNoteDialog::setupUI(const Node *p_node)
{
    setupNodeInfoWidget(p_node, this);
    setCentralWidget(m_infoWidget);

    setDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    setButtonEnabled(QDialogButtonBox::Ok, false);

    setWindowTitle(tr("New Note"));
}

void NewNoteDialog::setupNodeInfoWidget(const Node *p_node, QWidget *p_parent)
{
    m_infoWidget = new NodeInfoWidget(p_node, Node::Type::File, p_parent);
    connect(m_infoWidget, &NodeInfoWidget::inputEdited,
            this, &NewNoteDialog::validateInputs);
}

void NewNoteDialog::validateInputs()
{
    bool valid = true;
    QString msg;

    valid = valid && validateNameInput(msg);
    setInformationText(msg, valid ? ScrollDialog::InformationLevel::Info
                                  : ScrollDialog::InformationLevel::Error);
    setButtonEnabled(QDialogButtonBox::Ok, valid);
}

bool NewNoteDialog::validateNameInput(QString &p_msg)
{
    p_msg.clear();

    auto name = m_infoWidget->getName();
    if (name.isEmpty()) {
        p_msg = tr("Please specify a name for the note.");
        return false;
    }

    if (m_infoWidget->getParentNode()->hasChild(name, false)) {
        p_msg = tr("Name conflicts with existing note.");
        return false;
    }

    return true;
}

void NewNoteDialog::acceptedButtonClicked()
{
    if (newNote()) {
        accept();
    }
}

bool NewNoteDialog::newNote()
{
    m_newNode.clear();

    Notebook *notebook = const_cast<Notebook *>(m_infoWidget->getNotebook());
    Node *parentNode = const_cast<Node *>(m_infoWidget->getParentNode());
    try {
        m_newNode = notebook->newNode(parentNode, Node::Type::File, m_infoWidget->getName());
    } catch (Exception &p_e) {
        QString msg = tr("Failed to create note under (%1) in (%2) (%3).").arg(parentNode->getName(),
                                                                               notebook->getName(),
                                                                               p_e.what());
        qCritical() << msg;
        setInformationText(msg, ScrollDialog::InformationLevel::Error);
        return false;
    }

    emit notebook->nodeUpdated(m_newNode.data());

    return true;
}

const QSharedPointer<Node> &NewNoteDialog::getNewNode() const
{
    return m_newNode;
}

void NewNoteDialog::initDefaultValues()
{
    {
        auto lineEdit = m_infoWidget->getNameLineEdit();
        lineEdit->setText(c_defaultNoteName);
        int dotIndex = c_defaultNoteName.lastIndexOf('.');
        lineEdit->setSelection(0, (dotIndex == -1) ? c_defaultNoteName.size() : dotIndex);

        validateInputs();
    }
}
