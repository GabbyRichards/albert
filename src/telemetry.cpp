// Copyright (C) 2014-2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/logging.h"
#include "telemetry.h"
#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QSettings>
static const char *CFG_LAST_REPORT = "last_report";
static const char *CFG_TELEMETRY = "telemetry";


Telemetry::Telemetry()
{
    QObject::connect(&timer_, &QTimer::timeout, &timer_, [this]{trySendReport();});

    auto settings = albert::settings();
    if (!settings->contains(CFG_TELEMETRY)) {
        QMessageBox mb(QMessageBox::Question, "Albert telemetry",
                       "Albert collects anonymous data to improve user experience. You can check "
                       "the data sent in the details. Opt in?",
                       QMessageBox::No|QMessageBox::Yes);
        mb.setDefaultButton(QMessageBox::Yes);
        mb.setDetailedText(QJsonDocument(buildReport()).toJson(QJsonDocument::Indented));
        settings->setValue(CFG_TELEMETRY, mb.exec() == QMessageBox::Yes);
    }
    enable(settings->value(CFG_TELEMETRY).toBool());

    // Move to state // TODO remove on next major version
    if (albert::settings()->contains(CFG_LAST_REPORT)){
        albert::state()->setValue(CFG_LAST_REPORT, albert::settings()->value(CFG_LAST_REPORT));
        albert::settings()->remove(CFG_LAST_REPORT);
    }
}

void Telemetry::enable(bool enable)
{
    if (enable)
        timer_.start(60000);
    else
        timer_.stop();
    albert::settings()->setValue(CFG_TELEMETRY, enable);
}

bool Telemetry::isEnabled() const
{
    return albert::settings()->value(CFG_TELEMETRY).toBool();
}

void Telemetry::trySendReport()
{
    // timezones and daytimes of users make it complicated to get trustworthy per day data.
    // Therefore three hours sampling rate.

    auto ts = albert::state()->value(CFG_LAST_REPORT, 0).toUInt();
    if (ts < QDateTime::currentSecsSinceEpoch() - 10800) {
        QJsonObject object = buildReport();
        QString addr = "Zffb,!!*\" $## $\"' **!";
        for ( auto &c: addr)
            c.unicode()=c.unicode()+14;

        QNetworkRequest request((QUrl(addr)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
        QNetworkReply* reply = albert::networkManager()->put(request, QJsonDocument(object).toJson(QJsonDocument::Compact));
        QObject::connect(reply, &QNetworkReply::finished, [reply](){
            if (reply->error() == QNetworkReply::NoError){
                DEBG << "Report sent.";
                albert::state()->setValue(CFG_LAST_REPORT, QDateTime::currentSecsSinceEpoch());
            }
            reply->deleteLater();
        });
    }
}

QJsonObject Telemetry::buildReport()
{
    QJsonObject object;
    object.insert("report", 1);
    object.insert("version", qApp->applicationVersion());
    object.insert("os", QSysInfo::prettyProductName());
    object.insert("id", QString::fromUtf8(QCryptographicHash::hash(QSysInfo::machineUniqueId(), QCryptographicHash::Sha1).toHex()).left(12));
    return object;
}
