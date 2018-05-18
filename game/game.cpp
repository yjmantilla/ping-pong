#include "game.h"
#include <QDebug>

#define FIELD_VISCOSITY 0.001
#define FIELD_COLOR Qt::black
#define PUCK_COLOR Qt::green
#define MIN_XV_PUCK 3
#define MAX_XV_PUCK 5
#define MIN_YV_PUCK 5
#define MAX_YV_PUCK 9
#define MIN_XV_AandB 1
#define MAX_XV_AandB 2
#define MIN_YV_AandB 2
#define MAX_YV_AandB 3
#define PUCK_STOP_XV 0.5
#define PUCK_STOP_YV 0.5
#define BOX_SPAWN_TIME 5 //IN SECONDS
#define ACCELERATOR_SPAWN_TIME 7 //IN SECONDS
#define REFRESH_TIME 10 //IN MILLISECONDS
#define ACCELERATOR_RADIUS 15
#define ACCELERATOR_MASS 3000
#define MIN_GOAL_TO_STRIKER_RATIO 3
#define MIN_STRIKER_VELOCITY 1
#define MAX_STRIKER_VELOCITY 7
#define INVISIBLE_PUCK_TIME 3 //IN SECONDS
#define VISCOSITY_TIME 10
#define WALL_RESTITUTION_TIME 5 //IN SECONDS
#define STRIKER_RESTITUTION_TIME 5 //IN SECONDS
#define PUCK_RADIUS 10
#define WALL_COLOR Qt::yellow
#define STRIKER1_COLOR Qt::cyan
#define STRIKER2_COLOR Qt::magenta
#define NARRATOR_COLOR Qt::yellow

Game::Game(QWidget *parent, qreal width, qreal height)
    : QWidget(parent)
{

    /*Set the Geometry of the Game*/

    this->width = width;
    this->height = height;
    this->scene = new QGraphicsScene();
    this->view = new QGraphicsView();
    this->view->setParent(this);
    this->view->setFixedSize(this->width+this->boundary,this->height+this->boundary);
    this->scene->setSceneRect(0,0,this->width,this->height);
    this->view->setScene(this->scene);
    this->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->view->setBackgroundBrush(QBrush(FIELD_COLOR, Qt::SolidPattern));

    /*Create Game Objects*/

    /*We want the puck with the same item coordinate origin as the scene to be able to track its position*/
    this->puck = new Puck(PUCK_RADIUS,Qt::SolidPattern,PUCK_COLOR,0,0);
    this->striker1 = new Striker(0,0,this->width/8,this->height/50,Qt::SolidPattern,STRIKER1_COLOR);
    this->striker2 = new Striker(0,0,this->width/8,this->height/50,Qt::SolidPattern,STRIKER2_COLOR);
    this->wallHU = new Wall(0,0,this->width,0,WALL_COLOR);
    this->wallHD = new Wall(0,this->height,this->width,this->height,WALL_COLOR);
    this->wallVL = new Wall(0,0,0,this->height,WALL_COLOR);
    this->wallVR = new Wall(this->width,0,this->width,this->height,WALL_COLOR);
    this->goal1 = new Goal(this->width/2,this->height,2*this->width/5,FIELD_COLOR);
    this->goal2 = new Goal(this->width/2,0,2*this->width/5,FIELD_COLOR);
    this->field = new Field(FIELD_VISCOSITY);
    this->score1 = new Score(0,this->striker1);
    this->score2 = new Score(0,this->striker2);
    this->narrator = new Narrator(NARRATOR_COLOR);




    /*Add Objects To Scene*/
    this->scene->addItem(this->puck);
    this->scene->addItem(this->striker1);
    this->scene->addItem(this->striker2);
    this->scene->addItem(this->wallHU);
    this->scene->addItem(this->wallHD);
    this->scene->addItem(this->wallVL);
    this->scene->addItem(this->wallVR);
    this->scene->addItem(this->goal1);
    this->scene->addItem(this->goal2);
    this->scene->addItem(this->score1);
    this->scene->addItem(this->score2);
    this->scene->addItem(this->narrator);


    /*Center Puck*/
    this->centerPuck();

    /*Give Puck Random Velocity*/
    /*Ensure x,y velocity is enough*/
    //set low x velocity for it to be more frontal
    this->velocify(this->puck->velocity,MIN_XV_PUCK,MAX_XV_PUCK,MIN_YV_PUCK,MAX_YV_PUCK);

    /*Set Striker,Scores Positions for each player*/
    this->striker1->setPos(this->scene->width()/2,this->scene->height()-this->striker1->rect().height());
    this->striker2->setPos(this->scene->width()/2,0);
    this->score1->setPos(this->scene->width()/2,this->scene->height()-this->striker1->rect().height()+25);
    this->score2->setPos(this->scene->width()/2,0-25);

    this->narrator->setPos(this->width/2,this->height/2);

    /*Initialize move booleans of players*/
    this->moveR1=false;
    this->moveR2=false;
    this->moveL1=false;
    this->moveL2=false;

    /*Animate Game with Timer*/
    this->motionTimer = new QTimer();
    connect(this->motionTimer,SIGNAL(timeout()),this,SLOT(animate()));

    /*Add Boxes and Accelerator*/
    this->boxTimer = new QTimer();
    connect(this->boxTimer,SIGNAL(timeout()),this,SLOT(addBox()));

    this->acceleratorTimer = new QTimer();
    connect(this->acceleratorTimer,SIGNAL(timeout()),this,SLOT(addAccelerator()));

    /*Show Game*/

    this->view->show();

    /*Start Timers*/
    this->motionTimer->start(REFRESH_TIME);
    this->boxTimer->start(1000 * BOX_SPAWN_TIME);
    this->acceleratorTimer->start(1000 * ACCELERATOR_SPAWN_TIME);

}

