#ifndef PATY_H
#define PATY_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <fstream>
#include <windows.h>
#include <time.h>
#include <math.h>
#include <iostream>

#define SCROLL_WIDTH 15.0f
#define BLUR_DOWNSCALE 4.0f
#define SOUND_MAX 10
#define SKY_SPEED 0.1f

#define PI 3.14159265
#define SQR(x) ((x)*(x))
#define ABS(x) ((x<0)?(-(x)):(x))
#define MAX(x, y) ((x>y)?(x):(y))
#define SIGN(x) ((x>0)?(1):(-1))
#define DEBUG 0

using namespace std;
using namespace sf;

class Storage {
private:
    static const int VAR_COUNT;
    static void cleanGame();
    static void cleanGUI();
public:
    static map<string, Texture*> texturesGame;
    static map<string, Sprite*> spritesGame;
    static map<string, Font*> fontsGame;
    static map<string, Texture*> texturesGUI;
    static map<string, Sprite*> spritesGUI;
    static map<string, Font*> fontsGUI;
    static map<int, string> strings;
    static vector<string> localizations;
    static int iVariables[];
    static string stringsPath;
    // TODO map for sounds

    static void loadConfiguration(const char *path);
    static void loadResources(const char *path, map<string, Texture*> &textures,
                            map<string, Sprite*> &sprites, map<string, Font*> &fonts);
    static void loadGUI(const char *path);
    static void clean();
};

class Utility {
public:
    static Clock timer;
    static float delta;
    static float minInArray(float *arr, size_t n);
    static float maxInArray(float *arr, size_t n);
    static inline float toRadians(float degree);
    static inline float toDegrees(float radian);
    static bool circle2circle(CircleShape &a, CircleShape &b);
    static inline float distance(Vector2f a, Vector2f b);
    static inline float distance(Vector2f p, Vector2f a, Vector2f b);
    static Vector2f rotatePoint(Vector2f p, Vector2f c, float angle);
    static Vector2f projectPointOnSegment(Vector2f p, Vector2f a, Vector2f b);
    static float whereOnSegment(Vector2f p, Vector2f a, Vector2f b);
    static void getPoints(Shape *s, Vector2f *a);
    static bool rect2circle(RectangleShape &r, CircleShape &c);
    static bool rect2rect(RectangleShape &a, RectangleShape &b);
    static bool isCollision(Shape *a, Shape *b);
    static float radius(Shape *s);
    static Shape* makeRect(Vector2f pos, Vector2f sz, float angle);
    static Shape* makeRect(float x, float y, Vector2f sz, float angle);
    static Shape* makeCircle(Vector2f pos, float radius);
    static Shape* makeCircle(float x, float y, float radius);
    static float angle(Vector2f from, Vector2f to);
    static void updateTiming();
    static Vector2f pointOnCircle(Vector2f pos, float radius, float angle);
    static Vector2f randomizePosition(Vector2f pos, float radius);
    static float parabolicChange(float x, float maxX, float maxY);
    static float gradeBetween(float a, float b, float grade);
    static Color gradeBetween(Color a, Color b, float grade);
    static int clamp(int n, int a, int b);
    static inline float length(Vector2f v);
    static inline Vector2f normalize(Vector2f v);
};

class Entity {
public:
    Shape *body;;
    float speed;
    float health;
    float maxHealth;
    int id;
    bool isAlive;
    Sprite sprite;

    Entity(Sprite _sprite, Shape *_body, float _speed, int _id, float _health);
    Entity(const Entity &e);
    Entity(Entity &&e);

    Entity& operator=(const Entity &e);

    virtual Entity * clone();
    void lookAt(float x, float);
    virtual void Move();
    virtual void update();
    virtual void takeDamage(float _damage, Entity *source);

    virtual ~Entity();
};

class Bullet : public Entity {
public:
    float damage;

    Bullet(Sprite _sprite, Shape *_body, float _speed, int _id, float _health, float _damage);

    virtual ~Bullet() {}
};

class Shooter : public Entity {
private:
    float currTime;
public:
    float damage;
    float reload;

    Shooter(Sprite _sprite,Shape *_body, float _speed, int _id, float _health, float _damage, float _reload);

    void update();
    void shoot(float speed);

    virtual ~Shooter() {}
};

