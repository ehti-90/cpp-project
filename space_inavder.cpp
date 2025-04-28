#include <SFML/Graphics.hpp>
#include <sstream>
#include <iostream>
#include <ctime>
using namespace std;
using namespace sf;
// Window dimensions


// ---------------------
// Base abstract class for all game objects
// Each object must update and render itself
// ---------------------
class GameObject {
public:
    virtual ~GameObject() {}
    virtual void update(float dt) = 0;   // Update object state
    virtual void render(RenderWindow &window) = 0;  // Draw object
};

// ---------------------
// Player class: handles player sprite and movement
// ---------------------
class Player : public GameObject {
private:
    Texture texture;  // Image for the ship
    Sprite sprite;    // Drawable sprite  -> Sprite without texture = Invisible. No image to show
    float speed;          // Movement speed
public:
    // Load texture and set starting position
    Player(const string &imagePath): speed(300.f)
    {
        texture.loadFromFile(imagePath);
        sprite.setTexture(texture);
        
        // Center origin on sprite
        sprite.setOrigin(texture.getSize().x/2.f, texture.getSize().y/2.f);

        // Start at bottom center of window
        sprite.setPosition(800/2.f, 620 - texture.getSize().y); // width is 800/2=400 on x-axis to the right and 600-52 downward
    }

    // Move left/right based on arrow keys
    void update(float dt) override {   // dt means The time elapsed  since the last frame.
        if (Keyboard::isKeyPressed(Keyboard::Left) &&
            sprite.getPosition().x > texture.getSize().x/2.f) {
            sprite.move(-speed * dt, 0.f);
        }
        if (Keyboard::isKeyPressed(Keyboard::Right) &&
            sprite.getPosition().x < 774) {
            sprite.move(speed * dt, 0.f);
        }
    }

    // Draw player on screen
    void render(RenderWindow &window) override {
        window.draw(sprite);
    }

    // Helpers to get position and collision box
    Vector2f getPosition() const { return sprite.getPosition(); }
    FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};

// ---------------------
// Projectile: base for bullets
// Inherits movement and rendering interface
// ---------------------
class Projectile : public GameObject {
protected:
    sf::RectangleShape shape;  // Simple rectangle shape
    float speed;               // Vertical speed
public:
    Projectile(float x, float y, sf::Color color, float speedY)
        : speed(speedY)
    {
        shape.setSize({5.f, 15.f});   // Width x Height
        shape.setFillColor(color);
        // Center bullet horizontally at x
        shape.setPosition(x - 2.5f, y);
    }
    virtual ~Projectile() {}
    virtual bool offScreen() const = 0;                 // Check if outside
    virtual sf::FloatRect getBounds() const = 0;        // Collision box
};

// ---------------------
// PlayerBullet: moves up
// ---------------------
class PlayerBullet : public Projectile {
public:
    PlayerBullet(float x, float y)
        : Projectile(x, y - 15.f, Color::Red, -500.f) {}

    void update(float dt) override { shape.move(0.f, speed * dt); }
    void render(RenderWindow &window) override { window.draw(shape); }
    bool offScreen() const override { return shape.getPosition().y + shape.getSize().y < 0; }
    FloatRect getBounds() const override { return shape.getGlobalBounds(); }
};

// ---------------------
// EnemyBullet: moves down
// ---------------------
class EnemyBullet : public Projectile {
public:
    EnemyBullet(float x, float y)
        : Projectile(x, y + 15.f, Color::Blue, 300.f) {}

    void update(float dt) override { shape.move(0.f, speed * dt); }
    void render(RenderWindow &window) override { window.draw(shape); }
    bool offScreen() const override { return shape.getPosition().y > 600; }
    FloatRect getBounds() const override { return shape.getGlobalBounds(); }
};

// ---------------------
// Enemy: simple sprite with move logic
// ---------------------
class Enemy : public GameObject {
private:
    Sprite sprite;
public:
    // Create enemy at (x, y)
    Enemy(const Texture &texture, float x, float y) {
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x/2.f, texture.getSize().y/2.f);
        sprite.setPosition(x, y);
    }
    void update(float) override {}        // No self-update
    // External code calls move to slide left/right/down
    void move(float dx, float dy) { sprite.move(dx, dy); }
    void render(RenderWindow &window) override { window.draw(sprite); }
    FloatRect getBounds() const { return sprite.getGlobalBounds(); }
    Vector2f getPosition() const { return sprite.getPosition(); }
};

// ---------------------
// Game: main controller for everything
// ---------------------
class Game {
private:
    RenderWindow window;         // Main window
    Clock clock;                 // Tracks time per frame
    Player player;                   // Player ship
    vector<PlayerBullet*> playerBullets; // Active player shots
    vector<EnemyBullet*> enemyBullets;   // Active enemy shots
    vector<Enemy*> enemies;     // Active enemies
    Texture enemyTexture;        // Shared enemy image
    Texture backgroundTexture;   // Background image
    Sprite backgroundSprite;     // Drawable background
    float enemySpeedX;               // Horizontal speed of enemies
    bool movingRight;                // Direction flag
    float dropDistance;              // How far to drop on edge
    float shootCooldown;             // Delay between player shots
    float shootTimer;                // Tracks time since last shot
    float enemyShootCooldown;        // Delay between enemy shots
    float enemyShootTimer;           // Tracks time since enemy shot
    int score;                       // Player score
    Font font;                   // Font for score text
    Text scoreText;              // Display score