void Game::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_J)
    {
        this->moveL1=true;
    }
    else if(event->key()==Qt::Key_A)
    {
        this->moveL2=true;
    }
    else if(event->key()==Qt::Key_D)
    {
        this->moveR2=true;
    }
    else if(event->key()==Qt::Key_L)
    {
        this->moveR1=true;
    }

    return;
}

void Game::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_J)
    {
        this->moveL1=false;
    }
    else if(event->key()==Qt::Key_A)
    {
        this->moveL2=false;
    }
    else if(event->key()==Qt::Key_D)
    {
        this->moveR2=false;
    }
    else if(event->key()==Qt::Key_L)
    {
        this->moveR1=false;
    }

    return;
}

void Game::mousePressEvent(QMouseEvent *event)
{
    //Boxes Test

    /*
    this->boxes.append(new Box());
    this->scene->addItem(this->boxes.last());
    qDebug()<<this->boxes.last()->scene();
    this->boxes.last()->setPos(event->x(),event->y());
    this->velocify(this->boxes.last()->velocity,1,2,2,3);
    */

    //Accelerators Test


    this->addAccelerator(event->x(),event->y());


    /*Random Effect Test*/

    //this->chooseRandomEffect();

    /*Striker and Goal size Test*/
    //this->multiplyStrikerWidthOfRandomPlayer(this->randomDoubleOrHalfIt());qDebug()<<"Random Striker Width Changed: "<<this->striker1->rect().width()<<","<<this->striker2->rect().width();
    //this->multiplyStrikerWidthOfRandomPlayer(1.1);qDebug()<<"Random Striker Width Changed: "<<this->striker1->rect().width()<<","<<this->striker2->rect().width();
    //this->multiplyStrikerWidthOfRandomPlayer(0.9);qDebug()<<"Random Striker Width Changed: "<<this->striker1->rect().width()<<","<<this->striker2->rect().width();

    //qDebug()<<"g1: "<<this->goal1->x()<<","<<this->goal1->y()<<" OR "<<this->goal1->line().x1()<<","<<this->goal1->line().y1()<<" - "<<this->goal1->line().x2()<<","<<this->goal1->line().y2();
    //qDebug()<<"g2: "<<this->goal2->x()<<","<<this->goal2->y()<<" OR "<<this->goal2->line().x1()<<","<<this->goal2->line().y1()<<" - "<<this->goal2->line().x2()<<","<<this->goal2->line().y2();

    //this->multiplyGoalWidthOfRandomPlayer(0.9);

    return;
}

