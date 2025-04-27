#include <SFML/Graphics.hpp>
#include <vector>
#include <sstream>
#include <iostream>

// ---------------------
// Class: Player
// Represents the player's ship using a texture sprite.
// ---------------------
class Player {
private:
    sf::Texture texture;  // Image for the ship
    sf::Sprite sprite;    // Sprite (displayable version) of the ship
    float speed;          // How fast the player moves
public:
    // Constructor: Load ship image and set initial position
    Player(const std::string &imagePath, float startX, float startY) {
        if (!texture.loadFromFile(imagePath)) { // Try to load image
            sf::Image fallback;                 // If fail, create a simple green rectangle
            fallback.create(50, 30, sf::Color::Green);
            texture.loadFromImage(fallback);
        }
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f); // Center origin
        sprite.setPosition(startX, startY);
        speed = 300.f; // Movement speed in pixels per second
    }

    // Update player's position based on keyboard input
    void update(float dt) {
        float x = sprite.getPosition().x;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && x - texture.getSize().x / 2.f > 0)
            sprite.move(-speed * dt, 0.f); // Move left
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && x + texture.getSize().x / 2.f < 800)
            sprite.move(speed * dt, 0.f);  // Move right
    }

    // Draw the player on the window
    void render(sf::RenderWindow &window) {
        window.draw(sprite);
    }

    // Get the current position of the player (needed for shooting bullets)
    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }
};

// ---------------------
// Class: Bullet
// Represents a shot fired by the player.
// ---------------------
class Bullet {
private:
    sf::RectangleShape shape; // Simple rectangle shape for the bullet
    float speed;              // Speed of bullet
public:
    // Constructor: Set initial position and properties
    Bullet(float startX, float startY) {
        shape.setSize({5.f, 15.f});          // Width and Height
        shape.setFillColor(sf::Color::Red);  // Bullet color
        shape.setPosition(startX - 2.5f, startY - 15.f); // Center bullet above player
        speed = 500.f;
    }

    // Move bullet upwards
    void update(float dt) { shape.move(0.f, -speed * dt); }

    // Draw the bullet
    void render(sf::RenderWindow &window) { window.draw(shape); }

    // Check if bullet is outside screen
    bool isOffScreen() const { return shape.getPosition().y + shape.getSize().y < 0; }

    // Get bullet's bounding box (used for collision)
    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
};

// ---------------------
// Class: Enemy
// Represents an invader using a shared texture sprite.
// ---------------------
class Enemy {
private:
    sf::Sprite sprite;  // Enemy sprite
public:
    // Constructor: Set up enemy using a shared texture
    Enemy(const sf::Texture &texture, float startX, float startY) {
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x / 2.f, texture.getSize().y / 2.f);
        sprite.setPosition(startX, startY);
    }

    // Move the enemy by dx, dy
    void update(float dx, float dy) { sprite.move(dx, dy); }

    // Draw the enemy
    void render(sf::RenderWindow &window) { window.draw(sprite); }

    // Get enemy's bounding box (used for collision)
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};

// ---------------------
// Class: Game
// Manages the game loop, loading textures, and all game entities.
// ---------------------
class Game {
private:
    sf::RenderWindow window;  // Game window
    sf::Clock clock;          // Keeps track of time
    Player player;            // The player
    std::vector<Bullet> bullets;  // List of bullets
    std::vector<Enemy> enemies;   // List of enemies
    sf::Texture enemyTexture;     // Shared enemy texture
    float enemySpeedX;            // Speed enemies move horizontally
    bool movingRight;             // Direction of enemy movement
    float dropDistance;           // Distance enemies drop when reaching edge
    float shootCooldown;          // Minimum time between player shots
    float shootTimer;             // Timer for shooting
    int score;                    // Player score
    sf::Font font;                // Font for displaying score
    sf::Text scoreText;           // Text object for score display