    // Create grid of enemies
    void initEnemies() {
        const float startX = 100.f, startY = 50.f;
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 8; ++col)
                enemies.push_back(new Enemy(enemyTexture,
                    startX + col * 60.f,
                    startY + row * 50.f));
    }
    // Prepare score display
    void initScoreText() {
        font.loadFromFile("ARIBLK.TTF");
        scoreText.setFont(font);
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10.f, 10.f);
    }

public:
    // Constructor loads assets and initializes variables
    Game()
        : window({800 , 600 }, "Space Battle Game"),
          player("ship_recolor_001.png"),
          enemySpeedX(200.f), movingRight(true),
          dropDistance(30.f), shootCooldown(0.2f), shootTimer(0.f),
          enemyShootCooldown(1.f), enemyShootTimer(0.f), score(0)
    {
        window.setFramerateLimit(60);            // Cap to 60 FPS
        std::srand((unsigned)std::time(nullptr));
        enemyTexture.loadFromFile("14.png");   // Load enemy image
        backgroundTexture.loadFromFile("bg5.jpg");
        backgroundSprite.setTexture(backgroundTexture);
        initEnemies();                          // Create enemies
        initScoreText();                        // Set up score text
    }
    //for score getting
    int getScore() const { return score; }

    // Main game loop
    void run() {
        while (window.isOpen()) {
            float dt = clock.restart().asSeconds(); // Time since last frame
            handleInput(dt);   // Respond to keys/events
            update(dt);        // Update game state
            render();          // Draw everything
        }
        // Clean up dynamic memory
        for (auto b : playerBullets) delete b;
        for (auto eb : enemyBullets) delete eb;
        for (auto e : enemies) delete e;
    }

    // Process user input
    void handleInput(float dt) {
        Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == Event::Closed)
                window.close();  // Close window event
        }
        player.update(dt);       // Move player
        shootTimer += dt;        // Track time for shooting
        // Fire bullet if space pressed and cooldown passed
        if (Keyboard::isKeyPressed(Keyboard::Space) &&
            shootTimer >= shootCooldown) {
            playerBullets.push_back(
                new PlayerBullet(
                    player.getPosition().x,
                    player.getPosition().y));
            shootTimer = 0.f;     // Reset shoot timer
        }
    }

    // Update all game objects and handle collisions
    void update(float dt) {
        // Move player bullets and remove off-screen ones
        for (auto b : playerBullets) b->update(dt);
        playerBullets.erase(
            std::remove_if(playerBullets.begin(), playerBullets.end(),
                [](PlayerBullet* b) {
                    bool off = b->offScreen();
                    if (off) delete b;
                    return off;
                }),
            playerBullets.end());
        // Move enemies horizontally and drop on edges
        float dx = enemySpeedX * dt * (movingRight ? 1 : -1);
        bool drop = false;
        for (auto e : enemies) {
            e->move(dx, 0.f);
            auto box = e->getBounds();
            if (box.left <= 0 || box.left + box.width >= 800)
                drop = true;
        }
        if (drop) {
            movingRight = !movingRight;  // Change direction
            for (auto e : enemies)
                e->move(0.f, dropDistance); // Move down
        }
        // Enemy shooting logic: random enemy fires
        enemyShootTimer += dt;
        if (enemyShootTimer >= enemyShootCooldown && !enemies.empty()) {
            int idx = std::rand() % enemies.size();
            auto pos = enemies[idx]->getPosition();
            enemyBullets.push_back(
                new EnemyBullet(pos.x, pos.y));
            enemyShootTimer = 0.f;
        }
        // Move enemy bullets and remove off-screen ones
        for (auto eb : enemyBullets) eb->update(dt);
        enemyBullets.erase(
            std::remove_if(enemyBullets.begin(), enemyBullets.end(),
                [](EnemyBullet* eb) {
                    bool off = eb->offScreen();
                    if (off) delete eb;
                    return off;
                }),
            enemyBullets.end());
        // Check collisions: player bullets vs. enemies
        for (auto it = playerBullets.begin(); it != playerBullets.end();) {
            bool hit = false;
            for (auto eit = enemies.begin(); eit != enemies.end() && !hit;) {
                if ((*it)->getBounds().intersects((*eit)->getBounds())) {
                    delete *eit;                       // Remove enemy
                    eit = enemies.erase(eit);
                    delete *it;                        // Remove bullet
                    it = playerBullets.erase(it);
                    score += 10;                      // Increase score
                    hit = true;
                } else ++eit;
            }
            if (!hit) ++it;
        }
        // Check collisions: enemy bullets vs. player
        for (auto eb : enemyBullets) {
            if (eb->getBounds().intersects(player.getBounds())) {
                window.close(); // End game on hit
            }
        }
        // Update score display text
        stringstream ss;
        ss << "Score: " << score;
        scoreText.setString(ss.str());
    }

    // Draw all objects
    void render() {
        window.clear();                     // Clear previous frame
        window.draw(backgroundSprite);      // Draw background image
        player.render(window);              // Draw player
        // Draw each player bullet
        for (size_t i = 0; i < playerBullets.size(); ++i)
            playerBullets[i]->render(window);
        // Draw each enemy
        for (size_t i = 0; i < enemies.size(); ++i)
            enemies[i]->render(window);
        // Draw each enemy bullet
        for (size_t i = 0; i < enemyBullets.size(); ++i)
            enemyBullets[i]->render(window);
        window.draw(scoreText);             // Draw score text
        window.display();                   // Show this frame
    }
};

// Program entry point
int main() {

    Game game;
    game.run();  
    cout<<"--------------------------------------------------\n";
    cout << "Final Score: " << game.getScore() << endl;
    cout<<"--------------------------------------------------\n";
    return 0;
}
