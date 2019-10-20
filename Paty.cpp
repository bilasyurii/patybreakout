#include "Paty.h"
using namespace sf;

map<string, Texture*> Storage::texturesGame;
map<string, Sprite*> Storage::spritesGame;
map<string, Font*> Storage::fontsGame;
map<string, Texture*> Storage::texturesGUI;
map<string, Sprite*> Storage::spritesGUI;
map<string, Font*> Storage::fontsGUI;
map<int, string> Storage::strings;
vector<string> Storage::localizations;
const int Storage::VAR_COUNT = 8;
int Storage::iVariables[VAR_COUNT];
string Storage::stringsPath;

void Storage::loadConfiguration(const char* path) {
    static ifstream fin;
    static string str;
    fin.open(path);
    for(unsigned int i = 0; i < VAR_COUNT; ++i)
        fin >> iVariables[i];
    localizations.clear();
    for(int i = 0; i < iVariables[0]; ++i) {
        fin >> str;
        localizations.push_back(str);
    }
    fin.close();
    if(iVariables[2] >= VideoMode::getFullscreenModes().size())
        iVariables[2] = 0;
    iVariables[3] = VideoMode::getFullscreenModes()[iVariables[2]].width;
    iVariables[4] = VideoMode::getFullscreenModes()[iVariables[2]].height;
    stringsPath = localizations[iVariables[1]];
}

void Storage::loadResources(const char *path, map<string, Texture*> &textures,
                            map<string, Sprite*> &sprites, map<string, Font*> &fonts) {
    /*
    t key path
    s key texturekey left top width height 1/0 [red green blue alpha] 1/0 [left top]
    e elementtype spritekey x y
    button: eventtype
    */
    static map<string, Texture*> texturesTemp;
    static map<string, Sprite*> spritesTemp;
    static map<string, Font*> fontsTemp;
    static char type;
    static ifstream fin;
    static string key;
    static string strTexture;
    static string fontPath;
    static Texture *tTexture;
    static Sprite *sprite;
    static Font *font;
    static IntRect rect;
    static bool flag;
    static int iColor;
    static Color color;
    texturesTemp.clear();
    spritesTemp.clear();
    fin.open(path);
    while(fin >> type) {
        fin >> key;
        if(type == 't') {
            if(textures.find(key) == textures.end()) {
                tTexture = new Texture();
                fin >> strTexture;
                tTexture->loadFromFile(strTexture);
                texturesTemp[key] = tTexture;
            } else {
                texturesTemp[key] = textures[key];
                textures.erase(key);
                getline(fin, key);
            }
        } else if(type == 's') {
            if(sprites.find(key) == sprites.end()) {
                sprite = new Sprite();
                fin >> strTexture;
                sprite->setTexture(*texturesTemp[strTexture]);
                fin >> rect.left;
                fin >> rect.top;
                fin >> rect.width;
                fin >> rect.height;
                sprite->setTextureRect(rect);
                fin >> flag;
                if(flag) {
                    fin >> iColor;
                    color.r = iColor;
                    fin >> iColor;
                    color.g = iColor;
                    fin >> iColor;
                    color.b = iColor;
                    fin >> iColor;
                    color.a = iColor;
                } else
                    color = Color::White;
                sprite->setColor(color);
                fin >> flag;
                if(flag) {
                    fin >> rect.width;
                    fin >> rect.height;
                } else {
                    rect.width /= 2;
                    rect.height /= 2;
                }
                sprite->setOrigin(rect.width, rect.height);
                spritesTemp[key] = sprite;
            } else {
                spritesTemp[key] = sprites[key];
                sprites.erase(key);
                getline(fin, key);
            }
        } else if(type == 'f') {
            if(fonts.find(key) == fonts.end()) {
                font = new Font();
                fin >> fontPath;
                font->loadFromFile(fontPath);
                fontsTemp[key] = font;
            } else {
                fontsTemp[key] = fonts[key];
                fonts.erase(key);
                getline(fin, key);
            }
        }
    }
    fin.close();
    if(&textures == &texturesGame)
        cleanGame();
    else
        cleanGUI();
    sprites = spritesTemp;
    textures = texturesTemp;
    fonts = fontsTemp;
}

void Storage::loadGUI(const char *path) {
    /*
    e elementtype id parent
    element: spritekey x y u/p u/p
    button: spritekey x y action u/p u/p
    panel: x y width height u/p u/p u/p u/p
    label: fontkey charsize stringid 1/0 [red green blue alpha] x y c/t/b/l/r u/p u/p
    list: scroll elViewed x y width height u/p u/p u/p u/p
    textbox: scroll elViewed justify padding-h x y width height u/p u/p u/p u/p
    */
    static char type;
    static ifstream fin;
    static ifstream strfin;
    static int elemType;
    static int id;
    static int parent;
    static PElement *element;
    static PButton *button;
    static PPanel *panel;
    static PLabel *label;
    static PList *plist;
    static PTextBox *textbox;
    static string key;
    static char unit;
    static bool units[4];
    static int eventType;
    static int destination;
    static float info;
    static int characterSize;
    static int stringId;
    static bool flag;
    static int iColor;
    static Color color;
    static FloatRect tempRect;
    static char align;
    static int strCount;
    static int fileLine;
    static string str;
    static char source;
    MasterRenderer::clean();
    fin.open(path);
    strings.clear();
    setlocale(LC_ALL, "Ukrainian");
    while(fin >> type) {
        if(type == 's') {
            fin >> strCount;
            fileLine = 0;
            strfin.open(stringsPath);
            for(int i = 0; i < strCount; ++i) {
                fin >> id;
                for(int j = fileLine; j <= id; ++j)
                    getline(strfin, str);
                if(str == "\\e")
                    str = "";
                strings[id] = str;
                fileLine = id + 1;
            }
            strfin.close();
        }
        if(type == 'e') {
            fin >> elemType;
            fin >> id;
            fin >> parent;
            switch(elemType) {
                case 0: { // PElement
                    element = new PElement();
                    fin >> key;
                    element->sprite = *spritesGUI[key];
                    fin >> element->pos.x;
                    fin >> element->pos.y;
                    element->size.x = element->sprite.getLocalBounds().width;
                    element->size.y = element->sprite.getLocalBounds().height;
                    for(int i = 0; i < 2; ++i) {
                        fin >> unit;
                        units[i] = (unit == 'u')?false:true;
                        units[i + 2] = false;
                    }
                } break;
                case 1: { // PButton
                    element = button = new PButton();
                    fin >> key;
                    button->sprite = *spritesGUI[key];
                    fin >> button->pos.x;
                    fin >> button->pos.y;
                    button->size.x = button->sprite.getLocalBounds().width;
                    button->size.y = button->sprite.getLocalBounds().height;
                    fin >> eventType;
                    button->event = (PEvent::Type) eventType;
                    fin >> destination;
                    button->destination = destination;
                    fin >> info;
                    button->info = info;
                    for(int i = 0; i < 2; ++i) {
                        fin >> unit;
                        units[i] = (unit == 'u')?false:true;
                        units[i + 2] = false;
                    }
                } break;
                case 2: { // PPanel
                    element = panel = new PPanel();
                    fin >> panel->pos.x;
                    fin >> panel->pos.y;
                    fin >> panel->size.x;
                    fin >> panel->size.y;
                    for(int i = 0; i < 4; ++i) {
                        fin >> unit;
                        units[i] = (unit == 'u')?false:true;
                    }
                } break;
                case 3: { // PLabel
                    element = label = new PLabel();
                    textbox = dynamic_cast<PTextBox*>(MasterRenderer::elements[parent]);
                    if(textbox)
                        textbox->labelId = id;
                    fin >> key;
                    label->text.setFont(*fontsGUI[key]);
                    fin >> characterSize;
                    characterSize *= characterSize / fontsGUI[key]->getLineSpacing(characterSize);
                    label->text.setCharacterSize(characterSize);
                    fin >> source;
                    fin >> stringId;
                    if(source == 's')
                        label->text.setString(strings[stringId]);
                    else
                        label->text.setString(to_string(iVariables[stringId]));
                    fin >> flag;
                    if(flag) {
                        fin >> iColor;
                        color.r = iColor;
                        fin >> iColor;
                        color.g = iColor;
                        fin >> iColor;
                        color.b = iColor;
                        fin >> iColor;
                        color.a = iColor;
                    } else
                        color = Color::Black;
                    label->text.setFillColor(color);
                    fin >> label->pos.x;
                    fin >> label->pos.y;
                    tempRect = label->text.getLocalBounds();
                    label->size.x = tempRect.width;
                    label->size.y = tempRect.height;
                    fin >> align;
                    switch(align) {
                        case 'c': { // center
                            tempRect.width = tempRect.left + tempRect.width / 2.0f;
                            tempRect.height = tempRect.top + tempRect.height / 2.0f;
                        } break;
                        case 't': { // top
                            tempRect.width = tempRect.left + tempRect.width / 2.0f;
                            tempRect.height = tempRect.top;
                        } break;
                        case 'b': { // bottom
                            tempRect.width = tempRect.left + tempRect.width / 2.0f;
                            tempRect.height = tempRect.top + tempRect.height;
                        } break;
                        case 'l': { // left
                            tempRect.width = tempRect.left;
                            tempRect.height = tempRect.top + tempRect.height / 2.0f;
                        } break;
                        case 'r': { // right
                            tempRect.width = tempRect.left + tempRect.width;
                            tempRect.height = tempRect.top + tempRect.height / 2.0f;
                        } break;
                    }
                    label->text.setOrigin(tempRect.width, tempRect.height);
                    for(int i = 0; i < 2; ++i) {
                        fin >> unit;
                        units[i] = (unit == 'u')?false:true;
                        units[i + 2] = false;
                    }
                } break;
                case 4: { // PList
                    element = plist = new PList();
                    fin >> key;
                    plist->sprite = *spritesGUI[key];
                    fin >> plist->elementsViewed;
                    fin >> plist->pos.x;
                    fin >> plist->pos.y;
                    fin >> plist->size.x;
                    fin >> plist->size.y;
                    for(int i = 0; i < 4; ++i) {
                        fin >> unit;
                        units[i] = (unit == 'u')?false:true;
                    }
                } break;
                case 5: { // PTextBox
                    element = textbox = new PTextBox();
                    fin >> key;
                    textbox->sprite = *spritesGUI[key];
                    fin >> textbox->elementsViewed;
                    fin >> textbox->isJustify;
                    fin >> textbox->paddingHorizontal;
                    fin >> textbox->pos.x;
                    fin >> textbox->pos.y;
                    fin >> textbox->size.x;
                    fin >> textbox->size.y;
                    for(int i = 0; i < 4; ++i) {
                        fin >> unit;
                        units[i] = (unit == 'u')?false:true;
                    }
                } break;
            }
            if(parent == 0) {
                element->parent = NULL;
                MasterRenderer::hierarchy[id] = element;
            } else {
                element->parent = MasterRenderer::elements[parent];
                element->parent->children.push_back(element);
            }
            element->update(units);
            MasterRenderer::elements[id] = element;
        }
    }
    fin.close();
    for(map<int, PElement*>::iterator i = MasterRenderer::elements.begin();
            i != MasterRenderer::elements.end(); ++i)
        i->second->update();
}

