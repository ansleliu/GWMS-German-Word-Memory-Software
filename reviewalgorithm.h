#ifndef REVIEWALGORITHM_H
#define REVIEWALGORITHM_H
#include "qmath.h"
#include "QString"
#include "QList"

class ReviewAlgorithm
{
public:
    ReviewAlgorithm();
    ~ReviewAlgorithm();

public:
    QList<QString> setInterval_Repetition(int currentInterval,double currentEF,
                                          int currentGrade,int currentRepetition);
    QList<QString> setInterval_Repetition_curr_Review(int currentInterval,double currentEF,
                                          int currentGrade,int currentRepetition);
    double setEF(double currentEF,int currentGrade);

private:
    int Grade;        //���ʵȼ�
    int Repetition;   //�ظ�����
    int Interval;     //���ʱ��
    double EF;        //�Ѷ�����
};

#endif // REVIEWALGORITHM_H