void Game::stopStrikersAtWallCollision()
{
    if(this->striker1->velocity->getX()>0)
    {
        if(this->striker1->collidesWithItem(this->wallVL)){this->moveL1=false;}
        if(this->striker1->collidesWithItem(this->wallVR)){this->moveR1=false;}
    }

    else
    {
        if(this->striker1->collidesWithItem(this->wallVL)){this->moveR1=false;}
        if(this->striker1->collidesWithItem(this->wallVR)){this->moveL1=false;}
    }

    if(this->striker2->velocity->getX()>0)
    {
        if(this->striker2->collidesWithItem(this->wallVL)){this->moveL2=false;}
        if(this->striker2->collidesWithItem(this->wallVR)){this->moveR2=false;}
    }

    else
    {
        if(this->striker2->collidesWithItem(this->wallVL)){this->moveR2=false;}
        if(this->striker2->collidesWithItem(this->wallVR)){this->moveL2=false;}
    }


    return;
}

void Game::moveStrikers()
{
    if(this->moveL1)
    {
        this->striker1->setPos(this->striker1->x()-this->striker1->velocity->getX(),this->striker1->y());
    }

    if(this->moveL2)
    {
        this->striker2->setPos(this->striker2->x()-this->striker2->velocity->getX(),this->striker2->y());
    }

    if(this->moveR1)
    {
        this->striker1->setPos(this->striker1->x()+this->striker1->velocity->getX(),this->striker1->y());
    }

    if(this->moveR2)
    {
        this->striker2->setPos(this->striker2->x()+this->striker2->velocity->getX(),this->striker2->y());
    }

    //qDebug()<<"1: "<<this->striker1->pos().x()<<","<<this->striker1->pos().y();
    //qDebug()<<"2: "<<this->striker2->pos().x()<<","<<this->striker2->pos().y();
    return;
}


void Game::movePuck()
{
    this->updatePuckVelocity();
    this->updatePuckPosition();
    this->updatePuckAcceleration();
    //qDebug() << "x:"<<this->puck->velocity->getX();
    //qDebug() << "y:"<<this->puck->velocity->getY();
    if(this->didThePuckStop(PUCK_STOP_XV,PUCK_STOP_YV)){this->velocify(this->puck->velocity,MIN_XV_PUCK,MAX_XV_PUCK,MIN_YV_PUCK,MAX_YV_PUCK);}
    //watchout , the ball gets suddenly impulsed

    return;

}

void Game::updatePuckPosition()
{
    this->puck->setX(this->puck->x()+this->puck->velocity->getX()*this->timeStep+0.5*this->puck->acceleration->getX()*this->timeStep*this->timeStep);
    this->puck->setY(this->puck->y()+this->puck->velocity->getY()*this->timeStep+0.5*this->puck->acceleration->getY()*this->timeStep*this->timeStep);
    return;
}

void Game::updatePuckVelocity()
{
    this->puck->velocity->setX(this->puck->velocity->getX()+this->puck->acceleration->getX()*this->timeStep);
    this->puck->velocity->setY(this->puck->velocity->getY()+this->puck->acceleration->getY()*this->timeStep);
    return;
}

void Game::scoreAtGoalCollision()
{
    if(this->puck->collidesWithItem(this->goal1)){/*qDebug()<<"goal1"*/;this->goalAt1=true;}
    if(this->puck->collidesWithItem(this->goal2)){/*qDebug()<<"goal2"*/;this->goalAt2=true;}
}