void Storage::cleanGame() {
    for(pair<string, Sprite*> s : spritesGame)
        delete s.second;
    for(pair<string, Texture*> t : texturesGame)
        delete t.second;
    for(pair<string, Font*> f : fontsGame)
        delete f.second;
    spritesGame.clear();
    texturesGame.clear();
    fontsGame.clear();
}

void Storage::cleanGUI() {
    for(pair<string, Sprite*> s : spritesGUI)
        delete s.second;
    for(pair<string, Texture*> t : texturesGUI)
        delete t.second;
    for(pair<string, Font*> f : fontsGUI)
        delete f.second;
    spritesGUI.clear();
    texturesGUI.clear();
    fontsGUI.clear();
}

void Storage::clean() {
    cleanGame();
    cleanGUI();
}

Clock Utility::timer;
float Utility::delta;

float Utility::minInArray(float *arr, size_t n) {
    static float x;
    x = arr[0];
    for(size_t i = 1; i < n; ++i)
        if(arr[i] < x)
            x = arr[i];
    return x;
}

float Utility::maxInArray(float *arr, size_t n) {
    static float x;
    x = arr[0];
    for(size_t i = 1; i < n; ++i)
        if(arr[i] > x)
            x = arr[i];
    return x;
}

inline float Utility::toRadians(float degree) {
    static float temp;
    temp = degree * PI / 180.0f;
    if(temp >= 0) return temp;
    return temp + 360.0f;
}

inline float Utility::toDegrees(float radian) {
    static float temp;
    temp = radian / PI * 180.0f;
    if(temp >= 0) return temp;
    return temp + 360.0f;
}

bool Utility::circle2circle(CircleShape &a, CircleShape &b) {
    return pow(b.getPosition().x - a.getPosition().x, 2.0) +
            pow(b.getPosition().y - a.getPosition().y, 2.0) <= pow(a.getRadius() + b.getRadius(), 2.0);
}

float inline Utility::distance(Vector2f a, Vector2f b) {
    return sqrt(SQR(b.x - a.x) + SQR(b.y - a.y));
}

float inline Utility::distance(Vector2f p, Vector2f a, Vector2f b) {
    return distance(projectPointOnSegment(p, a, b), p);
}

Vector2f Utility::rotatePoint(Vector2f p, Vector2f c, float angle) {
    return Vector2f(c.x + (p.x - c.x)  * cos(toRadians(angle)) - (p.y - c.y) * sin(toRadians(angle)),
                    c.y + (p.x - c.x)  * sin(toRadians(angle)) + (p.y - c.y) * cos(toRadians(angle)));
}