    // Create enemy formation
    void initEnemies() {
        const float startX = 100.f;
        const float startY = 50.f;
        const int rows = 3, cols = 7; // 3 rows and 7 columns
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                enemies.emplace_back(enemyTexture, startX + j * 60.f, startY + i * 50.f);
    }

    // Set up score display text
    void initScoreText() {
        if (!font.loadFromFile("ARIBLK.TTF")) {
            // If font fails to load, do nothing
        }
        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setPosition(10.f, 10.f);
    }

public:
    // Constructor: Set up game
    Game()
    : window(sf::VideoMode(800, 600), "Space Invaders"), // Window size
      player("ship_recolor_001.png", 400.f, 550.f)       // Player position
    {
        window.setFramerateLimit(60); // Limit FPS for smoothness

        // Try to load enemy image
        if (!enemyTexture.loadFromFile("14.png")) {
            sf::Image fallback;
            fallback.create(32, 32, sf::Color::Blue);
            enemyTexture.loadFromImage(fallback);
        }

        initEnemies();
        initScoreText();

        enemySpeedX = 100.f;
        movingRight = true;
        dropDistance = 10.f;
        shootCooldown = 0.5f; // Player can shoot every 0.5 seconds
        shootTimer = 0.f;
        score = 0;
    }

    // Game loop: run until the window is closed
    void run() {
        while (window.isOpen()) {
            float dt = clock.restart().asSeconds(); // Delta time
            handleInput(dt); // Process inputs
            update(dt);      // Update game objects
            render();        // Draw everything
        }
    }

    // Handle input events like moving and shooting
    void handleInput(float dt) {
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed)
                window.close(); // Close window if requested
        }

        player.update(dt); // Update player's position

        shootTimer += dt;  // Increase shooting timer
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && shootTimer >= shootCooldown) {
            bullets.emplace_back(
                player.getPosition().x,
                player.getPosition().y - 20.f
            );
            shootTimer = 0.f; // Reset shoot timer
        }
    }

    // Update bullets, enemies, and check collisions
    void update(float dt) {
        // Move all bullets
        for (auto &b : bullets) b.update(dt);

        // Remove bullets that go off screen
        bullets.erase(
            std::remove_if(bullets.begin(), bullets.end(),
                [](const Bullet &b){ return b.isOffScreen(); }),
            bullets.end()
        );

        // Move enemies horizontally
        float dx = enemySpeedX * dt * (movingRight ? 1 : -1);
        bool needDrop = false;
        for (auto &e : enemies) {
            e.update(dx, 0.f);
            auto bounds = e.getBounds();
            if (bounds.left <= 0.f || bounds.left + bounds.width >= 800.f)
                needDrop = true; // If an enemy hits edge
        }

        // If enemies hit wall, move down and change direction
        if (needDrop) {
            movingRight = !movingRight;
            for (auto &e : enemies) e.update(0.f, dropDistance);
        }

        // Check for bullet-enemy collisions
        for (auto bIt = bullets.begin(); bIt != bullets.end();) {
            bool erased = false;
            for (auto eIt = enemies.begin(); eIt != enemies.end() && !erased;) {
                if (bIt->getBounds().intersects(eIt->getBounds())) {
                    eIt = enemies.erase(eIt); // Remove enemy
                    bIt = bullets.erase(bIt); // Remove bullet
                    score += 10;              // Increase score
                    erased = true;
                } else ++eIt;
            }
            if (!erased) ++bIt;
        }

        // Update score text
        std::stringstream ss;
        ss << "Score: " << score;
        scoreText.setString(ss.str());
    }

    // Draw everything
    void render() {
        window.clear();        // Clear screen
        player.render(window); // Draw player
        for (auto &b : bullets) b.render(window); // Draw bullets
        for (auto &e : enemies) e.render(window); // Draw enemies
        window.draw(scoreText); // Draw score
        window.display();       // Show everything
    }
};

// ---------------------
// Main function
// Entry point of the game
// ---------------------
int main() {
    Game game;
    game.run();
    return 0;
}
