#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <ctime>
#include <cmath>

using namespace std;
using namespace sf;

double user_health;
const int Hunt_Num = 30;
const double hunt_boost=1.75;
const int user_speed = 6;
const float screenWid = 4000;
const float screenHei = 2400;
RenderWindow window(VideoMode(screenWid,screenHei), "Let's Play!");
CircleShape user_blip(20);

class Hunter: public sf::Drawable, public sf::Transformable
{
    sf::Texture m_tileset;
    double accuracy, speed, radius, damage;
    sf::Color color;

    public:

    void setAccuracy(double);
    void setSpeed(double);
    void setRadius(double);
    void setDamage(double);
    void setFillColor(sf::Color);

    double getAccuracy();
    double getSpeed();
    double getRadius();
    double getDamage();
    sf::Color getFillColor();

    Hunter();
    Hunter(double,double,double,double,sf::Color);

    private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        states.transform *= getTransform();

        states.texture = &m_tileset;

        sf::CircleShape shape(radius);
        shape.setFillColor(color);

        target.draw(shape, states);
    }
};

void Hunter::setAccuracy(double a){
    accuracy=a;
};
void Hunter::setSpeed(double a){
    speed=a;
};
void Hunter::setRadius(double a){
    radius=a;
};
void Hunter::setDamage(double a){
    damage=a;
};
void Hunter::setFillColor(sf::Color a){
    color=a;
};
double Hunter::getAccuracy(){
    return accuracy;
};
double Hunter::getSpeed(){
    return speed;
};
double Hunter::getRadius(){
    return radius;
};
double Hunter::getDamage(){
    return damage;
};
sf::Color Hunter::getFillColor(){
    return color;
};
Hunter::Hunter(){
    accuracy=20;
    speed=2.4;
    radius=6;
    damage=.2;
    color=sf::Color::White;
};
Hunter::Hunter(double a,double b,double c,double d,sf::Color e) {
    accuracy=a;
    speed=b;
    radius=c;
    damage=d;
    color=e;
};

void hunt_collision(Hunter hunters[],double h_move[Hunt_Num][2],vector<string>&run_tog, int tog_counter[])
{
    int i,j,collide_once;
    double dist, rad;
    sf::Vector2f hunt_xy;
    sf::Vector2f hunt_xy_2;

    for (i=0; i<Hunt_Num; i++)
    {
        collide_once=0;
        hunt_xy=hunters[i].getPosition();
        for (j=0; j<Hunt_Num; j++)//finds the distance between two hunters
        {
            if (i==j)
            {
                //skip action, if both loops refer to same hunter
            }
            else if (collide_once==0)//only check the current hunter if no collision has occurred
            {

                hunt_xy_2=hunters[j].getPosition();
                dist=sqrt(((hunt_xy_2.x-hunt_xy.x)*(hunt_xy_2.x-hunt_xy.x)+(hunt_xy_2.y-hunt_xy.y)*(hunt_xy_2.y-hunt_xy.y))+1);
                rad=hunters[i].getRadius()+hunters[j].getRadius();
                if (dist<=rad)
                {
                    collide_once++;//ensures position is reversed only for one collision (don't want it to collide with one, reverse,
                                    //then collide simultaneously with another and reverse back, negating initial reversal).
                    if (hunters[i].getRadius()<=hunters[j].getRadius())//if the distance between two hunters is too close, reverse the "i" hunter's direction
                    {                                                                   //Also, only the smaller hunter reports a collision and will reverse directions (in hunters_move)
                        h_move[i][0]=h_move[i][0]*-1;
                        h_move[i][1]=h_move[i][1]*-1;
                        run_tog[i]="run"; tog_counter[i]=0;
                    }
                    if (hunters[i].getFillColor()==sf::Color::Blue)
                    {
                        hunters[i].setSpeed(hunters[i].getSpeed()+(hunters[j].getSpeed()/2));
                        hunters[i].setDamage(hunters[i].getDamage()+(hunters[j].getDamage()/2));
                        hunters[i].setAccuracy(hunters[i].getAccuracy()+(hunters[j].getAccuracy()*10));
                        hunters[i].setRadius(hunters[i].getRadius()+(hunters[j].getRadius()/5));
                        hunters[j].setAccuracy(0);
                        if (hunters[j].getFillColor()==sf::Color::Blue)
                        {
                            hunters[j].setPosition(10000+100*j,10000+100*j);
                            hunters[j].setFillColor(sf::Color::White);
                            hunters[j].setRadius(.1);
                        }
                        else
                        {
                            hunters[j].setPosition(12000,12000);
                        }
                    }
                }
            }
        }
    }
}

