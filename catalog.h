#ifndef CATALOG_H
#define CATALOG_H

#include <QtGlobal>
#include <QVector>

struct CatalogData /*структура, содержащая звездный каталог*/
{
    QVector <double> alpha_vec;
    QVector <double> beta_vec;
    QVector <float> mv_vec;
    QVector <double> alpha_vec_sec;
    QVector <double> beta_vec_sec;
    QVector <long> count_sec_vec;
    QVector <long> shift_vec;
    QVector<short> new_numn;
};

#pragma pack(push,1)
struct sector // каталог секторов
{
    float alpha_c;
    float beta_c;
    qint16 count_in_sector;
    int shift;
};
#pragma pack(pop)



#pragma pack(push,1)
struct data_star // основной каталог/бортовой каталог
{
    qint32  NSAO;
    qint32 alpha;
    qint32 beta;
    qint16 ualpha;
    qint16 ubeta;
    unsigned char mv;
    char sp;
};
#pragma pack(pop)



#pragma pack(push,1)
struct numbers // основной каталог/бортовой каталог
{
    qint16 num;
};
#pragma pack(pop)

#endif // CATALOG_H
