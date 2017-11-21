#ifndef AJSONTOOLS_H
#define AJSONTOOLS_H

#include <QJsonObject>
#include <QJsonArray>

class QCheckBox;
class QSpinBox;
class QLineEdit;
class QComboBox;

bool parseJson(const QJsonObject &json, const QString &key, bool &var);
bool parseJson(const QJsonObject &json, const QString &key, int &var);
bool parseJson(const QJsonObject &json, const QString &key, double &var);
bool parseJson(const QJsonObject &json, const QString &key, float &var);
bool parseJson(const QJsonObject &json, const QString &key, QString &var);
bool parseJson(const QJsonObject &json, const QString &key, QJsonArray &var);
bool parseJson(const QJsonObject &json, const QString &key, QJsonObject &var);

void JsonToCheckbox(QJsonObject& json, QString key, QCheckBox* cb);
void JsonToSpinBox(QJsonObject& json, QString key, QSpinBox* sb);
void JsonToLineEdit(QJsonObject& json, QString key, QLineEdit* le);
void JsonToLineEditText(QJsonObject& json, QString key, QLineEdit* le);
void JsonToComboBox(QJsonObject& json, QString key, QComboBox* qb);

bool writeTwoQVectorsToJArray(QVector<double> &x, QVector<double> &y, QJsonArray &ar);
void readTwoQVectorsFromJArray(QJsonArray &ar, QVector<double> &x, QVector<double> &y);
bool write2DQVectorToJArray(QVector< QVector<double> > &xy, QJsonArray &ar);
void read2DQVectorFromJArray(QJsonArray &ar, QVector<QVector<double> > &xy);

bool LoadJsonFromFile(QJsonObject &json, QString fileName);
bool SaveJsonToFile(QJsonObject &json, QString fileName);

const QJsonObject SaveWindowToJson(int x, int y, int w, int h, bool bVis);
void LoadWindowFromJson(const QJsonObject& js, int& x, int& y, int& w, int& h, bool& bVis);

#endif // AJSONTOOLS_H