void hunter_hits_user(Hunter hunt_hit,double& user_health) {//tracks damage
    user_health=user_health-hunt_hit.getDamage();
}

void hunters_move(sf::CircleShape user, Hunter hunters[], double h_move[Hunt_Num][2], int& counter,vector<string>&run_toggle,int tog_counter[],double& health)
{
    double y_dist, x_dist;
    sf::Vector2f target_xy=user.getPosition();
    sf::Vector2f hunt_xy;
    int hunt_check,dist,i;

    if(counter==100)
    {
        counter=0;
    }
    for(i=0; i<Hunt_Num; i++)
    {
        if(counter==0)
        {
            run_toggle[i]="chase";
            h_move[i][0]=(rand()%3-1)*hunters[i].getSpeed();
            h_move[i][1]=(rand()%3-1)*hunters[i].getSpeed();
        }

        hunt_xy=hunters[i].getPosition();

        dist=sqrt(((target_xy.x-hunt_xy.x)*(target_xy.x-hunt_xy.x)+(target_xy.y-hunt_xy.y)*(target_xy.y-hunt_xy.y))+1);

        if (((counter%20==0) && run_toggle[i]=="chase"))//setting h_move to -1,0,or 1 ==> move negative direction, stay put, or go positive direction
        {                               //counter provides a delay between re-randomizing h_move, to prevent jerky motion
            h_move[i][0]=(rand()%3-1)*hunters[i].getSpeed();
            h_move[i][1]=(rand()%3-1)*hunters[i].getSpeed();
        }
        if(tog_counter[i]==60)
        {
            run_toggle[i]="chase";
            tog_counter[i]=0;
        }
        if (dist>3000)//sit still, if the distance is too great
        {
            h_move[i][0]=0;
            h_move[i][1]=0;
        }
        hunt_check=((rand()%dist+1)/(hunters[i].getAccuracy()))+1;//establish a number range based on the hunter's distance from user (divided by ten, to shorten range).

        if (hunt_check==1 && run_toggle[i]=="chase")
        {
            x_dist=hunt_xy.x-target_xy.x;
            y_dist=hunt_xy.y-target_xy.y;

            if(x_dist>0) //if the hunter is to the right of the user, move it 10 pixels to the left (closer).
                                        //else, if the hunter is to the left, move it 10 pixels to the right (also closer)
            {
                h_move[i][0]=-1*hunters[i].getSpeed();
            }
            else if(x_dist<0)
            {
                h_move[i][0]=hunters[i].getSpeed();
            }
            if(y_dist>0) //if the hunter is above the user, move it 10 pixels down (closer).
                                        //else, if the hunter is below, move it 10 pixels up(also closer)
            {
                h_move[i][1]=-1*hunters[i].getSpeed();
            }
            else if(y_dist<0)
            {
                h_move[i][1]=hunters[i].getSpeed();
            }
        }
        hunt_collision(hunters,h_move,run_toggle,tog_counter);//checks for collisions between hunters and then reassigns h_move if they do collide

        if (dist<(hunters[i].getRadius()+user.getRadius()))
        {
            hunter_hits_user(hunters[i],health);
        }

        hunters[i].move(h_move[i][0],h_move[i][1]);

        if (run_toggle[i]=="run")
        {
            tog_counter[i]++;
        }
    }
    counter++;
}