bool Game::isItemOutside(QGraphicsItem * item)
{
    /*
     * Notice this->puck->x/y are given in "item coordinates relative to the position they were initialized
     * (the origin of the scene)" So these coomparisons are relative to the origin of the scene
    */

    if(item->y()<0 - this->boundary){return true;}
    if(item->y()>this->height + this->boundary){return true;}
    if(item->x()<0 - this->boundary){return true;}
    if(item->x()>this->width + this->boundary){return true;}
    return false;
}

void Game::updatePuckAcceleration()
{
    /*Clear Previous acceleration to recalculate it*/
    this->puck->acceleration->setX(0);
    this->puck->acceleration->setY(0);

    /*For simplicity the formula is simplified as just a drag proportional to the velocity*/
    this->puck->acceleration->setX(-1*this->puck->velocity->getX()*this->field->viscosity);
    this->puck->acceleration->setY(-1*this->puck->velocity->getY()*this->field->viscosity);

    double dummyAcceleration;
    double dummyAngle;

    for(int i=0; i<this->accelerators.size();i++)
    {
        dummyAcceleration = this->accelerators.at(i)->mass / squaredDistanceToPuck(this->accelerators.at(i)->x(),this->accelerators.at(i)->y());
        dummyAngle = this->angleToPuck(this->accelerators.at(i)->x(),this->accelerators.at(i)->y());

        //qDebug()<<"magnitude:"<<dummyAcceleration;
        //qDebug()<<"angle:"<<dummyAngle;
        this->puck->acceleration->setX(this->puck->acceleration->getX() + dummyAcceleration*cos(dummyAngle));
        this->puck->acceleration->setY(this->puck->acceleration->getY() + dummyAcceleration*sin(dummyAngle));
    }

    //qDebug()<<"xA:"<<this->puck->acceleration->getX();
    //qDebug()<<"yA:"<<this->puck->acceleration->getY();
    return;

}

void Game::centerPuck()
{
    /*Center Puck , Notice items coordinates, Puck was initialized at the origin of the scene*/

    this->puck->setX(this->scene->width()/2);
    this->puck->setY(this->scene->height()/2);

    return;

}

void Game::markGoalAndRestart()
{
    if(this->isItemOutside(this->puck))
    {
        //qDebug()<<"outside";
        this->centerPuck();
        this->velocify(this->puck->velocity,MIN_XV_PUCK,MAX_XV_PUCK,MIN_YV_PUCK,MAX_YV_PUCK);
        /*Put here score register*/
        /*We increase the score of the other player*/
        if(this->goalAt1){this->score2->increase();this->changeGoalWidth(this->goal2,this->striker2,1.1);this->changeGoalWidth(this->goal1,this->striker1,0.9);}
        if(this->goalAt2){this->score1->increase();this->changeGoalWidth(this->goal1,this->striker1,1.1);this->changeGoalWidth(this->goal2,this->striker2,0.9);}
        this->goalAt1=false;
        this->goalAt2=false;
    }

    return;
}

bool Game::didThePuckStop(qreal minX, qreal minY)
{
    if(qFabs(this->puck->velocity->getX()) < minX && qFabs(this->puck->velocity->getY()) < minY)
    {
        return true;
    }
    else
    {
        return false;
    }
}

double Game::squaredDistanceToPuck(qreal x, qreal y)
{
    /*Notice that both items need to have the same item origin*/
    return (((this->puck->x()-x)*(this->puck->x()-x))+((this->puck->y()-y)*(this->puck->y()-y)));
}

double Game::angleToPuck(qreal x, qreal y)
{
    return qAtan2(y - this->puck->y(),x-this->puck->x());
}

int Game::signRandomizer()
{    
    int dummy;

    while(1)
    {
        //remember the upper bound is exclusive
        dummy = QRandomGenerator::global()->bounded(-1,2);

        if(dummy != 0){break;}
    }

    //qDebug()<<dummy;
    return dummy;

}

