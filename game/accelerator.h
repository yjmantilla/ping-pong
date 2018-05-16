#ifndef ACCELERATOR_H
#define ACCELERATOR_H

#include <QGraphicsEllipseItem>
#include <QBrush>


class Accelerator:public QGraphicsEllipseItem
{
public:
    QBrush brush;

    double xVelocity=0;
    double yVelocity=0;
    double xAcceleration=0;
    double yAcceleration=0;

    double mass;
    double radius;


    Accelerator();
    Accelerator(double radius, double mass, Qt::BrushStyle brush, Qt::GlobalColor color, double x, double y);
};

#endif // ACCELERATOR_H