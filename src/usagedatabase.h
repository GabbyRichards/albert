// Copyright (c) 2022 Manuel Schneider

#pragma once
#include <QSqlDatabase>
#include <QString>
#include <vector>
#include <shared_mutex>
#include <mutex>
#include <map>
namespace albert {
class Extension;
class RankItem;
}

struct Activation {
    Activation(QString q, QString e, QString i, QString a);
    QString query;
    QString extension_id;
    QString item_id;
    QString action_id;
};

class UsageHistory
{
public:
    static void initialize();

    static void applyScores(const QString &id, std::vector<albert::RankItem> &rank_items);
    static void applyScores(std::vector<std::pair<albert::Extension*,albert::RankItem>>*);

    static double memoryDecay();
    static void setMemoryDecay(double);

    static bool prioritizePerfectMatch();
    static void setPrioritizePerfectMatch(bool);

    static void addActivation(const QString &query, const QString &extension,
                              const QString &item, const QString &action);

private:
    inline static void applyScore(const QString &extension_id, albert::RankItem *rank_item);
    static void updateScores();

    static std::shared_mutex global_data_mutex_;
    static std::map<std::pair<QString,QString>,float> usage_scores_;
    static bool prioritize_perfect_match_;
    static double memory_decay_;

    static std::recursive_mutex db_recursive_mutex_;
    static void db_connect();
    static void db_initialize();
    static void db_clearActivations();
    static std::vector<Activation> db_activations();
    static void db_addActivation(const QString &query, const QString &extension,
                                 const QString &item, const QString &action);
};