Vector2f Utility::projectPointOnSegment(Vector2f p, Vector2f a, Vector2f b) {
    float t = ((b.x - a.x) * (p.x - a.x) + (b.y - a.y) * (p.y - a.y)) / (SQR(b.x - a.x) + SQR(b.y - a.y));
    return Vector2f(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
}

float Utility::whereOnSegment(Vector2f p, Vector2f a, Vector2f b) {
    static Vector2f diff;
    diff = b - a;
    if(ABS(diff.x) >= ABS(diff.y))
        return (p.x - a.x) / diff.x;
    return (p.y - a.y) / diff.y;
}

void Utility::getPoints(Shape *s, Vector2f *a) {
    for(int i = 0; i < s->getPointCount(); ++i)
        a[i] = s->getTransform().transformPoint(s->getPoint(i));
}

// only for shapes with origin in the center
bool Utility::rect2circle(RectangleShape &r, CircleShape &c) {
    if(!r.getGlobalBounds().intersects(c.getGlobalBounds()))
        return false;
    static Vector2f p[4];
    static float w1;
    static float w2;
    getPoints(&r, p);
    w1 = whereOnSegment(projectPointOnSegment(c.getPosition(), p[0], p[1]), p[0], p[1]);
    w2 = whereOnSegment(projectPointOnSegment(c.getPosition(), p[1], p[2]), p[1], p[2]);
    if(w1 > 0 && w1 < 1 && w2 > 0 && w2 < 1)
        return true;
    /*
      012 0x1
      3 5 xxx
      678 3x2
    */
    static int posNumber;
    if(w2 <= 0) posNumber = 0;
    else if(w2 >= 1) posNumber = 6;
    else posNumber = 3;
    if(w1 >= 1) posNumber += 2;
    else if(w1 > 0) posNumber += 1;
    switch(posNumber) {
        case 0: if(distance(c.getPosition(), p[0]) < c.getRadius()) return true; break;
        case 2: if(distance(c.getPosition(), p[1]) < c.getRadius()) return true; break;
        case 8: if(distance(c.getPosition(), p[2]) < c.getRadius()) return true; break;
        case 6: if(distance(c.getPosition(), p[3]) < c.getRadius()) return true; break;
        case 1: if(distance(c.getPosition(), p[0], p[1]) < c.getRadius()) return true; break;
        case 5: if(distance(c.getPosition(), p[1], p[2]) < c.getRadius()) return true; break;
        case 7: if(distance(c.getPosition(), p[2], p[3]) < c.getRadius()) return true; break;
        case 3: if(distance(c.getPosition(), p[3], p[0]) < c.getRadius()) return true; break;
    }
    return false;
}

bool Utility::rect2rect(RectangleShape &a, RectangleShape &b) {
    if(!a.getGlobalBounds().intersects(b.getGlobalBounds()))
        return false;
    static Vector2f ax[4];
    static Vector2f po[4];
    static float w[6];
    static Vector2f *axises;
    static Vector2f *points;
    getPoints(&a, ax);
    getPoints(&b, po);
    axises = ax;
    points = po;
    for(int j = 0; j < 2; ++j) {
        if(j) {
            axises = po;
            points = ax;
        }
        for(int k = 0; k < 2; ++k) {
            for(int i = 0; i < 4; ++i)
                w[i] = whereOnSegment(projectPointOnSegment(points[i], axises[k], axises[k + 1]),
                                      axises[k], axises[k + 1]);
            w[4] = minInArray(w, 4);
            w[5] = maxInArray(w, 4);
            if(w[4] > 1 || w[5] < 0)
                return false;
        }
    }
    return true;
}

bool Utility::isCollision(Shape *a, Shape *b) {
    if(dynamic_cast<RectangleShape*>(a) && dynamic_cast<RectangleShape*>(b))
        return rect2rect(*((RectangleShape*) a), *((RectangleShape*) b));
    if(dynamic_cast<CircleShape*>(a) && dynamic_cast<CircleShape*>(b))
        return circle2circle(*((CircleShape*) a), *((CircleShape*) b));
    if(dynamic_cast<CircleShape*>(a))
        return rect2circle(*((RectangleShape*) b), *((CircleShape*) a));
    return rect2circle(*((RectangleShape*) a), *((CircleShape*) b));
}

float Utility::radius(Shape *s) {
    static RectangleShape *rect;
    if((rect = dynamic_cast<RectangleShape*>(s)))
        return sqrt(SQR(rect->getSize().x / 2) + SQR(rect->getSize().y / 2));
    return dynamic_cast<CircleShape*>(s)->getRadius();
}

Shape* Utility::makeRect(Vector2f pos, Vector2f sz, float angle = 0) {
    static RectangleShape *rect;
    rect = new RectangleShape(sz);
    rect->setPosition(pos);
    rect->setRotation(angle);
    rect->setOrigin(sz.x / 2, sz.y / 2);
    #if DEBUG
        rect->setFillColor(Color(255, 0, 0, 200));
    #endif
    return rect;
}

Shape* Utility::makeRect(float x, float y, Vector2f sz, float angle = 0) {
    static RectangleShape *rect;
    rect = new RectangleShape(sz);
    rect->setPosition(x, y);
    rect->setRotation(angle);
    rect->setOrigin(sz.x / 2, sz.y / 2);
    #if DEBUG
        rect->setFillColor(Color(255, 0, 0, 200));
    #endif
    return rect;
}

Shape* Utility::makeCircle(Vector2f pos, float radius) {
    static CircleShape *circle;
    circle = new CircleShape(radius);
    circle->setPosition(pos);
    circle->setOrigin(radius, radius);
    #if DEBUG
        circle->setFillColor(Color(255, 255, 0, 200));
    #endif
    return circle;
}

Shape* Utility::makeCircle(float x, float y, float radius) {
    static CircleShape *circle;
    circle = new CircleShape(radius);
    circle->setPosition(x, y);
    circle->setOrigin(radius, radius);
    #if DEBUG
        circle->setFillColor(Color(255, 255, 0, 200));
    #endif
    return circle;
}

float Utility::angle(Vector2f from, Vector2f to) {
    static float temp;
    temp = toDegrees(atan2(to.y - from.y, to.x - from.x));
    if(temp >= 0) return temp;
    return temp + 360.0f;
}

void Utility::updateTiming() {
    delta = timer.restart().asMicroseconds() / 1000.0f;
}

Vector2f Utility::pointOnCircle(Vector2f pos, float radius, float angle) {
    angle = Utility::toRadians(angle);
    pos.x += radius * cos(angle);
    pos.y += radius * sin(angle);
    return pos;
}

Vector2f Utility::randomizePosition(Vector2f pos, float radius) {
    static float dst;
    static float angle;
    dst = rand() % (int) radius;
    angle = rand() % (int) (200.0f * PI);
    angle /= 100.0f;
    pos.x += dst * cos(angle);
    pos.y += dst * sin(angle);
    return pos;
}

float Utility::parabolicChange(float x, float maxX, float maxY) {
    return maxY / (maxX * maxX) * x * x;
}

float Utility::gradeBetween(float a, float b, float grade) {
    if(a == b) return a;
    if(a < b) return a + (b - a) * grade;
    else return b + (a - b) * grade;
}

Color Utility::gradeBetween(Color a, Color b, float grade) {
    return Color(gradeBetween(a.r, b.r, grade),
                 gradeBetween(a.g, b.g, grade),
                 gradeBetween(a.b, b.b, grade),
                 gradeBetween(a.a, b.a, grade));
}

int Utility::clamp(int n, int a, int b) {
    if(n > b)
        return b;
    if(n < a)
        return a;
    return n;
}

inline float Utility::length(Vector2f v) {
    return sqrt(SQR(v.x) + SQR(v.y));
}

inline Vector2f Utility::normalize(Vector2f v) {
    return v / length(v);
}

Entity::Entity(Sprite _sprite, Shape *_body, float _speed, int _id, float _health) :
        body(_body), speed(_speed), health(_health),
        maxHealth(_health), id(_id), isAlive(true), sprite(_sprite) {}

Entity::Entity(const Entity &e) {
    if(dynamic_cast<RectangleShape*>(e.body))
        body = new RectangleShape(*(RectangleShape*)(e.body));
    else
        body = new CircleShape(*(CircleShape*)(e.body));
    speed = e.speed;
    health = e.health;
    maxHealth = e.maxHealth;
    id = e.id;
    isAlive = e.isAlive;
    sprite = e.sprite;
}

Entity::Entity(Entity &&e) {
    body = e.body;
    e.body = NULL;
    speed = e.speed;
    health = e.health;
    maxHealth = e.maxHealth;
    id = e.id;
    isAlive = e.isAlive;
    sprite = e.sprite;
}

Entity& Entity::operator=(const Entity &e) {
    if(body)
        delete body;
    if(dynamic_cast<RectangleShape*>(e.body))
        body = new RectangleShape(*(RectangleShape*)(e.body));
    else
        body = new CircleShape(*(CircleShape*)(e.body));
    speed = e.speed;
    health = e.health;
    maxHealth = e.maxHealth;
    id = e.id;
    isAlive = e.isAlive;
    sprite = e.sprite;
    return *this;
}

Entity * Entity::clone() {
    return new Entity(*this);
}

void Entity::lookAt(float x, float y) {
    body->setRotation(Utility::toDegrees(atan2(y - body->getPosition().y, x - body->getPosition().x)));
}

void Entity::Move() {
    body->move(Vector2f(cos(Utility::toRadians(body->getRotation())) * speed * Utility::delta,
                        sin(Utility::toRadians(body->getRotation())) * speed * Utility::delta));
}

void Entity::update() {}

void Entity::takeDamage(float _damage, Entity *source) {
    static int burstCount;
    if(_damage > 0) {
        health -= _damage;
        if(health <= 0) {
            health = 0;
            isAlive = false;
            burstCount = rand() % 2 + 2;
            for(int i = 0; i < burstCount; ++i)
                ParticleEmitter::burst(Utility::randomizePosition(body->getPosition(), Utility::radius(body)),
                                       15 + rand() % 11, 3 + rand() % 3, 0.1f + (rand() % 3) * 0.05f,
                                       500.0f + (rand() % 5) * 100.0f,
                                       Utility::gradeBetween(Color(255, 100, 0),
                                                             Color(255, 255, 0),
                                                             rand() % 101 / 100.0f), true);
        } else
            ParticleEmitter::burst(Utility::randomizePosition(body->getPosition(), Utility::radius(body)),
                                   10 + rand() % 7, 2 + rand() % 2, 0.1f + (rand() % 2) * 0.05f, 500.0f,
                                   Utility::gradeBetween(Color(255, 100, 0),
                                                         Color(255, 255, 0),
                                                         rand() % 101 / 100.0f), true);
    }
}

Entity::~Entity() {
    if(body)
        delete body;
}

Bullet::Bullet(Sprite _sprite, Shape *_body, float _speed, int _id, float _health, float _damage) :
        Entity(_sprite, _body, _speed, _id, _health), damage(_damage) {}


Shooter::Shooter(Sprite _sprite, Shape *_body, float _speed,
        int _id, float _health, float _damage, float _reload) :
        Entity(_sprite, _body, _speed, _id, _health),
        currTime(0), damage(_damage), reload(_reload) {}

void Shooter::update() {
    currTime += Utility::delta;
}

void Shooter::shoot(float speed) {
    if(currTime > reload) {
        currTime = 0;
        static Bullet *bullet;
        bullet = new Bullet(*Storage::spritesGame["bulletGreen"],
                            Utility::makeRect(body->getPosition(), Vector2f(16, 4), body->getRotation()),
                            speed, id, 100.0f, damage);
        World::effects.push_back(new Sticky(*Storage::spritesGame["glowGreen"], bullet, 50, id));
        World::effects.back()->body->setRotation(body->getRotation());
        World::entities.push_back(bullet);
        World::entities.back()->body->setRotation(body->getRotation());
    }
}

EntitySpawner::EntitySpawner(float _delay) : currTime(0), delay(_delay) {}

bool EntitySpawner::spawn(Entity *Ref) {
    currTime += Utility::delta;
    if(currTime >= delay) {
        currTime = 0;
        dir = rand() % 4;
        if(dir % 2 == 0) {
            margin = rand() % MasterRenderer::WINDOW_W;
            spawnPos.x = margin;
            if(dir == 0) spawnPos.y = 0;
            else spawnPos.y = MasterRenderer::WINDOW_H;
        } else {
            margin = rand() % MasterRenderer::WINDOW_H;
            spawnPos.y = margin;
            if(dir == 1) spawnPos.x = MasterRenderer::WINDOW_W;
            else spawnPos.x = 0;
        }
        Entity *e = Ref->clone();
        e->body->setPosition(spawnPos);
        World::entities.push_back(e);
        return true;
    }
    return false;
}

void EntitySpawner::reset() {
    currTime = 0;
}

const float Shield::opacityPerTick = 0.001f;

Shield::Shield(Entity *_parent) : parent(_parent), opacity(0), restoring(0) {}

void Shield::initShield(Sprite _sprite, float _radius, float _energy, float _energyPerTick, float _restoreDelay) {
    spriteShield = _sprite;
    radius = _radius;
    maxEnergy = energy = _energy;
    energyPerTick = _energyPerTick;
    restoreDelay = _restoreDelay;
}

void Shield::update() {
    if(opacity > 0) {
        opacity -= opacityPerTick * Utility::delta;
        if(opacity < 0)
            opacity = 0;
    }
    if(restoring > 0) {
        restoring -= Utility::delta;
        if(restoring < 0)
            restoring = 0;
    }
    if(restoring == 0) {
        energy += energyPerTick;
        if(energy > maxEnergy)
            energy = maxEnergy;
    }
}

float Shield::takeDamage(float _damage, Entity *source) {
    static float damageForShip;
    if(energy > 0) {
        energy -= _damage;
        opacity = 1;
        restoring = restoreDelay;
        if(energy > 0)
            damageForShip = 0;
        else {
            damageForShip = -energy;
            energy = 0;
        }
        ParticleEmitter::burst(Utility::pointOnCircle(parent->body->getPosition(), radius,
                                                      Utility::angle(parent->body->getPosition(),
                                                                     source->body->getPosition())),
                               10 + rand() % 7, 2 + rand() % 2, 0.1f + (rand() % 2) * 0.05f, 500.0f,
                               Utility::gradeBetween(Color(0, 0, 255),
                                                     Color(0, 255, 255),
                                                     rand() % 101 / 100.0f), true);
        return damageForShip;
    }
    return _damage;
}


Player::Player(Sprite _sprite, Shape *_body, float _speed, int _id, float _health, float _damage, float _reload) :
        Shooter(_sprite, _body, _speed, _id, _health, _damage, _reload), Shield(this),
        emitter(0.1f, 500.0f, 15.0f, 10.0f, Color::Blue, true) {}

Player * Player::clone() {
    return new Player(*this);
}

void Player::Move() {
    wasPressed = false;
    if(Keyboard::isKeyPressed(Keyboard::A)) { body->move(-speed * Utility::delta, 0); wasPressed = true; }
    if(Keyboard::isKeyPressed(Keyboard::D)) { body->move(speed * Utility::delta, 0); wasPressed = true; }
    if(Keyboard::isKeyPressed(Keyboard::W)) { body->move(0, -speed * Utility::delta); wasPressed = true; }
    if(Keyboard::isKeyPressed(Keyboard::S)) { body->move(0, speed * Utility::delta); wasPressed = true; }
    if(wasPressed) {
        // anti-gothroughwall protection
        if(body->getPosition().x > MasterRenderer::WINDOW_W - 25.0f)
            body->setPosition(MasterRenderer::WINDOW_W - 25.0f, body->getPosition().y);
        else if(body->getPosition().x < 25.0f)
            body->setPosition(25.0f, body->getPosition().y);
        if(body->getPosition().y > MasterRenderer::WINDOW_H - 25.0f)
            body->setPosition(body->getPosition().x, MasterRenderer::WINDOW_H - 25.0f);
        else if(body->getPosition().y < 25.0f)
            body->setPosition(body->getPosition().x, 25.0f);
        lookAt(Mouse::getPosition(MasterRenderer::window).x, Mouse::getPosition(MasterRenderer::window).y);
        emitter.emit(Utility::pointOnCircle(body->getPosition(),
                                            Utility::radius(body),
                                            body->getRotation() + 180),
                     body->getRotation() + 180);
    }
}

void Player::update() {
    Shooter::update();
    Shield::update();
}

void Player::takeDamage(float _damage, Entity *source) {
    Entity::takeDamage(Shield::takeDamage(_damage, source), source);
}

Chaser::Chaser(float _angularSpeed, Entity *_target, Entity *_parent) :
    angularSpeed(_angularSpeed), target(_target), parent(_parent) {}

void Chaser::update() {
    static float diff;
    diff = Utility::angle(parent->body->getPosition(), target->body->getPosition()) -
            parent->body->getRotation();
    if(ABS(diff) < 0.0001f)
        return;
    if(diff <= -180.0f)
        diff += 360.0f;
    else if(diff >= 180.0f)
        diff -= 360.0f;
    if(ABS(diff) <= angularSpeed * Utility::delta) {
        parent->body->rotate(diff);
        return;
    }
    if(ABS(diff) > 60.0f)
        return;
    parent->body->rotate(SIGN(diff) * angularSpeed * Utility::delta);
}

Obstacle::Obstacle(Sprite _sprite, Shape *_body, float _speed, int _id, float _health) :
        Entity(_sprite, _body, _speed, _id, _health) {}

Obstacle * Obstacle::clone() {
    return new Obstacle(*this);
}

Enemy::Enemy(Sprite _sprite, Shape *_body, float _speed, int _id,
             float _health, float _damage, float _reload) :
        Shooter(_sprite, _body, _speed, _id, _health, _damage, _reload),
        Chaser(0.1f, World::player, this),
        emitter(0.1f, 500.0f, 10.0f, 30.0f, Color::Red, true) {}

Enemy::Enemy(const Enemy &e) : Shooter(e), Chaser(e), emitter(e.emitter) {
    parent = this;
}

Enemy * Enemy::clone() {
    return new Enemy(*this);
}

void Enemy::Move() {
    Entity::Move();
    emitter.emit(Utility::pointOnCircle(body->getPosition(),
                                            Utility::radius(body),
                                            body->getRotation() + 180),
                     body->getRotation() + 180);
}

void Enemy::update() {
    Chaser::update();
    Shooter::update();
    if(ABS(Utility::angle(body->getPosition(), target->body->getPosition()) - body->getRotation()) < 10.0f)
        shoot(0.5f);
}

Rocket::Rocket(Sprite _sprite, Shape *_body, float _speed, int _id, float _health, float _damage, float _angularSpeed) :
    Bullet(_sprite, _body, _speed, _id, _health, _damage),
    Chaser(0.05f, World::player, this),
    emitter(0.1f, 500.0f, 10.0f, 30.0f, Color::Red, true) {}

void Rocket::Move() {
    Entity::Move();
    emitter.emit(Utility::pointOnCircle(body->getPosition(),
                                            Utility::radius(body),
                                            body->getRotation() + 180),
                     body->getRotation() + 180);
}

void Rocket::update() {
    Chaser::update();
}

Particle::Particle(Sprite _sprite, Shape *_body, float _speed, float _health, Color _color, bool _fade = false) :
        Entity(_sprite, _body, _speed, 0, _health), maxColor(_color), fade(_fade), color(_color) {}

void Particle::update() {
    if(isAlive) {
        health -= Utility::delta;
        if(fade)
            color.a = 255 * health / maxHealth;
        if(health <= 0)
            isAlive = false;
    }
}

ParticleEmitter::ParticleEmitter(float _speed, float _health, float _radius, float _delay, Color _color, bool _fade = false) :
        currTime(0), speed(_speed), health(_health), radius(_radius), delay(_delay), color(_color), fade(_fade) {}

void ParticleEmitter::emit(Vector2f pos, float angle) {
    currTime += Utility::delta;
    if(currTime >= delay) {
        currTime = 0;
        World::particles.push_back(Particle(MasterRenderer::sEmpty,
                                            Utility::makeRect(Utility::randomizePosition(pos, radius),
                                                              Vector2f(0, 0)),
                                            speed, health, color, fade));
        World::particles.back().body->setRotation(angle);
    }
}

void ParticleEmitter::burst(Vector2f _pos, int _particleCount, int _levels,
                            float _speed, float _health, Color _color, bool _fade) {
    static int offset;
    for(int i = 0; i < _particleCount; ++i)
        for(int j = 1; j <= _levels; ++j) {
            offset = rand() % 31 - 15;
            World::particles.push_back(Particle(MasterRenderer::sEmpty,
                                                Utility::makeRect(_pos, Vector2f(0, 0)),
                                                Utility::parabolicChange(j, _levels, _speed),
                                                Utility::parabolicChange(j, _levels, _health),
                                                Utility::gradeBetween(_color, Color::White,
                                                                      rand() % 20 / 100.0f), _fade));
            World::particles.back().body->setRotation(360.0f / _particleCount * i + offset);
        }
}

Sticky::Sticky(Sprite _sprite, Entity* _target, float _radius, int _id) :
    Entity(_sprite, Utility::makeCircle(_target->body->getPosition(), _radius), 0, _id, 0),
    target(_target) {}

void Sticky::update() {
    if(!target->isAlive)
        isAlive = false;
    body->setPosition(target->body->getPosition());
}

void Sticky::Move() {}

Enemy* World::sampleEnemy = NULL;
Obstacle* World::sampleObstacle = NULL;
vector<Entity*> World::entities;
vector<Particle> World::particles;
vector<Entity*> World::effects;
Player * World::player;
EntitySpawner World::spEnemy(2000.0f);
EntitySpawner World::spObstacle(4000.0f);

void World::init() {
    player = new Player(*Storage::spritesGame["player"],
                           Utility::makeCircle(MasterRenderer::WINDOW_W / 2,
                                               MasterRenderer::WINDOW_H / 2, 25.0f),
                           0.2f, 0, 50.0f, 10.0f, 200.0f);
    player->initShield(*Storage::spritesGame["shieldCyan"], 35.0f, 100.0f, 0.05f, 2000.0f);
    entities.push_back(player);
    sampleEnemy = new Enemy(*Storage::spritesGame["bomber"],
                            Utility::makeCircle(0.0f, 0.0f, 30.0f), 0.15f, 1, 20.0f, 10.0f, 1000.0f);
    sampleObstacle = new Obstacle(*Storage::spritesGame["obstacle"],
                                  Utility::makeCircle(0.0f, 0.0f, 25.0f), 0.05f, 1, 40.0f);
    spEnemy.reset();
    spObstacle.reset();
}

void World::update() {
    if(World::spEnemy.spawn(sampleEnemy))
        World::entities.back()->lookAt(player->body->getPosition().x, player->body->getPosition().y);
    if(World::spObstacle.spawn(sampleObstacle))
        World::entities.back()->lookAt(player->body->getPosition().x, player->body->getPosition().y);
    for(unsigned int i = 0; i < World::particles.size(); ++i) {
        World::particles.at(i).Move();
        World::particles.at(i).update();
        if(!World::particles.at(i).isAlive) {
            World::particles.erase(World::particles.begin() + i);
            --i;
        }
    }
    for(unsigned int i = 0; i < World::entities.size(); ++i) {
        World::entities.at(i)->Move();
        World::entities.at(i)->update();
    }
    for(unsigned int i = 0; i < World::entities.size(); ++i) {
        for(unsigned int j = i + 1; j < World::entities.size(); ++j) {
            if(World::entities.at(i)->isAlive && World::entities.at(j)->isAlive &&
               World::entities.at(i)->id != World::entities.at(j)->id &&
               Utility::isCollision(World::entities.at(i)->body, World::entities.at(j)->body)) {
                Bullet *a = dynamic_cast<Bullet*>(World::entities.at(i));
                Bullet *b = dynamic_cast<Bullet*>(World::entities.at(j));
                if(!a && !b) {
                    World::entities.at(i)->takeDamage(World::entities.at(j)->health, World::entities[j]);
                    World::entities.at(j)->takeDamage(World::entities.at(i)->health, World::entities[i]);
                } else if(a && !b) {
                    a->isAlive = false;
                    World::entities.at(j)->takeDamage(a->damage, a);
                } else if(!a && b) {
                    World::entities.at(i)->takeDamage(b->damage, b);
                    b->isAlive = false;
                }
            }
        }
    }
    // TODO radius
    // TODO same in player's Move
    for(unsigned int i = 0; i < World::entities.size(); ++i) {
        if(World::entities.at(i)->body->getPosition().x > MasterRenderer::WINDOW_W + 100 ||
           World::entities.at(i)->body->getPosition().x < -100 ||
           World::entities.at(i)->body->getPosition().y > MasterRenderer::WINDOW_H  + 100 ||
           World::entities.at(i)->body->getPosition().y < -100) {
            World::entities.at(i)->isAlive = false;
        }
    }
    for(unsigned int i = 0; i < World::effects.size(); ++i) {
        World::effects.at(i)->update();
        if(!World::effects.at(i)->isAlive) {
            delete World::effects.at(i);
            World::effects.erase(World::effects.begin() + i);
            --i;
        }
    }
    for(unsigned int i = 0; i < World::entities.size(); ++i)
        if(!World::entities.at(i)->isAlive) {
            delete World::entities.at(i);
            World::entities.erase(World::entities.begin() + i);
            --i;
        }
    if(Utility::length(MasterRenderer::skyTarget - MasterRenderer::skyOffset) <= SKY_SPEED * Utility::delta)
        MasterRenderer::skyOffset = MasterRenderer::skyTarget;
    else
        MasterRenderer::skyOffset += Utility::normalize(MasterRenderer::skyTarget - MasterRenderer::skyOffset) *
                                                        (SKY_SPEED * Utility::delta);
}

void World::clean() {
    for(Entity *e : entities)
        delete e;
    for(Entity *e : effects)
        delete e;
    entities = vector<Entity*>();
    particles = vector<Particle>();
    effects = vector<Entity*>();
    if(sampleEnemy)
        delete sampleEnemy;
    if(sampleObstacle)
        delete sampleObstacle;
    sampleEnemy = NULL;
    sampleObstacle = NULL;
}

PElement::PElement() : parent(NULL), visible(true) {
    size.x = 0;
    size.y = 0;
}

void PElement::update(bool *units) {
    static Vector2f temp;
    if(parent) {
        if(units[0])
            pos.x *= parent->size.x / 100.0f;
        if(units[1])
            pos.y *= parent->size.y / 100.0f;
        pos += parent->pos - parent->size / 2.0f;
    } else {
        if(units[0])
            pos.x *= MasterRenderer::WINDOW_W / 100.0f;
        if(units[1])
            pos.y *= MasterRenderer::WINDOW_H / 100.0f;
    }
    sprite.setPosition(pos);
    if(parent) {
        if(units[2])
            size.x *= parent->size.x / 100.0f;
        if(units[3])
            size.y *= parent->size.y / 100.0f;
    } else {
        if(units[2])
            size.x *= MasterRenderer::WINDOW_W / 100.0f;
        if(units[3])
            size.y *= MasterRenderer::WINDOW_W / 100.0f;
    }
}

void PElement::draw(RenderTarget& target, RenderStates states) const {
    if(visible) {
        target.draw(sprite, states);
        for(PElement *e : children)
            target.draw(*e, states);
    }
}

PLabel::PLabel() : PElement() {}

void PLabel::update(bool *units) {
    PElement::update(units);
    text.setPosition(pos);
}

void PLabel::draw(RenderTarget& target, RenderStates states) const {
    if(visible)
        target.draw(text, states);
}

PPanel::PPanel() : PElement() {}

void PPanel::draw(RenderTarget& target, RenderStates states) const {
    if(visible)
        for(PElement *e : children)
            target.draw(*e, states);
}

PList::PList() : PElement(), scrollable(false), scrollSpacing(0), paddingHorizontal(0) {
    scroll = 0;
}

void PList::update() {
    updateVisible();
    scrollable = (children.size() > elementsViewed);
    if(scrollable)
        sprite.setPosition((size.x + SCROLL_WIDTH) / 2.0f + pos.x,
                           pos.y + sprite.getLocalBounds().height / 2.0f +
                           scrollSpacing * scroll);
}

void PList::Scroll() {
    sprite.move(0.0f, EventManager::mouseMove.y);
    if(sprite.getPosition().y > pos.y + size.y - sprite.getLocalBounds().height / 2.0f)
        sprite.setPosition(sprite.getPosition().x, pos.y + size.y - sprite.getLocalBounds().height / 2.0f);
    if(sprite.getPosition().y < pos.y + sprite.getLocalBounds().height / 2.0f)
        sprite.setPosition(sprite.getPosition().x, pos.y + sprite.getLocalBounds().height / 2.0f);
    scroll = ((int)(sprite.getPosition().y - pos.y - sprite.getLocalBounds().height / 2.0f)) /
        ((int)scrollSpacing);
    updateVisible();
}

void PList::Scroll(int delta) {
    scroll += delta;
    if(scroll < 0)
        scroll = 0;
    if(scroll > children.size() - elementsViewed)
        scroll = children.size() - elementsViewed;
    PList::update();
}

void PList::updateVisible() {
    static float elementHeight;
    for(PElement *e : children)
        e->visible = false;
    if(children.size() > 0) {
        elementHeight = children[0]->size.y;
        scrollSpacing = (size.y - sprite.getLocalBounds().height) / (children.size() - elementsViewed);
    }
    for(unsigned int i = 0; i < elementsViewed; ++i)
        if(i + scroll < children.size()) {
            children[i + scroll]->visible = true;
            children[i + scroll]->pos.y = pos.y + elementHeight / 2.0f + i * elementHeight;
            children[i + scroll]->sprite.setPosition(children[i + scroll]->pos);
            if(PLabel *label = dynamic_cast<PLabel*>(children[i + scroll]))
                label->text.setPosition(label->sprite.getPosition());
        }
}

void PList::draw(RenderTarget& target, RenderStates states) const {
    if(visible) {
        for(PElement *e : children)
            target.draw(*e, states);
        if(scrollable)
            target.draw(sprite, states);
    }
}

PTextBox::PTextBox() : PList(), isJustify(false) {}

void PTextBox::update() {
    static PLabel *sample;
    sample = static_cast<PLabel*>(children[0]);
    sample->update();
    text = sample->text;
    static string str;
    str = sample->text.getString().toAnsiString();
    vector<string> lines;
    queue<string> words;
    static unsigned int j;
    for(unsigned int i = 0; i < str.length(); ++i) {
        j = i;
        while(str[j] != ' ' && j < str.length())
            ++j;
        words.push(str.substr(i, j - i));
        i = j;
    }
    static string line;
    static int spaceLen;
    line = "";
    spaceLen = getLength(" ");
    while(!words.empty()) {
        if(words.front() == "\\n") {
            lines.push_back(line);
            words.pop();
            line = "";
        } else if(line == "") {
            line += words.front();
            words.pop();
        } else {
            if(getLength(words.front()) + getLength(line) + spaceLen  > size.x - paddingHorizontal * 2.0f) {
                lines.push_back(line);
                line = "";
            } else {
                line += " ";
                line += words.front();
                words.pop();
            }
        }
    }
    if(line != "")
        lines.push_back(line);
    static size_t pos;
    static int spaceCount;
    static bool found;
    if(isJustify) {
        for(unsigned int i = 0; i < lines.size(); ++i) {
            if(lines[i] == "" || (i + 1 < lines.size() && lines[i + 1] == "") || i == lines.size() - 1)
                continue;
            line = "";
            spaceCount = (size.x - getLength(lines[i]) - paddingHorizontal * 2.0f + 1.0f) / getLength(" ");
            while(spaceCount > 0) {
                line += " ";
                pos = 0;
                found = false;
                while(spaceCount > 0) {
                    pos = lines[i].find(line, pos);
                    if(pos >= lines[i].length())
                        break;
                    lines[i].insert(pos, " ");
                    pos += line.length() + 1;
                    --spaceCount;
                    found = true;
                }
                if(!found)
                    break;
            }
        }
    }
    static PLabel *label;
    for(string s : lines) {
        label = new PLabel(*sample);
        label->text.setString(s);
        label->text.setOrigin(size.x / 2.0f - paddingHorizontal, label->text.getLocalBounds().top +
                              label->text.getLocalBounds().height / 2.0f);
        children.push_back(label);
    }
    delete children.front();
    children.erase(children.begin());
    MasterRenderer::elements.erase(labelId);
    PList::update();
}

int PTextBox::getLength(string str) {
    text.setString(str);
    return text.getLocalBounds().width;
}

PTextBox::~PTextBox() {
    for(PElement *e : children)
        delete e;
}

PButton::PButton() : PElement(), active(true) {}

void PButton::click() {
    if(active)
        EventManager::events.push(PEvent(event, this, destination, info));
}

PEvent::PEvent() {}

PEvent::PEvent(Type _type, PElement *_source, int _destination, float _info) :
    type(_type), source(_source), destination(_destination), info(_info) {}

queue<PEvent> EventManager::events;
bool EventManager::dragging = false;
Vector2i EventManager::prevMousePos = Mouse::getPosition();
Vector2i EventManager::mouseMove(0, 0);

void EventManager::processQueue() {
    mouseMove = Mouse::getPosition(MasterRenderer::window) - prevMousePos;
    prevMousePos = Mouse::getPosition(MasterRenderer::window);
    if(StateMachine::state == GameState::PLAYING)
        MasterRenderer::targetSky();
    static PEvent e;
    while(!events.empty()) {
        e = events.front();
        switch(e.type) {
            case PEvent::PLAY: StateMachine::changeState(GameState::PLAYING); break;
            case PEvent::PAUSE: StateMachine::changeState(GameState::PAUSE); break;
            case PEvent::CONTINUE: StateMachine::changeState(GameState::PLAYING); break;
            case PEvent::SETTINGS_MENU: StateMachine::changeState(GameState::SETTINGS_MENU); break;
            case PEvent::SETTINGS_PLAYING: StateMachine::changeState(GameState::SETTINGS_PLAYING); break;
            case PEvent::SETTINGS_OK: {
                StateMachine::changeState((StateMachine::state == GameState::SETTINGS_PLAYING)?
                                          (GameState::PAUSE):(GameState::MENU));
            } break;
            case PEvent::EXIT_TO_MENU: StateMachine::changeState(GameState::MENU); break;
            case PEvent::EXIT_TO_DESKTOP: StateMachine::changeState(GameState::DESKTOP); break;
            case PEvent::SPIN_CLICK: {
                Storage::iVariables[e.destination] += e.info;
                if(e.destination > 4 && e.destination < 8)
                    Storage::iVariables[e.destination] =
                            Utility::clamp(Storage::iVariables[e.destination], 0, SOUND_MAX);
                else if(e.destination == 2) {
                    Storage::iVariables[2] = Utility::clamp(Storage::iVariables[2], 0,
                                                            VideoMode::getFullscreenModes().size() -1);
                    Storage::iVariables[3] =
                            VideoMode::getFullscreenModes()[Storage::iVariables[2]].width;
                    Storage::iVariables[4] =
                            VideoMode::getFullscreenModes()[Storage::iVariables[2]].height;
                    MasterRenderer::init();
                }
                Storage::loadGUI("res/config/settings.gui");
            } break;
        }
        events.pop();
    }
}

RectangleShape MasterRenderer::particle(Vector2f(10, 10));
RectangleShape MasterRenderer::healthBar(Vector2f(6, 50));
RectangleShape MasterRenderer::energyBar(Vector2f(6, 50));

int MasterRenderer::WINDOW_W;
int MasterRenderer::WINDOW_H;
Sprite MasterRenderer::sky;
Texture MasterRenderer::starTexture;
Vector2f MasterRenderer::skyOffset(0, 0);
Vector2f MasterRenderer::skyTarget(0, 0);
Sprite MasterRenderer::sEmpty;
Sprite MasterRenderer::cursor;
Shader MasterRenderer::blurShader;
Sprite MasterRenderer::blurred;
RenderWindow MasterRenderer::window;
map<int, PElement*> MasterRenderer::hierarchy;
map<int, PElement*> MasterRenderer::elements;

void MasterRenderer::init() {
    WINDOW_W = Storage::iVariables[3];
    WINDOW_H = Storage::iVariables[4];
    window.create(VideoMode::getFullscreenModes()[Storage::iVariables[2]],
                  "Yura's spaceships", Style::Fullscreen);
    window.setFramerateLimit(120);
    particle.setOrigin(5, 5);
    healthBar.setFillColor(Color(0, 255, 0, 150));
    energyBar.setFillColor(Color(0, 0, 255, 150));
    healthBar.setOrigin(3, 0);
    energyBar.setOrigin(3, 0);
    starTexture.loadFromFile("res/textures/game/glow.png");
    generateSky();
    blurShader.loadFromFile("res/shaders/blur.frag", Shader::Fragment);
    static Texture cursorT;
    cursorT.loadFromFile("res/textures/game/crosshair.png");
    cursor.setTexture(cursorT);
    cursor.setOrigin(12.0f, 12.0f);
    window.setMouseCursorVisible(false);
}

void MasterRenderer::drawGame(RenderTarget *target) {
    // drawing visual effects
        for(unsigned int i = 0; i < World::effects.size(); ++i) {
            World::effects.at(i)->sprite.setPosition(World::effects.at(i)->body->getPosition());
            World::effects.at(i)->sprite.setRotation(World::effects.at(i)->body->getRotation());
            target->draw(World::effects.at(i)->sprite);
        }
        // drawing particles
        for(unsigned int i = 0; i < World::particles.size(); ++i) {
            particle.setFillColor(World::particles.at(i).color);
            particle.setPosition(World::particles.at(i).body->getPosition());
            target->draw(particle);
        }
        // drawing all entities like enemies, obstacles, bullets
        for(unsigned int i = 1; i < World::entities.size(); ++i) {
            World::entities.at(i)->sprite.setPosition(World::entities.at(i)->body->getPosition());
            World::entities.at(i)->sprite.setRotation(World::entities.at(i)->body->getRotation());
            target->draw(World::entities.at(i)->sprite);
            #if DEBUG
                target->draw(*(World::entities.at(i)->body));
            #endif
        }
        // drawing player
        static Color tempColor;
        tempColor = World::player->spriteShield.getColor();
        tempColor.a = World::player->opacity * 255;
        World::player->spriteShield.setColor(tempColor);
        World::player->spriteShield.setPosition(World::player->body->getPosition());
        target->draw(World::player->spriteShield);
        World::player->sprite.setPosition(World::player->body->getPosition());
        World::player->sprite.setRotation(World::player->body->getRotation());
        target->draw(World::player->sprite);

        healthBar.setSize(Vector2f(6, World::player->health / World::player->maxHealth * 50.0f));
        healthBar.setPosition(World::player->body->getPosition() +
                              Vector2f(-World::player->body->getLocalBounds().width / 2.0f - 20.0f,
                                       -healthBar.getSize().y / 2.0f));
        energyBar.setSize(Vector2f(6, World::player->energy / World::player->maxEnergy * 50.0f));
        energyBar.setPosition(World::player->body->getPosition() +
                              Vector2f(-World::player->body->getLocalBounds().width / 2.0f - 10.0f,
                                       -energyBar.getSize().y / 2.0f));
        target->draw(healthBar);
        target->draw(energyBar);
}

void MasterRenderer::drawGUI(RenderTarget *target) {
    for(map<int, PElement*>::iterator it = hierarchy.begin(); it != hierarchy.end(); ++it)
        target->draw(*it->second);
}

void MasterRenderer::drawSky(RenderTarget* target) {
    sky.setPosition(Vector2f(WINDOW_W / 2.0f, WINDOW_H / 2.0f) + skyOffset);
    target->draw(sky);
}

void MasterRenderer::drawCursor(RenderTarget* target) {
    cursor.setPosition(Mouse::getPosition(*(RenderWindow*)target).x,
                       Mouse::getPosition(*(RenderWindow*)target).y);
    target->draw(cursor);
}

void MasterRenderer::generateSky() {
    static vector<Vector3f> stars;
    static Sprite sprite(starTexture);
    static RenderTexture rTexture;
    static unsigned int starCount;
    static Vector3f tempStar;
    static float closestStar;
    static const float STAR_DST = 75.0f;
    static int tiles;
    static unsigned int tries;
    static float dst;
    tiles = MasterRenderer::WINDOW_W * MasterRenderer::WINDOW_H / STAR_DST / STAR_DST;
    stars.clear();
    starCount = tiles;
    starCount += rand() % (int)(tiles * 0.2f);
    for(unsigned int i = 0; i < starCount; ++i) {
        tries = 0;
        do {
            if(tries > 20) break;
            ++tries;
            tempStar.x = rand() % (MasterRenderer::WINDOW_W - 20) + 10;
            tempStar.y = rand() % (MasterRenderer::WINDOW_H - 20) + 10;
            closestStar = 10000.0f;
            for(unsigned int j = 0; j < i; ++j) {
                dst = Utility::distance(Vector2f(tempStar.x, tempStar.y),
                                        Vector2f(stars[j].x, stars[j].y)) / stars[j].z / 10.0f;
                if(dst < closestStar)
                    closestStar = dst;
            }
        } while(closestStar < STAR_DST);
        tempStar.z = (rand() % 21 + 5)  / 100.0f;
        stars.push_back(tempStar);
    }
    rTexture.create(MasterRenderer::WINDOW_W, MasterRenderer::WINDOW_H);
    for(Vector3f star : stars) {
        sprite.setPosition(star.x, star.y);
        sprite.setScale(star.z, star.z);
        rTexture.draw(sprite);
    }
    rTexture.display();
    sky.setTexture(rTexture.getTexture());
    sky.setOrigin(MasterRenderer::WINDOW_W / 2.0f, MasterRenderer::WINDOW_H / 2.0f);
    sky.setPosition(MasterRenderer::WINDOW_W / 2.0f, MasterRenderer::WINDOW_H / 2.0f);
}

void MasterRenderer::targetSky() {
    skyTarget = Vector2f(65.0f - (30.0f * Mouse::getPosition(window).x +
                                  100.0f * World::player->body->getPosition().x) / (float) WINDOW_W,
                        65.0f  - (30.0f * Mouse::getPosition(window).y +
                                  100.0f * World::player->body->getPosition().y) / (float) WINDOW_H);
}

Sprite MasterRenderer::blurTexture(const Texture& texture, float blur) {
    static Texture tex;
    tex = texture;
    tex.setSmooth(true);
    static Sprite spr;
    spr.setTexture(tex);
    spr.setScale(1.0f / BLUR_DOWNSCALE, 1.0f / BLUR_DOWNSCALE);
    static RenderTexture smallT;
    smallT.create(texture.getSize().x / BLUR_DOWNSCALE,
                           texture.getSize().y / BLUR_DOWNSCALE);
    smallT.clear();
    smallT.draw(spr);
    smallT.display();
    tex = smallT.getTexture();
    tex.setSmooth(true);
    spr.setTexture(tex);
    spr.setScale(1.0f, 1.0f);
    blurShader.setParameter("source", tex);
    blurShader.setParameter("offsetFactor", blur, 0);
    smallT.draw(spr, &blurShader);
    tex = smallT.getTexture();
    tex.setSmooth(true);
    spr.setTexture(tex);
    blurShader.setParameter("source", tex);
    blurShader.setParameter("offsetFactor", 0, blur);
    smallT.draw(spr, &blurShader);
    tex = smallT.getTexture();
    tex.setSmooth(true);
    spr.setTexture(tex);
    spr.setScale(BLUR_DOWNSCALE, BLUR_DOWNSCALE);
    return spr;
}

void MasterRenderer::clean() {
    for(pair<int, PElement*> e : elements)
        delete e.second;
    elements.clear();
    hierarchy.clear();
}

void PlayingState::init(GameState::State from) {
    if(from == MENU) {
        Storage::loadResources("res/config/battle.res",
                           Storage::texturesGame, Storage::spritesGame, Storage::fontsGame);
        World::clean();
        World::init();
    }
    Utility::updateTiming();
    MasterRenderer::targetSky();
}

void PlayingState::render() {
    MasterRenderer::window.clear();
    MasterRenderer::drawSky(&MasterRenderer::window);
    MasterRenderer::drawGame(&MasterRenderer::window);
    MasterRenderer::drawCursor(&MasterRenderer::window);
    MasterRenderer::window.display();
}

void PlayingState::update() {
    World::update();
}

void PlayingState::processEvents() {
    Event event;
    while(MasterRenderer::window.pollEvent(event)) {
        if(event.type == Event::Closed)
            MasterRenderer::window.close();
        else if(event.type == Event::MouseMoved) {
            World::player->lookAt(Mouse::getPosition(MasterRenderer::window).x,
                                  Mouse::getPosition(MasterRenderer::window).y);
            MasterRenderer::targetSky();
        } else if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::Space)
                World::entities.push_back(new Rocket(*Storage::spritesGame["rocket"],
                                                     Utility::makeRect(0, 0,
                                                                       Vector2f(40.0f, 10.0f),
                                                                       45),
                                                     0.3f, 1, 100.0f, 50.0f, 0.05f));
            else if(event.key.code == Keyboard::Escape)
                EventManager::events.push(PEvent(PEvent::PAUSE, NULL, 0, 0));
        }
    }
    if(Mouse::isButtonPressed(Mouse::Left))
        World::player->shoot(1.0f);
}