qreal Game::boundedRandomizer(int min, int max)
{
    return QRandomGenerator::global()->bounded(min,max + 1);
}

void Game::velocify(VectorXY * velocity, int minX, int maxX, int minY, int maxY)
{

    velocity->setY(this->boundedRandomizer(minY,maxY)*this->signRandomizer());

    //set low x velocity for it to be more frontal
    velocity->setX(this->boundedRandomizer(minX,maxX)*this->signRandomizer());

    return;
}

void Game::posify(QGraphicsItem *item, int minX, int maxX, int minY, int maxY)
{    
    //item->setPos(this->boundedRandomizer(minX,maxX)*this->signRandomizer(),this->boundedRandomizer(minY,maxY)*this->signRandomizer());
    item->setPos(this->boundedRandomizer(minX,maxX),this->boundedRandomizer(minY,maxY));
    return;
}

void Game::moveEverything()
{
    this->movePuck();
    /* Everything else has constant velocity so we only need to update the position*/
    for(int i = 0; i < this->accelerators.size(); i++)
    {
        this->moveItem(this->accelerators.at(i),this->accelerators.at(i)->velocity);

        /*Delete Item if it is outside*/
        if(this->isItemOutside(this->accelerators.at(i)))
        {
            //this->scene->removeItem(this->accelerators.at(i));
            //delete this->accelerators.at(i);
            this->deleteAccelerator(this->accelerators.at(i));
            this->accelerators.removeAt(i);
            qDebug()<<"Accelerator Deleted";
        }
    }

    for(int i = 0; i < this->boxes.size(); i++)
    {
        this->moveItem(this->boxes.at(i),this->boxes.at(i)->velocity);

        /*Delete Item if it is outside*/
        if(this->isItemOutside(this->boxes.at(i)))
        {
            //this->scene->removeItem(this->Boxes.at(i));
            //delete this->Boxes.at(i);
            this->deleteBox(this->boxes.at(i));
            this->boxes.removeAt(i);
            qDebug()<<"Box Deleted";
        }
    }

    return;

}

void Game::bounceEverything()
{
    this->bounceFromStrikers(this->puck,this->puck->velocity);
    this->bounceFromWalls(this->puck,this->puck->velocity);

    for(int i = 0; i < this->accelerators.size(); i++)
    {
        this->bounceFromWalls(this->accelerators.at(i),this->accelerators.at(i)->velocity);
        this->bounceFromStrikers(this->accelerators.at(i),this->accelerators.at(i)->velocity);
    }

    for(int i = 0; i < this->boxes.size(); i++)
    {
        this->bounceFromWalls(this->boxes.at(i),this->boxes.at(i)->velocity);
        this->bounceFromStrikers(this->boxes.at(i),this->boxes.at(i)->velocity);
    }

    return;

}

void Game::bounceFromWalls(QGraphicsItem *item, VectorXY * velocity)
{
    if(item->collidesWithItem(this->wallHD)&&!item->collidesWithItem(this->goal1)){velocity->setY(-1*velocity->getY()*this->wallHD->getRestitution());}
    if(item->collidesWithItem(this->wallHU)&&!item->collidesWithItem(this->goal2)){velocity->setY(-1*velocity->getY()*this->wallHU->getRestitution());}
    if(item->collidesWithItem(this->wallVL)){velocity->setX(-1*velocity->getX()*this->wallVL->getRestitution());}
    if(item->collidesWithItem(this->wallVR)){velocity->setX(-1*velocity->getX()*this->wallVR->getRestitution());}
    return;
}

void Game::bounceFromStrikers(QGraphicsItem *item, VectorXY *velocity)
{
    if(item->collidesWithItem(this->striker1)){velocity->setY(-1*velocity->getY()*this->striker1->getRestitution());}
    if(item->collidesWithItem(this->striker2)){velocity->setY(-1*velocity->getY()*this->striker2->getRestitution());}
    return;

}

