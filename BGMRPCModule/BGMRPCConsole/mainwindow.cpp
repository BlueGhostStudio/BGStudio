#include "mainwindow.h"

#include <callgraph.h>

#include <QJsonDocument>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUi(this);
    toolBar->addWidget(leUrl);
    toolBar->addWidget(lePort);
    toolBar->addWidget(tbConnect);
    splitter->setSizes({ 1000, 256 });

    /*QObject::connect(&m_client, &NS_BGMRPCClient::BGMRPCClient::connected,
    this,
                     [=]() {
                         qDebug() << "connected";
                         tbConnect->setText("Disconnect");
                     });
    QObject::connect(&m_client, &NS_BGMRPCClient::BGMRPCClient::disconnected,
                     this, [=]() {
                         qDebug() << "disconnected";
                         tbConnect->setText("Connect");
                     });*/
    QObject::connect(&m_client,
                     &NS_BGMRPCClient::BGMRPCClient::isConnectedChanged, this,
                     [=](bool c) {
                         widExec->setVisible(c);
                         tbConnect->setText(c ? "Disconnect" : "Connect");
                     });
    QObject::connect(
        &m_client, &NS_BGMRPCClient::BGMRPCClient::remoteSignal, this,
        [=](const QString& object, const QString& sig, const QJsonArray& args) {
            qDebug() << "<strong>onRemoteSignal</strong>" << object << sig
                     << args;
        });

    widExec->hide();
}

MainWindow::~MainWindow() {}

void
MainWindow::message(const QString& m) {
    tbMessage->append(m);
}

void
MainWindow::switchConnect() {
    if (!m_client.isConnected()) {
        QString urlStr =
            QString("%1:%2").arg(leUrl->text()).arg(lePort->text());
        qDebug() << urlStr;
        m_client.connectToHost(QUrl(urlStr));
    } else
        m_client.disconnectFromHost();
}

void
MainWindow::callMethod() {
    /*qDebug() << "object" << leRemoteObject->text() << "method"
             << leRemoteMethod->text()
             << QJsonDocument::fromJson(
                    pteRemoteMethodArgs->toPlainText().toUtf8())
                    .toVariant();*/

    CallGraph::start("initial", this)
        ->nodes("initial",
                [=](QPointer<CallGraph> cg, const QVariant&) {
                    if (leRemoteObject->text().isEmpty() ||
                        leRemoteMethod->text().isEmpty()) {
                        qCritical() << "<strong>ERROR</strong>"
                                    << "No remote object or method specified";
                        cg->toFinal();
                    } else {
                        widExec->setEnabled(false);
                        cg->to("call");
                    }
                })
        ->nodes(
            "call",
            [=](QPointer<CallGraph> cg, const QVariant&) {
                qDebug() << "<strong>Call</strong>"
                         << leRemoteObject->text() + '.' +
                                leRemoteMethod->text() + "()";
                m_client
                    .callMethod(leRemoteObject->text(), leRemoteMethod->text(),
                                QJsonDocument::fromJson(
                                    pteRemoteMethodArgs->toPlainText().toUtf8())
                                    .toVariant()
                                    .toList())
                    ->then([=](const QVariant& ret) { cg->to("result", ret); });
            })
        ->nodes("result",
                [=](QPointer<CallGraph> cg, const QVariant& data) {
                    qDebug() << "<strong>return</strong>";
                    qDebug().noquote()
                        << QJsonDocument::fromVariant(data).toJson();
                    widExec->setEnabled(true);
                    cg->toFinal();
                })
        ->exec();
}