void PlayingState::clean(GameState::State to) {}

float PauseState::blur = 0.0f;
bool PauseState::isBlurring;
float PauseState::dir;
RenderTexture PauseState::background;

void PauseState::init(State from) {
    Storage::loadResources("res/config/pause.res",
                           Storage::texturesGUI, Storage::spritesGUI, Storage::fontsGUI);
    Storage::loadGUI("res/config/pause.gui");
    if(from == GameState::PLAYING) {
        background.create(MasterRenderer::WINDOW_W, MasterRenderer::WINDOW_H);
        MasterRenderer::drawSky(&background);
        MasterRenderer::drawGame(&background);
        background.display();
        isBlurring = true;
        dir = 1.0f;
        blur = 0.001f;
    }
}

void PauseState::render() {
    if(isBlurring)
        MasterRenderer::blurred = MasterRenderer::blurTexture(background.getTexture(), blur);
    MasterRenderer::window.clear();
    MasterRenderer::window.draw(MasterRenderer::blurred);
    if(!isBlurring)
        MasterRenderer::drawGUI(&MasterRenderer::window);
    MasterRenderer::drawCursor(&MasterRenderer::window);
    MasterRenderer::window.display();
}

void PauseState::update() {
    if(isBlurring) {
        blur += Utility::delta * 0.000002f * dir;
        if(blur > 0.002f) {
            isBlurring = false;
            blur = 0.002f;
        } else if(blur < 0.0005f) {
            isBlurring = false;
            blur = 0.0005f;
        }
    }
}