void Game::moveItem(QGraphicsItem *item, VectorXY *velocity)
{
    /*This assumes constant velocity*/

    item->setPos(item->x()+velocity->getX(),item->y()+velocity->getY());

    return;
}

void Game::deleteBox(Box * box)
{
    this->scene->removeItem(box);
    delete box;
    return;
}

void Game::deleteAccelerator(Accelerator *accel)
{
    this->scene->removeItem(accel);
    delete accel;
    return;
}

void Game::BoxesCollidingWithPuck()
{
    for (int i = 0, n = this->puck->collidingItems().size();i < n;i++)
    {
        if(typeid (*(this->puck->collidingItems().at(i)))==typeid(Box))
        {
            this->boxes.removeOne((Box *)(this->puck->collidingItems().at(i)));
            this->deleteBox((Box *)(this->puck->collidingItems().at(i)));
            qDebug()<<"Box Deleted by Touch";
            this->chooseRandomEffect();
            return;
        }
    }
}

void Game::chooseRandomEffect()
{
    int effect = this->boundedRandomizer(1,10);
    qDebug()<<"Effect: "<<effect;
    QString comment;
    switch (effect)
    {
    case 1:
    {
        this->addAccelerator(this->boundedRandomizer(0,this->width),this->boundedRandomizer(0,this->height));
        comment = QString("Accelerator Added");
        this->narrator->narrate(comment);
        break;
    }
    case 2:
    {
        this->velocify(this->puck->velocity,MIN_XV_PUCK,MAX_XV_PUCK,MIN_YV_PUCK,MAX_YV_PUCK);
        comment = QString("Puck Velocified");
        this->narrator->narrate(comment);
        break;
    }
    case 3:
    {
        this->field->setViscosity(this->boundedRandomizer(1,100)*0.001);
        comment = QString("Field Viscosity Changed to: ") + QString::number(this->field->viscosity);
        this->narrator->narrate(comment);
        QTimer::singleShot(VISCOSITY_TIME*1000,this,SLOT(restoreFieldViscosity()));
        break;
    }
    case 4:
    {
        this->setPuckColor(FIELD_COLOR);//Qt::transparent? What if pucl is over somethinf else?
        comment = QString("Puck Invisible");
        this->narrator->narrate(comment);
        QTimer::singleShot(INVISIBLE_PUCK_TIME *1000,this,SLOT(setPuckVisible()));
        break;
    }
    case 5:
    {
        this->multiplyStrikerWidthOfRandomPlayer(this->random10PercentMoreOrLess());
        //qDebug()<<"Random Striker Width Changed: "<<this->striker1->rect().width()<<","<<this->striker2->rect().width();
        comment = QString("Random Striker Width Changed (if possible)");
        this->narrator->narrate(comment);
        break;
    }
    case 6:
    {
        this->multiplyGoalWidthOfRandomPlayer(this->random10PercentMoreOrLess());
        comment = QString("Random Goal Width Changed");
        this->narrator->narrate(comment);
        break;
    }
    case 7:
    {
        this->randomStrikerVelocityForRandomPlayer();
        comment = QString("Random Striker Velocity For Random Player");
        this->narrator->narrate(comment);
        break;
    }
    case 8:
    {
        this->negateRandomPlayerStrikerVelocity();
        comment = QString("Negate Striker Velocity for Random Player");
        this->narrator->narrate(comment);
        break;
    }
    case 9:
    {
        qreal r = this->randomRestitutionForAllWalls();
        comment = QString("Walls with restitution of ") + QString::number(r)+QString(" for ")+QString::number(WALL_RESTITUTION_TIME)+QString(" seconds");
        this->narrator->narrate(comment);
        QTimer::singleShot(WALL_RESTITUTION_TIME*1000,this,SLOT(restoreWallRestitution()));
        break;
    }
    case 10:
    {
        qreal r = this->randomRestitutionForAllPlayers();
        comment = QString("Strikers with restitution of ") + QString::number(r)+QString(" for ") + QString::number(STRIKER_RESTITUTION_TIME) + QString(" seconds");
        this->narrator->narrate(comment);
        QTimer::singleShot(STRIKER_RESTITUTION_TIME*1000,this,SLOT(restoreStrikersRestitution()));
        break;
    }

    default:
        this->narrator->narrate("Why are you here?");
        break;
    }
}

