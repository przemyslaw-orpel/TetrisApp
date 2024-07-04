#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <ctime>
#include <cstdlib>

//Author: S7443 Przemyslaw Orpel 

// Window properties
const int ROWS = 20;
const int COLUMNS = 10;
const int BLOCK_SIZE = 30;

// Game field
int field[ROWS][COLUMNS] = { 0 };

// Tetrimino segment positions
struct Point {
	int x, y;
	Point(int x = 0, int y = 0) : x(x), y(y) {}
};

class Tetris {
private:
	Point act_position[4], temp_position[4];

	// Tetrimino figures
	int figures[7][4] = {
		1, 3, 5, 7, // I
		2, 4, 5, 7, // Z
		3, 5, 4, 6, // S
		3, 5, 4, 7, // T
		2, 3, 5, 7, // L
		3, 5, 7, 6, // J
		2, 3, 4, 5  // O
	};
	int dx = 0;
	bool rotate = false;
	bool gameEnd = false;

public:
	Tetris() {
		int n = rand() % 7;
		for (int i = 0; i < 4; i++) {
			// Center 
			act_position[i].x = figures[n][i] % 2 +(COLUMNS / 2) - 1;
			act_position[i].y = figures[n][i] / 2 -1;
		}
	}

	bool checkCollision() {
		bool isOk = true;
		for (int i = 0; i < 4; i++) {
			// Window collision
			if (act_position[i].x < 0 || act_position[i].x >= COLUMNS || act_position[i].y >= ROWS) {
				isOk = false;
				// Field collision
			}else if (field[act_position[i].y][act_position[i].x]){
					isOk = false;
			}
			// Check end game
			if (!isOk && act_position[i].y == 1)
				gameEnd = true;
		}		
		return isOk;
	}

	void move() {
		// Move horizontally
		for (int i = 0; i < 4; i++) {
			temp_position[i] = act_position[i];
			act_position[i].x += dx;
		}
		if (!checkCollision()) {
			for (int i = 0; i < 4; i++) act_position[i] = temp_position[i];
		}
	}

	void rotateTetrimino() {
		// Center of rotation
		Point p = act_position[1]; 
		for (int i = 0; i < 4; i++) {
			int x = act_position[i].y - p.y;
			int y = act_position[i].x - p.x;
			act_position[i].x = p.x - x;
			act_position[i].y = p.y + y;
		}
		if (!checkCollision()) {
			for (int i = 0; i < 4; i++) act_position[i] = temp_position[i];
		}
	}

	void timeMove() {
		// Move vertically
		for (int i = 0; i < 4; i++) {
			temp_position[i] = act_position[i];
			act_position[i].y += 1;
		}

		if (!checkCollision() && !gameEnd) {
			// Block Tetrimino positions
			for (int i = 0; i < 4; i++) {
				field[temp_position[i].y][temp_position[i].x] = 1;
			}

			//New Tetrimino
			int n = rand() % 7;
			for (int i = 0; i < 4; i++) {
				act_position[i].x = figures[n][i] % 2 + (COLUMNS / 2 ) - 1;
				act_position[i].y = figures[n][i] / 2 - 1;
			}
		}
	}

	// Getter & Setters
	void setDirection(int direction) {
		dx = direction;
	}

	void setRotate(bool rot) {
		rotate = rot;
	}

	bool getGameEnd() {
		return gameEnd;
	}

	Point* getActivePositions() {
		return act_position;
	}
};

class Game {
private:
	sf::RenderWindow window;
	sf::RectangleShape block;
	Tetris tetris;
	int dx = 0;
	bool rotate = false;
	float timer = 0, delay = 0.3;
	sf::Clock clock;
	sf::Color act_block_color;
	sf::Color old_block_color;
public:
	Game() : window(sf::VideoMode(COLUMNS * BLOCK_SIZE, ROWS * BLOCK_SIZE), "Tetris Game"),
		block(sf::Vector2f(BLOCK_SIZE - 1, BLOCK_SIZE - 1)) {
		srand(static_cast<unsigned int>(time(0)));
		act_block_color = sf::Color(255, 32, 32);
		old_block_color = sf::Color(32, 32, 255);
	}

	void run() {
		while (window.isOpen()) {
			// Register move time
			setTime();
			// Check user input
			processEvents();
			// Update Tetrimino position and check rules
			update();
			// Render window
			render();
		}
	}
	void setTime() {
		// Get ulapsed time [s] 
		float time = clock.getElapsedTime().asSeconds();
		// Reset clock
		clock.restart();
		// Update timer
		timer += time;
	}

	void processEvents() {
		dx = 0;
		rotate = false;
		sf::Event event;
		// Pop the event on top of the event queue, if any, and return it.
		while (window.pollEvent(event)) {
			// Windows close button
			if (event.type == sf::Event::Closed)
				window.close();
			// Chceck user pressed key
			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Up) {
					rotate = true;
				}
				else if (event.key.code == sf::Keyboard::Left) {
					dx = -1;
				}
				else if (event.key.code == sf::Keyboard::Right) {
					dx = 1;
				}
			}
		}
		// Fast move down 
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) delay = 0.05;
	}

	void update() {
		// Move horizontally
		if (dx != 0) {
			tetris.setDirection(dx);
			tetris.move();
		}

		// Rotate
		if (rotate) {
			tetris.setRotate(true);
			tetris.rotateTetrimino();
		}

		// Tick
		if (timer > delay) {
			tetris.timeMove();
			timer = 0;
		}

		// Reset dx and delay for the next frame
		dx = 0;
		delay = 0.3;
		// Check space in rows
		checkLines();
		if (tetris.getGameEnd())
			window.close();
	}

	void render() {
		window.clear(sf::Color::White);

		// Old Tetrimino
		for (int i = 0; i < ROWS; i++)
			for (int j = 0; j < COLUMNS; j++) {
				if (field[i][j] == 0) continue;
				block.setFillColor(old_block_color);
				block.setPosition(j * BLOCK_SIZE, i * BLOCK_SIZE);
				window.draw(block);
			}

		// Active Tetrimino
		Point* act_position = tetris.getActivePositions();
		for (int i = 0; i < 4; i++) {
			block.setFillColor(act_block_color);
			block.setPosition(act_position[i].x * BLOCK_SIZE, act_position[i].y * BLOCK_SIZE);
			window.draw(block);
		}

		window.display();
	}

	void checkLines() {
		// Check lines
		int k = ROWS - 1;
		for (int i = ROWS - 1; i > 0; i--) {
			int count = 0;
			for (int j = 0; j < COLUMNS; j++) {
				if (field[i][j]) count++;
				field[k][j] = field[i][j];
			}
			if (count < COLUMNS) k--;
		}
	}
};

int main() {
	Game game;
	game.run();
	return 0;
}