void PauseState::processEvents() {
    static PButton *b;
    static PScrollable *d;
    Event event;
    while(MasterRenderer::window.pollEvent(event)) {
        if(event.type == Event::Closed)
            MasterRenderer::window.close();
        else if(event.type == Event::MouseButtonPressed) {
            if(event.mouseButton.button == Mouse::Left) {
                for(pair<int, PElement*> e : MasterRenderer::elements) {
                    if(e.second->visible && e.second->sprite.getGlobalBounds().contains(
                            Mouse::getPosition(MasterRenderer::window).x,
                            Mouse::getPosition(MasterRenderer::window).y)) {
                        if((b = dynamic_cast<PButton*>(e.second))) {
                            b->click();
                            break;
                        } else if((d = dynamic_cast<PScrollable*>(e.second))) {
                            EventManager::dragging = true;
                        }
                    }
                }
            }
        } else if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::Escape)
                EventManager::events.push(PEvent(PEvent::CONTINUE, NULL, 0, 0));
        }
    }
    /*
    else if(event.type == Event::MouseMoved) {
        if(EventManager::dragging)
            d->Scroll();
    } else if(event.type == Event::MouseButtonReleased) {
        if(event.mouseButton.button == Mouse::Left) {
            if(EventManager::dragging)
                d->Scroll();
            EventManager::dragging = false;
        }
    } else if(event.type == Event::MouseWheelMoved) {
        for(pair<int, PElement*> e : MasterRenderer::elements)
                if(e.second->visible && (d = dynamic_cast<PScrollable*>(e.second)) &&
                        FloatRect(e.second->pos.x, e.second->pos.y,
                                  e.second->size.x, e.second->size.y).contains(
                                    Mouse::getPosition(MasterRenderer::window).x,
                                    Mouse::getPosition(MasterRenderer::window).y)) {
                    d->Scroll(-SIGN(event.mouseWheel.delta));
                }
    }
    */
}