void Game::addAccelerator(qreal x, qreal y)
{
    this->accelerators.append(new Accelerator(ACCELERATOR_RADIUS,this->signRandomizer()*ACCELERATOR_MASS,Qt::SolidPattern,Qt::white,0,0,0,0));
    this->scene->addItem(this->accelerators.last());
    this->accelerators.last()->setPos(x,y);
    this->velocify(this->accelerators.last()->velocity,MIN_XV_AandB,MAX_XV_AandB,MIN_YV_AandB,MAX_YV_AandB);
    this->accelerators.last()->paintAccelerator();
    return;
}

void Game::setPuckColor(Qt::GlobalColor color)
{
        this->puck->setColor(color);
        qDebug()<<"Puck colored";
        return;
}

void Game::multiplyStrikerWidthOfRandomPlayer(qreal gain)
{
    int player = this->generateRandomPlayer();
    qDebug()<<"player: "<<player<<" gain: "<<gain<<" 1: "<<this->striker1->rect().width()<<" 2: "<<this->striker2->rect().width();
    if(player == 1){this->changeStrikerWidth(this->striker1,this->goal1,gain);}
    else if(player == 2){this->changeStrikerWidth(this->striker2,this->goal2,gain);}
    return;
}

qreal Game::random10PercentMoreOrLess()
{
    int dummy = this->boundedRandomizer(1,2);
    if(dummy == 1){qDebug()<<"gain: "<<0.9;return 0.9;}
    if(dummy == 2){qDebug()<<"gain: "<<1.1;return 1.1;}
    return 1;
}

void Game::changeStrikerWidth(Striker *striker, Goal * goal, qreal gain)
{
    if(gain > 1)
    {
        if (MIN_GOAL_TO_STRIKER_RATIO*striker->rect().width() < goal->getWidth()) //max is less 1.5 of the goal
        {
            striker->setWidth(striker->rect().width()*gain);
            return;
        }
        else
        {
            return;
        }
    }
    else if (gain < 1)
    {
        if (striker->rect().width() > this->width/32) //min is width/32
        {
            striker->setWidth(striker->rect().width()*gain);
        }
        else
        {
            return;
        }
    }

    return;
}

void Game::multiplyGoalWidthOfRandomPlayer(qreal gain)
{
    int player = this->generateRandomPlayer();
    qDebug()<<"player: "<<player<<" gain: "<<gain<<" 1: "<<this->goal1->width<<" 2: "<<this->goal2->width;
    if(player == 1){this->changeGoalWidth(this->goal1,this->striker1,gain);}
    else if(player == 2){this->changeGoalWidth(this->goal2,this->striker2,gain);}
    return;

}


void Game::changeGoalWidth(Goal * goal,Striker * striker, qreal gain)
{
    if(gain > 1)
    {
        if (goal->getWidth() < this->width) //max is width/2 (whole width)
        {
            goal->setWidth(goal->getWidth()*gain);
            return;
        }
        else
        {
            return;
        }
    }
    else if (gain < 1)
    {
        if (goal->getWidth() > MIN_GOAL_TO_STRIKER_RATIO * striker->rect().width()) //min is determined by striker width
        {
            goal->setWidth(goal->getWidth()*gain);
        }
        else
        {
            return;
        }
    }

    return;

}

int Game::generateRandomPlayer()
{
    return this->boundedRandomizer(1,2);
}