class EntitySpawner {
private:
    float currTime;
    int dir;
    float margin;
    Vector2f spawnPos;
public:
    float delay;

    EntitySpawner(float _delay);

    bool spawn(Entity *Ref);
    void reset();

    virtual ~EntitySpawner() {}
};

class Particle : public Entity {
private:
    Color maxColor;
    bool fade;
public:
    Color color;

    Particle(Sprite _sprite, Shape *_body, float _speed, float _health, Color _color, bool _fade);

    virtual void update();

    virtual ~Particle() {}
};

class ParticleEmitter {
private:
    float currTime;
public:
    float speed;
    float health;
    float radius;
    float delay;
    Color color;
    bool fade;

    ParticleEmitter(float _speed, float _health, float _radius, float _delay, Color _color, bool _fade);

    void emit(Vector2f pos, float angle);
    static void burst(Vector2f pos, int particleCount, int levels, float _speed, float _health, Color _color, bool _fade);

    virtual ~ParticleEmitter() {}
};

class Shield {
private:
    static const float opacityPerTick;
public:
    Entity *parent;
    Sprite spriteShield;
    float radius;
    float energy;
    float maxEnergy;
    float opacity;
    float energyPerTick;
    float restoreDelay;
    float restoring;

    Shield(Entity *_parent);

    void initShield(Sprite _sprite, float _radius, float _energy, float _energyPerTick, float _restoreDelay);
    void update();
    float takeDamage(float _damage, Entity *source);

    virtual ~Shield() {}
};

class Player : public Shooter, public Shield {
private:
    bool wasPressed;
public:
    ParticleEmitter emitter;

    Player(Sprite _sprite, Shape *_body, float _speed, int _id, float _health, float _damage, float _reload);

    virtual Player * clone();
    virtual void Move();
    virtual void update();
    virtual void takeDamage(float _damage, Entity* source);

    virtual ~Player() {}
};

class Chaser {
public:
    float angularSpeed;
    Entity *target;
    Entity *parent;

    Chaser(float _angularSpeed, Entity *_target, Entity *_parent);

    virtual void update();

    virtual ~Chaser() {}
};

class Obstacle : public Entity {
public:
    Obstacle(Sprite _sprite, Shape *_body, float _speed, int _id, float _health);

    virtual Obstacle * clone();

    virtual ~Obstacle() {}
};

class Enemy : public Shooter, public Chaser {
public:
    ParticleEmitter emitter;

    Enemy(Sprite _sprite, Shape *_body, float _speed, int _id, float _health, float _damage, float _reload);
    Enemy(const Enemy &e);

    virtual Enemy * clone();
    virtual void Move();
    virtual void update();

    virtual ~Enemy() {}
};

class Rocket : public Bullet, public Chaser {
public:
    ParticleEmitter emitter;

    Rocket(Sprite _sprite, Shape *_body, float _speed, int _id, float _health, float _damage, float _angularSpeed);

    virtual void Move();
    virtual void update();

    virtual ~Rocket() {}
};

class Sticky : public Entity {
public:
    Entity *target;
    float radius;

    Sticky(Sprite _sprite, Entity* _target, float _radius, int _id);

    virtual void update();
    virtual void Move();

    virtual ~Sticky() {}
};

class PShooter {
private:
    float currTime;
    float id;
public:
    float damage;
    float reload;

    PShooter(float damage, float reload, float id);

    virtual void shoot(Vector2f from, float angle);
    virtual void update();

    virtual ~PShooter() {}
};
/*
class Fighter : public Entity, public PShooter {
private:
    Vector2f target;
public:
    Fighter(Sprite _sprite, Shape *_body, float _speed, int _id, float _health, float _damage, float _reload);

    virtual void update();
    virtual void Move();

    virtual void ~Fighter() {}
};*/

class World {
private:
    static Enemy *sampleEnemy;
    static Obstacle *sampleObstacle;
public:
    static vector<Entity*> entities;
    static vector<Particle> particles;
    static vector<Entity*> effects;

    static Player *player;
    static EntitySpawner spEnemy;
    static EntitySpawner spObstacle;

    static void init();
    static void update();
    static void clean();
};

class PElement : public Drawable {
public:
    PElement *parent;
    Sprite sprite;
    Vector2f pos;
    bool visible;
    Vector2f size;
    vector<PElement*> children;

    PElement();
    virtual ~PElement() {}