void PauseState::clean(GameState::State to) {
    if(to == GameState::PLAYING) {
        isBlurring = true;
        dir = -2.0;
        while(isBlurring) {
            Utility::updateTiming();
            render();
            update();
        }
    }
    MasterRenderer::clean();
}

void MenuState::init(State from) {
    Storage::loadResources("res/config/menu.res",
                           Storage::texturesGUI, Storage::spritesGUI, Storage::fontsGUI);
    Storage::loadGUI("res/config/menu.gui");
}

void MenuState::render() {
    MasterRenderer::window.clear();
    MasterRenderer::drawSky(&MasterRenderer::window);
    MasterRenderer::drawGUI(&MasterRenderer::window);
    MasterRenderer::drawCursor(&MasterRenderer::window);
    MasterRenderer::window.display();
}

void MenuState::update() {}

void MenuState::processEvents() {
    static PButton *b;
    static PScrollable *d;
    Event event;
    while(MasterRenderer::window.pollEvent(event)) {
        if(event.type == Event::Closed)
            MasterRenderer::window.close();
        else if(event.type == Event::MouseButtonPressed) {
            if(event.mouseButton.button == Mouse::Left) {
                for(pair<int, PElement*> e : MasterRenderer::elements) {
                    if(e.second->visible && e.second->sprite.getGlobalBounds().contains(
                            Mouse::getPosition(MasterRenderer::window).x,
                            Mouse::getPosition(MasterRenderer::window).y)) {
                        if((b = dynamic_cast<PButton*>(e.second))) {
                            b->click();
                            break;
                        } else if((d = dynamic_cast<PScrollable*>(e.second))) {
                            EventManager::dragging = true;
                        }
                    }
                }
            }
        } else if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::Escape)
                EventManager::events.push(PEvent(PEvent::EXIT_TO_DESKTOP, NULL, 0, 0));
        }
    }
}