void Game::randomStrikerVelocityForRandomPlayer()
{
    int player = this->generateRandomPlayer();
    if(player==1){this->striker1->velocity->setX(this->boundedRandomizer(MIN_STRIKER_VELOCITY,MAX_STRIKER_VELOCITY));}
    if(player==2){this->striker2->velocity->setX(this->boundedRandomizer(MIN_STRIKER_VELOCITY,MAX_STRIKER_VELOCITY));}
    return;
}

void Game::negateRandomPlayerStrikerVelocity()
{
    int player = this->generateRandomPlayer();
    if(player==1){this->striker1->negateVelocity();}
    else
    if(player==2){this->striker2->negateVelocity();}
    return;
}

qreal Game::randomRestitutionForAllWalls()
{
    qreal r = this->boundedRandomizer(1,10)*0.1;
    this->wallHD->setRestitution(1+r);
    this->wallHU->setRestitution(1+r);
    this->wallVL->setRestitution(1+r);
    this->wallVR->setRestitution(1+r);
    return 1+r;
}

qreal Game::randomRestitutionForAllPlayers()
{
    qreal r = this->boundedRandomizer(1,10)*0.1;
    this->striker1->setRestitution(1+r);
    this->striker2->setRestitution(1+r);
    return 1+r;
}

void Game::botsify(Striker * striker)
{
    if(striker->pos().x()+striker->rect().width()/2 > this->puck->pos().x() && !striker->collidesWithItem(this->wallVL)){striker->setX(striker->x()-qFabs(striker->velocity->getX()));}
    if(striker->pos().x()+striker->rect().width()/2 < this->puck->pos().x() && !striker->collidesWithItem(this->wallVR)){striker->setX(striker->x()+qFabs(striker->velocity->getX()));}
    return;
}


Game::~Game()
{

}

void Game::animate()
{
    this->botsify(this->striker2);
    this->botsify(this->striker1);
    this->BoxesCollidingWithPuck();
    this->scoreAtGoalCollision();
    this->markGoalAndRestart();
    this->bounceEverything();
    this->moveEverything();
    this->stopStrikersAtWallCollision();
    this->moveStrikers();

}

void Game::addBox()
{

    this->boxes.append(new Box());
    this->scene->addItem(this->boxes.last());
    this->posify(this->boxes.last(),0+this->boxes.last()->boundingRect().width(),this->width-this->boxes.last()->boundingRect().width(),0+this->boxes.last()->boundingRect().height(),this->height-this->boxes.last()->boundingRect().height());
    this->velocify(this->boxes.last()->velocity,MIN_XV_AandB,MAX_XV_AandB,MIN_YV_AandB,MAX_YV_AandB);

    return;

}

void Game::setPuckVisible()
{
    this->puck->setColor(PUCK_COLOR);
    this->narrator->narrate(QString("Puck Visible"));
    return;
}

void Game::restoreFieldViscosity()
{
    this->field->setViscosity(FIELD_VISCOSITY);
    this->narrator->narrate(QString("Field Viscosity Restored"));
    return;
}

void Game::restoreWallRestitution()
{
    this->wallHD->setRestitution(1);
    this->wallHU->setRestitution(1);
    this->wallVL->setRestitution(1);
    this->wallVR->setRestitution(1);
    this->narrator->narrate(QString("Walls Restitution Restored"));
    return;
}

void Game::restoreStrikersRestitution()
{
    this->striker1->setRestitution(1);
    this->striker2->setRestitution(1);
    this->narrator->narrate(QString("Strikers Restitution Restored"));
    return;
}

void Game::addAccelerator()
{
    this->accelerators.append(new Accelerator(ACCELERATOR_RADIUS,this->signRandomizer()*ACCELERATOR_MASS,Qt::SolidPattern,Qt::white,0,0,0,0));
    this->scene->addItem(this->accelerators.last());
    this->posify(this->accelerators.last(),0,this->width,0,this->height);
    this->velocify(this->accelerators.last()->velocity,MIN_XV_AandB,MAX_XV_AandB,MIN_YV_AandB,MAX_YV_AandB);
    this->accelerators.last()->paintAccelerator();
    return;
}