void user_moves(CircleShape& user_blip, int user_x_speed[2], int user_y_speed[2], int& pondColl, int& treeColl,
char& lastKeyPress, char& twoKeysAgo) {

    sf::Vector2f user_pos=user_blip.getPosition();

         if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            if ((treeColl > 0) && (lastKeyPress == 'L'
            || (lastKeyPress == 'D' && twoKeysAgo == 'L')
            || (lastKeyPress == 'U' && twoKeysAgo == 'L')))
                user_x_speed[1] = 0;
            else if (pondColl > 0)
                user_x_speed[1] = user_speed*-0.5;
            else {
                twoKeysAgo = lastKeyPress;
                lastKeyPress = 'L';
                user_x_speed[1] = user_speed*-1;
            }
            if (user_pos.x<user_blip.getRadius())
            {
                 user_x_speed[1]=0;
            }
            user_blip.move(user_x_speed[1], 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            if ((treeColl > 0) && (lastKeyPress == 'R'
            || (lastKeyPress == 'D' && twoKeysAgo == 'R')
            || (lastKeyPress == 'U' && twoKeysAgo == 'R')))
                user_x_speed[0] = 0;
            else if (pondColl > 0)
                user_x_speed[0] = user_speed*0.5;
            else {
                twoKeysAgo = lastKeyPress;
                lastKeyPress = 'R';
                user_x_speed[0]=user_speed;
            }
            if (user_pos.x>screenWid-user_blip.getRadius())
            {
                 user_x_speed[0]=0;
            }

            user_blip.move(user_x_speed[0], 0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            if ((treeColl > 0) && (lastKeyPress == 'U'
            || (lastKeyPress == 'L' && twoKeysAgo == 'U')
            || (lastKeyPress == 'R' && twoKeysAgo == 'U')))
                user_y_speed[1] = 0;
            else if (pondColl > 0)
                user_y_speed[1] = user_speed*-0.5;
            else {
                twoKeysAgo = lastKeyPress;
                lastKeyPress = 'U';
                user_y_speed[1]=user_speed*-1;
            }
            if (user_pos.y<user_blip.getRadius())
            {
                 user_y_speed[1]=0;
            }
            user_blip.move(0, user_y_speed[1]);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            if ((treeColl > 0) && (lastKeyPress == 'D'
            || (lastKeyPress == 'L' && twoKeysAgo == 'D')
            || (lastKeyPress == 'R' && twoKeysAgo == 'D')))
                user_y_speed[0] = 0;
            else if (pondColl > 0)
                user_y_speed[0] = user_speed*0.5;
            else {
                twoKeysAgo = lastKeyPress;
                lastKeyPress = 'D';
                user_y_speed[0]=user_speed;
            }
            if (user_pos.y>screenHei-user_blip.getRadius())
            {
                 user_y_speed[0]=0;
            }
            user_blip.move(0, user_y_speed[0]);
        }
};

void draw_dynamics(Hunter hunt_array[], sf::CircleShape user_blip, const double user_health) {
    int i;
    for (i=0;i<Hunt_Num;i++)
    {
        window.draw(hunt_array[i]);
    }

    if(user_health>=0)
    {
        sf::Vector2f health_pos=user_blip.getPosition();
        sf::RectangleShape health_bar_wipe(sf::Vector2f(33, 100-user_health));
        health_bar_wipe.setPosition(health_pos.x+532,health_pos.y-323);
        health_bar_wipe.setFillColor(sf::Color::White);
        window.draw(health_bar_wipe);
    }

    window.draw(user_blip);
}

void draw_statics(sf::Font font) {
    sf::Vector2f health_pos=user_blip.getPosition();
    sf::RectangleShape health_bar(sf::Vector2f(33, 100));
    health_bar.setPosition(health_pos.x+532,health_pos.y-323);
    health_bar.setFillColor(sf::Color::Red);

    sf::RectangleShape health_bar_background(sf::Vector2f(37, 104));
    health_bar_background.setFillColor(sf::Color::White);
    health_bar_background.setPosition(health_pos.x+530,health_pos.y-325);

    sf::Text healthtext("Health", font, 16);
    healthtext.setColor(sf::Color::Red);
    healthtext.setPosition(health_pos.x+522,health_pos.y-220);

    window.draw(healthtext);
    window.draw(health_bar_background);
    window.draw(health_bar);
}

void set_user_blip(sf::CircleShape& blip,int user_x_speed[2],int user_y_speed[2]) {
    blip.setFillColor(sf::Color::Black);
    blip.setOrigin(20,20);
    blip.setPosition(500,500);
    //give user some default speed values
    user_x_speed[0]=user_speed; user_x_speed[1]=user_speed*-1;
    user_y_speed[0]=user_speed; user_y_speed[1]=user_speed*-1;
}


void make_hunters(Hunter hunt_array[Hunt_Num]){

    double rad, hunt_x, hunt_y;
    int i, height, width;
    sf::Vector2f user_xy;

    hunt_array[0]=Hunter(100,1.5,25,1,sf::Color::Red);
    rad=hunt_array[0].getRadius();
    hunt_array[0].setOrigin(rad,rad);

    hunt_array[1]=Hunter(100,1.5,25,1,sf::Color::Red);
    rad=hunt_array[1].getRadius();
    hunt_array[1].setOrigin(rad,rad);

    hunt_array[2]=Hunter(100,1.5,25,1,sf::Color::Red);
    rad=hunt_array[2].getRadius();
    hunt_array[2].setOrigin(rad,rad);

    hunt_array[3]=Hunter(10,.4,3,.01,sf::Color::Blue);
    rad=hunt_array[3].getRadius();
    hunt_array[3].setOrigin(rad,rad);

    hunt_array[4]=Hunter(10,.4,3,.01,sf::Color::Blue);
    rad=hunt_array[4].getRadius();
    hunt_array[4].setOrigin(rad,rad);

    hunt_array[5]=Hunter(10,.4,3,.01,sf::Color::Blue);
    rad=hunt_array[5].getRadius();
    hunt_array[5].setOrigin(rad,rad);

    hunt_array[6]=Hunter(100,1.5,25,1,sf::Color::Red);
    rad=hunt_array[6].getRadius();
    hunt_array[6].setOrigin(rad,rad);

    hunt_array[7]=Hunter(100,1.5,25,1,sf::Color::Red);
    rad=hunt_array[7].getRadius();
    hunt_array[7].setOrigin(rad,rad);

    hunt_array[8]=Hunter(100,1.5,25,1,sf::Color::Red);
    rad=hunt_array[8].getRadius();
    hunt_array[8].setOrigin(rad,rad);

    hunt_array[9]=Hunter(100,1.5,25,1,sf::Color::Red);
    rad=hunt_array[9].getRadius();
    hunt_array[9].setOrigin(rad,rad);

    hunt_array[10]=Hunter(100,1.5,25,1,sf::Color::Red);
    rad=hunt_array[10].getRadius();
    hunt_array[10].setOrigin(rad,rad);

    hunt_array[30]=Hunter();

    user_xy=user_blip.getPosition();
    width=screenWid;
    height=screenHei;

    for (i=0; i<Hunt_Num; i++)
    {
        hunt_x=rand()% width;
        hunt_y=rand()% height;

        while (abs(hunt_x-user_xy.x)<100 || abs(hunt_y-user_xy.y)<100)
        {
            hunt_x=rand()% width;
            hunt_y=rand()% height;
        }
        hunt_array[i].setPosition(hunt_x,hunt_y);
    }

    for (i=0; i<Hunt_Num; i++)
    {
        hunt_array[i].setSpeed(hunt_array[i].getSpeed()*hunt_boost);
        hunt_array[i].setAccuracy(hunt_array[i].getAccuracy()*hunt_boost);
        hunt_array[i].setDamage(hunt_array[i].getDamage()*hunt_boost);
    }
}

void buildEnvir(vector<CircleShape>& pondGroup, vector<RectangleShape>& treeGroup,
vector<Vector2f>& treeCoords, vector<Vector2f>& pondCoords) {
    double pondRad = 45;        //set up pond
    CircleShape pond(pondRad);
    pond.setOrigin(pondRad,pondRad);
    pond.setFillColor(Color(0,102,224));
    pond.setScale(1.2f, 1.0f);
    const double treeWid = 20;      //set up tree trunks
    const double treeHei = 100;
    RectangleShape tree(Vector2f(treeWid,treeHei));
    tree.setFillColor(Color(102,51,0));
    tree.setOrigin(treeWid/2,treeHei/2);
    tree.setOrigin(treeWid/2,treeHei/2);
    double ranX, ranY, dist;
    int ranPondWid = screenWid-pondRad*2;   //set range of random coordinates within screen that objects can be placed
    int ranPondHei = screenHei-pondRad*2;
    int ranTreeWid = screenWid-treeWid;
    int ranTreeHei = screenHei-treeHei;
    int intersect;
    int ranPondNum = rand() % 20 + 4;   //create random num of ponds between 1-3
    int ranTreeNum = rand() % 300 + 20;   //create random num of tree trunks between 1-10
    for (int i=0; i<ranPondNum; i++) {
        pondGroup.push_back(pond);
        do {                        //set random positions for ponds and make sure they don't overlap
            intersect = 0;
            ranX = rand() % ranPondWid + pondRad;
            ranY = rand() % ranPondHei + pondRad;
            pondGroup[i].setPosition(ranX, ranY);
            for (int j=0; j<i; j++) {
                dist = sqrt((abs(pondGroup[i].getPosition().x-pondGroup[j].getPosition().x)
                    * abs(pondGroup[i].getPosition().x-pondGroup[j].getPosition().x))
                    + (abs(pondGroup[i].getPosition().y-pondGroup[j].getPosition().y)
                    * abs(pondGroup[i].getPosition().y-pondGroup[j].getPosition().y)));
                if (dist <= (pondRad*1.2*2))
                    intersect++;
            }
            if (abs(ranX-500) <= 74 && abs(ranY-500) <= 65)
                intersect++;
        } while (intersect>0);
        pondCoords.push_back(pondGroup[i].getPosition());
    }
    for (int i=0; i<ranTreeNum; i++) {
        treeGroup.push_back(tree);
        do {                        //set random position for tree trunks so they aren't overlapping
            intersect = 0;
            ranX = rand() % ranTreeWid + treeWid/2;
            ranY = rand() % ranTreeHei + treeHei/2;
            treeGroup[i].setPosition(ranX, ranY);
            int xdist, ydist;
            for (int j=0; j<i; j++) {
                xdist = abs(treeGroup[i].getPosition().x - treeGroup[j].getPosition().x);
                ydist = abs(treeGroup[i].getPosition().y - treeGroup[j].getPosition().y);
                if (xdist <= 20 && ydist <= 100)
                    intersect++;
            }
            for (int k=0; k<ranPondNum; k++) {
                if ((abs(treeGroup[i].getPosition().x - pondCoords[k].x) <= (treeWid + pondRad*1.2)) &&
                (abs(treeGroup[i].getPosition().y - pondCoords[k].y) <= (treeHei/2 + pondRad)))
                    intersect++;
            }
            if (abs(ranX-500) <= 30 && abs(ranY-500) <= 70)
                intersect++;
        } while (intersect>0);
        treeCoords.push_back(treeGroup[i].getPosition());
    }
}

void drawStars(vector<CircleShape>& starGroup, vector<Vector2f>& pondCoords,
vector<Vector2f>& treeCoords, vector<Vector2f>& starCoords) {
    CircleShape star(5);        //set up "star" attributes
    star.setOrigin(5,5);
    star.setFillColor(Color(230,230,0));
    int intersect;
    double ranX, ranY;
    float dist;
    int ranStarWid = screenWid - 10;
    int ranStarHei = screenHei - 10;
    for (int i=0; i<25; i++) {
        starGroup.push_back(star);
        do {                        //check that stars aren't overlapping
            intersect = 0;
            ranX = rand() % ranStarWid + 5;
            ranY = rand() % ranStarHei + 5;
            starGroup[i].setPosition(ranX, ranY);
            for (int j=0; j<pondCoords.size(); j++) {       //check that stars don't overlap with ponds
                dist = sqrt(abs(ranX-pondCoords[j].x) * abs(ranX-pondCoords[j].x)
                    + abs(ranY-pondCoords[j].y)*abs(ranY-pondCoords[j].y));
                if (dist <= (54+5))
                    intersect++;
            }
            for (int k=0; k<treeCoords.size(); k++) {       //check that stars don't overlap with trees
                if (abs(ranX-treeCoords[k].x) <= 15 && abs(ranY-treeCoords[k].y) <= 55)
                    intersect++;
            }
        } while (intersect>0);
        starCoords.push_back(starGroup[i].getPosition());
    }
}

void drawTree(vector<CircleShape>& treeTop, vector<RectangleShape>& treeGroup) {
    double leafRad = 20;        //draw leaves for tops of trees
    Vector2f position;
    CircleShape leaf(leafRad);
    leaf.setFillColor(Color::Green);
    leaf.setOrigin(leafRad, leafRad);
    int counter = 0;
    for (int j=0; j<treeGroup.size(); j++) {        //three groups of leaves per tree
        for (int i=0; i<3; i++) {
            treeTop.push_back(leaf);
        }
        position = treeGroup[j].getPosition();
        treeTop[counter].setPosition(position.x-15,position.y-20);
        treeTop[counter+1].setPosition(position.x+15,position.y-20);
        treeTop[counter+2].setPosition(position.x,position.y-40);
        counter = counter+3;
    }
}

void checkCollision(vector<Vector2f>& treeCoords, vector<Vector2f>& pondCoords, vector<Vector2f>& starCoords,
vector<CircleShape>& starGroup, CircleShape& user_blip, bool& isCollision,
int& pondColl, int& treeColl, int& starColl, Music& ding, Music& crash, Music& splash, int& starCount, sf::Font font) {
    Vector2f userPos = user_blip.getPosition();         //get user blip position to check if it collides with pond, trees, and stars
    double pondDist, starDist;
    pondColl = 0;
    treeColl = 0;
    starColl = 0;
    for (int i=0; i<pondCoords.size(); i++) {       //check for collision with pond
        pondDist = sqrt((abs(pondCoords[i].x-userPos.x)*abs(pondCoords[i].x-userPos.x))
                + (abs(pondCoords[i].y-userPos.y)*abs(pondCoords[i].y-userPos.y)));
        if (pondDist <= 70) {
            isCollision = true;
            pondColl++;
            splash.play();
        }
        else
            isCollision = false;
    }
    for (int j=0; j<treeCoords.size(); j++) {       //check for collision with tree
        if ((abs(userPos.x - treeCoords[j].x) <= 30) && (abs(userPos.y - treeCoords[j].y) <= 70)) {
            treeColl++;
            isCollision = true;
            crash.play();
        }
        else
            isCollision = false;
    }
    for (int k=0; k<starCoords.size(); k++) {       //check for collision with star
        starDist = sqrt((abs(starCoords[k].x-userPos.x)*abs(starCoords[k].x-userPos.x))
                + (abs(starCoords[k].y-userPos.y)*abs(starCoords[k].y-userPos.y)));
        if (starDist <= 25) {
            starColl++;
            starCount++;
            ding.play();
            starGroup[k].setPosition(10000,10000);
            starCoords[k] = starGroup[k].getPosition();
            user_health=user_health+10;
            if (user_health>100)
            {
                user_health=100;
            }
        }
    }
    Vector2f starCount_pos=user_blip.getPosition();
    string a = to_string(starCount);
    Text starText("Star count: " + a, font, 16);
    starText.setPosition(starCount_pos.x + 500, starCount_pos.y -195);
    starText.setColor(Color::Blue);
    window.draw(starText);
}

void drawEnvir(vector<CircleShape>& pondGroup, vector<RectangleShape>& treeGroup, vector<CircleShape>& treeTop,vector<CircleShape>& starGroup) {
    for (int i=0; i<pondGroup.size(); i++)
        window.draw(pondGroup[i]);
    for (int j=0; j<treeGroup.size(); j++)
        window.draw(treeGroup[j]);
    for (int k=0; k<treeTop.size(); k++)
        window.draw(treeTop[k]);
    for (int l=0; l<starGroup.size(); l++)
        window.draw(starGroup[l]);
}

void intro_text(Font font, Music& intro)
{
    window.clear(Color::White);
    Text text("Welcome to our game! \n \n Try to gobble 10 yellow 'stars' using the \n arrows before the 'hunters' catch you. \n Watch out for trees and ponds! \n \n Press any key to begin playing.", font, 40);
    text.setColor(sf::Color::Black);
    text.setPosition(100, 100);
    window.draw(text);
    window.display();

}

int main()
{
    Clock clock;
    srand(time(NULL));

    Music ding, crash, splash, intro;
    if (!ding.openFromFile("bell.wav"))
        return EXIT_FAILURE;
    if (!crash.openFromFile("crash.wav"))
        return EXIT_FAILURE;
    if (!splash.openFromFile("water.wav"))
        return EXIT_FAILURE;
    if (!intro.openFromFile("gameintro.wav"))
        return EXIT_FAILURE;
    vector<CircleShape> pondGroup;      //draw environmental obstacles (trees trunks and ponds)
    vector<RectangleShape> treeGroup;
    vector<Vector2f> treeCoords, pondCoords;
    buildEnvir(pondGroup, treeGroup, treeCoords, pondCoords);

    vector<CircleShape> treeTop;    //draw treetops
    drawTree(treeTop, treeGroup);

    vector<CircleShape> starGroup;  //draw "stars"
    vector<Vector2f> starCoords;
    drawStars(starGroup, pondCoords, treeCoords, starCoords);

    int user_x_speed[2],user_y_speed[2],counter=0,tog_counter[Hunt_Num],i;  //give user some default speed values
    Hunter hunt_array[Hunt_Num]; make_hunters(hunt_array);  //initialize an array of hunters
    double hunt_move[Hunt_Num][2];
    vector<string>run_toggle; run_toggle.resize(Hunt_Num);
    set_user_blip(user_blip,user_x_speed,user_y_speed);
    sf::View view;

    sf::RectangleShape field(sf::Vector2f(screenWid,screenHei));
    field.setFillColor(Color(10,150,10));

    user_health=100;
    view.setSize(sf::Vector2f(1200, 750));

    char lastKeyPress = 'Z';    //used in "user_moves" function to track collision with environment
    char twoKeysAgo = 'Z';

    sf::Font font;
    if (!font.loadFromFile("sansation.ttf"))
    {
        return EXIT_FAILURE;
    }

    int pondColl, treeColl, starColl;
    int starCount = 0;
    bool isCollision = false;

    bool pause = true;
    intro.setLoop(true);
    intro.play();

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event)) {
            intro.setLoop(true);
            if (event.type == Event::Closed)
                window.close();
            else  if (event.type == Event::KeyPressed) {
                    pause = false;
                    intro.stop();
            }
        }
        if (pause)
            intro_text(font, intro);
        else if (!pause) {
            while (user_health>0 && starCount<10) {
                window.clear();
                window.draw(field);
                drawEnvir(pondGroup, treeGroup, treeTop, starGroup);
                user_moves(user_blip,user_x_speed,user_y_speed, pondColl, treeColl, lastKeyPress, twoKeysAgo);
                draw_statics(font);
                hunters_move(user_blip,hunt_array, hunt_move, counter,run_toggle,tog_counter,user_health);
                draw_dynamics(hunt_array,user_blip,user_health);
                checkCollision(treeCoords, pondCoords, starCoords, starGroup, user_blip, isCollision, pondColl, treeColl,
                    starColl, ding, crash, splash, starCount,font);
                view.setCenter(sf::Vector2f(user_blip.getPosition()));
                window.setView(view);
                window.display();
            }
            if (starCount >= 10) {
                window.clear(sf::Color::Red);
                window.setSize(sf::Vector2u(800,800));
                view.setCenter(400,400);
                sf::Text text; text.setFont(font); text.setString("YOU WIN!");text.setCharacterSize(100);text.setPosition(120,325);
                window.draw(text);
                window.setView(view);
                window.display();
            }
            else if (starCount < 10) {
                window.clear();
                window.setSize(sf::Vector2u(800,800));
                view.setCenter(400,400);
                sf::Text text; text.setFont(font); text.setString("GAME OVER.");text.setCharacterSize(100);text.setPosition(80,325);

                window.draw(text);
                window.setView(view);
                window.display();
            }
        }
    }
};