    virtual void update(bool *units);
    virtual void update() {}
private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class PLabel : public PElement {
public:
    Text text;

    PLabel();

    virtual void update(bool *units);
    virtual void update() {}
private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class PPanel : public PElement {
public:
    PPanel();
    virtual ~PPanel() {}
private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
};

class PScrollable {
public:
    int scroll;
    virtual void Scroll() = 0;
    virtual void Scroll(int delta) = 0;
};

class PList : public PElement, public PScrollable {
public:
    unsigned int elementsViewed;
    bool scrollable;
    float scrollSpacing;
    float paddingHorizontal;

    PList();
    virtual ~PList() {}

    virtual void update();
    virtual void Scroll();
    virtual void Scroll(int delta);
private:
    virtual void draw(RenderTarget& target, RenderStates states) const;
    virtual void updateVisible();
};

class PTextBox : public PList {
public:
    Text text;
    int labelId;
    bool isJustify;

    PTextBox();
    virtual ~PTextBox();

    virtual void update();
    virtual int getLength(string str);
};

class PEvent {
public:
    enum Type {
        /*  0 */    PLAY,
        /*  1 */    SETTINGS_MENU,
        /*  2 */    EXIT_TO_MENU,
        /*  3 */    PAUSE,
        /*  4 */    CONTINUE,
        /*  5 */    EXIT_TO_DESKTOP,
        /*  6 */    SETTINGS_PLAYING,
        /*  7 */    SETTINGS_OK,
        /*  8 */    SPIN_CLICK
    };
    Type type;
    PElement *source;
    int destination;
    float info;

    PEvent();
    PEvent(Type _type, PElement *_source, int _destination, float _info);
};

class PButton : public PElement {
public:
    PEvent::Type event;
    bool active;
    int destination;
    float info;

    PButton();
    virtual ~PButton() {}

    void click();
};

class EventManager {
public:
    static queue<PEvent> events;
    static bool dragging;
    static Vector2i prevMousePos;
    static Vector2i mouseMove;

    static void processQueue();
};

class MasterRenderer {
private:
    static RectangleShape particle;
    static RectangleShape healthBar;
    static RectangleShape energyBar;
    static Texture starTexture;
public:
    static int WINDOW_W;
    static int WINDOW_H;
    static Sprite sEmpty;
    static Sprite sky;
    static Sprite cursor;
    static Vector2f skyOffset;
    static Vector2f skyTarget;
    static Shader blurShader;
    static RenderWindow window;
    static Sprite blurred;
    static map<int, PElement*> hierarchy;
    static map<int, PElement*> elements;

    static void init();
    static void drawGame(RenderTarget *target);
    static void drawGUI(RenderTarget *target);
    static void drawSky(RenderTarget *target);
    static void drawCursor(RenderTarget *target);
    static void generateSky();
    static void targetSky();
    static Sprite blurTexture(const Texture &texture, float blur);
    static void clean();
};

class GameState {
public:
    enum State {
        MENU,
        PLAYING,
        PAUSE,
        SETTINGS_MENU,
        SETTINGS_PLAYING,
        DESKTOP
    };

    virtual void init(State from) = 0;
    virtual void render() = 0;
    virtual void update() = 0;
    virtual void processEvents() = 0;
    virtual void clean(State to) = 0;

    virtual ~GameState() {};
};

class PlayingState : public GameState{
public:
    void init(State from);
    void render();
    void update();
    void processEvents();
    void clean(State to);

    virtual ~PlayingState() {}
};

class PauseState : public GameState {
private:
    static float blur;
    static bool isBlurring;
    static float dir;
    static RenderTexture background;
public:
    void init(State from);
    void render();
    void update();
    void processEvents();
    void clean(State to);

    virtual ~PauseState() {}
};

class MenuState : public GameState {
public:
    void init(State from);
    void render();
    void update();
    void processEvents();
    void clean(State to);

    virtual ~MenuState() {}
};

class SettingsState : public GameState {
public:
    bool fromGame;

    SettingsState(bool _fromGame);

    void init(State from);
    void render();
    void update();
    void processEvents();
    void clean(State to);

    virtual ~SettingsState() {}
};

class StateMachine {
public:
    static GameState::State state;
    static GameState *gameState;
    static void prepare();
    static void changeState(GameState::State newState);
    static void work();
};

#endif // PATY_H