void MenuState::clean(State to) {
    if(to == DESKTOP) {
        Storage::clean();
        World::clean();
    }
    MasterRenderer::clean();
}

SettingsState::SettingsState(bool _fromGame) : fromGame(_fromGame) {}

void SettingsState::init(State from) {
    Storage::loadResources("res/config/settings.res",
                           Storage::texturesGUI, Storage::spritesGUI, Storage::fontsGUI);
    Storage::loadGUI("res/config/settings.gui");
}

void SettingsState::render() {
    MasterRenderer::window.clear();
    if(fromGame)
        MasterRenderer::window.draw(MasterRenderer::blurred);
    else
        MasterRenderer::drawSky(&MasterRenderer::window);
    MasterRenderer::drawGUI(&MasterRenderer::window);
    MasterRenderer::drawCursor(&MasterRenderer::window);
    MasterRenderer::window.display();
}

void SettingsState::update() {}

void SettingsState::processEvents() {
    static PButton *b;
    static PScrollable *d;
    Event event;
    while(MasterRenderer::window.pollEvent(event)) {
        if(event.type == Event::Closed)
            MasterRenderer::window.close();
        else if(event.type == Event::MouseButtonPressed) {
            if(event.mouseButton.button == Mouse::Left) {
                for(pair<int, PElement*> e : MasterRenderer::elements) {
                    if(e.second->visible && e.second->sprite.getGlobalBounds().contains(
                            Mouse::getPosition(MasterRenderer::window).x,
                            Mouse::getPosition(MasterRenderer::window).y)) {
                        if((b = dynamic_cast<PButton*>(e.second))) {
                            b->click();
                            break;
                        } else if((d = dynamic_cast<PScrollable*>(e.second))) {
                            EventManager::dragging = true;
                        }
                    }
                }
            }
        } else if(event.type == Event::KeyPressed) {
            if(event.key.code == Keyboard::Escape)
                EventManager::events.push(PEvent((fromGame)?(PEvent::PAUSE):(PEvent::EXIT_TO_MENU),
                                                 NULL, 0, 0));
        }
    }
}

void SettingsState::clean(State to) {
    MasterRenderer::clean();
}

GameState::State StateMachine::state = GameState::MENU;
GameState* StateMachine::gameState = NULL;

void StateMachine::prepare() {
    gameState = new MenuState();
    gameState->init(GameState::DESKTOP);
}

void StateMachine::changeState(GameState::State newState) {
    gameState->clean(newState);
    delete gameState;
    switch(newState) {
        case GameState::PLAYING: gameState = new PlayingState(); break;
        case GameState::PAUSE: gameState = new PauseState(); break;
        case GameState::MENU: gameState = new MenuState(); break;
        case GameState::SETTINGS_MENU: gameState = new SettingsState(false); break;
        case GameState::SETTINGS_PLAYING: gameState = new SettingsState(true); break;
        case GameState::DESKTOP: {
            MasterRenderer::window.close();
            state = GameState::DESKTOP;
            return;
        } break;
    }
    gameState->init(state);
    state = newState;
}

void StateMachine::work() {
    Utility::updateTiming();
    gameState->processEvents();
    EventManager::processQueue();
    if(state == GameState::DESKTOP)
        return;
    gameState->update();
    gameState->render();
}
