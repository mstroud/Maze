#include <SFML/Graphics.hpp>
#include "level.h"

#define HORIZONTAL 640
#define VERTICAL 480
#define LEVELHORIZONTAL 40
#define LEVELVERTICAL 30
#define PI 3.14159265

void ProcessEvents(sf::RenderWindow &window);
void Render(sf::RenderWindow &window, struct Player player);
void Rotate(struct Player *player, int dir);
void RotateArb(struct Player *player, float angle);
void Move(struct Player *player, int dir);
int isInsideWall(float x, float y, int level[][LEVELHORIZONTAL]);
void CastRays(struct Player *player, struct Vec2d rays[], int level[][LEVELHORIZONTAL]);

struct Vec2d
{
	float x;
	float y;
} vec2d;

struct Player
{
	/* Location on the screen. */
	struct Vec2d location;
	/* Direction vector. */
	struct Vec2d direction;
	/* Since this version of the game has no time keeping, just
	adjust these numbers until they feel right :) */
	float movementSpeed;
	float turningSpeed;
};

struct Player player;
struct Vec2d rays[480];

int main()
{
	/* Set the player in the center of the screen, and facing down.	Remember that our coordinates 
	are in pixel locations, so the	location (0,0) is in the top left of the screen, and X increments
	to the right and Y increments downwards. */
	player.location.x = HORIZONTAL / 2;
	player.location.y = VERTICAL / 2;
	player.direction.x = 0;
	player.direction.y = 1;
	player.movementSpeed = 5.f;
	player.turningSpeed = .1f;
	
	/* Initialize and open the SFML window.*/
	sf::RenderWindow window(sf::VideoMode(HORIZONTAL, VERTICAL), "Maze");

	/* While the window is open, process the keyboard events, update the global data
	and then render the data to screen. */
	while (window.isOpen())
	{
		ProcessEvents(window);
		CastRays(&player, rays, level);
		Render(window, player);
	}

	return 0;
}

void ProcessEvents(sf::RenderWindow &window)
{
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed) { window.close(); }

		if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::A)
			{
				Rotate(&player, 1);
			}

			if (event.key.code == sf::Keyboard::D)
			{
				Rotate(&player, 0);
			}

			if (event.key.code == sf::Keyboard::W)
			{
				Move(&player, 0);
			}

			if (event.key.code == sf::Keyboard::S)
			{
				Move(&player, -1);
			}
		}
	}
}

void Render(sf::RenderWindow &window, struct Player player)
{
	/* Clear the window. */
	window.clear();
	
	/* Draw the level. */
	sf::VertexArray varray(sf::Points);
	int x = 0, y = 0;
	for (y = 0; y < VERTICAL; y++)
	{
		for (x = 0; x < HORIZONTAL; x++)
		{
			if (isInsideWall((float)x, (float)y, level))
			{
				sf::Vertex vertex(sf::Vector2f(x, y), sf::Color::Red);
				varray.append(vertex);
			}
		}
	}
	window.draw(varray);
	
	/* The player is a circle with a line indicating direction. */

	/* Set up and draw the player as a circle. */
	sf::CircleShape playerCircle;
	float playerCircleRadius = 10;
	playerCircle.setRadius(playerCircleRadius);
	playerCircle.setPosition(player.location.x, player.location.y);
	playerCircle.setFillColor(sf::Color::Blue);
	window.draw(playerCircle);

	/* Set up and draw the line indicating the player direction. */
	int playerDirectionLineLength = 25;
	sf::Vector2f lineFrom(player.location.x + playerCircleRadius, player.location.y + playerCircleRadius);
	sf::Vector2f lineTo(player.location.x + (player.direction.x * playerDirectionLineLength) + playerCircleRadius, player.location.y + (player.direction.y * playerDirectionLineLength) + playerCircleRadius);
	sf::Vertex line[] =
	{
		sf::Vertex(lineFrom),
		sf::Vertex(lineTo)
	};
	window.draw(line, 2, sf::Lines);

	int i = 0;
	for (i = 0; i < 1; i++)
	{
		sf::Vector2f lineFrom(player.location.x + playerCircleRadius, player.location.y + playerCircleRadius);
		sf::Vector2f lineTo(rays[i].x, rays[i].y);
		sf::Vertex linex[] =
		{
			sf::Vertex(lineFrom),
			sf::Vertex(lineTo)
		};
		window.draw(linex, 2, sf::Lines);
	}

	/* Display what's been drawn to the screen. */
	window.display();
}

void Rotate(struct Player *player, int dir)
{
	/* Negative angle is counter-clockwise. */
	float angle = player->turningSpeed;
	if (dir)
	{
		angle *= -1;
	}

	/* Apply rotation to the direction vector. */
	float newX = (player->direction.x * cos(angle)) - (player->direction.y * sin(angle));
	float newY = (player->direction.x * sin(angle)) + (player->direction.y * cos(angle));
	player->direction.x = newX;
	player->direction.y = newY;
}

void RotateArb(struct Player *player, float degrees)
{
	/* Negative angle is counter-clockwise. */
	float angle = ((float)PI / 180.0) * degrees;
	/* Apply rotation to the direction vector. */
	float newX = (player->direction.x * cos(angle)) - (player->direction.y * sin(angle));
	float newY = (player->direction.x * sin(angle)) + (player->direction.y * cos(angle));
	player->direction.x = newX;
	player->direction.y = newY;
}

void Move(struct Player *player, int dir)
{
	/* Negative direction is backwards. */
	float movement = player->movementSpeed;
	if (dir)
	{
		movement *= -1;
	}

	/* Apply movement along the player direction to the location. */
	float newX = player->location.x + (player->direction.x * movement);
	float newY = player->location.y + (player->direction.y * movement);
	player->location.x = newX;
	player->location.y = newY;
}

int isInsideWall(float x, float y, int level[][LEVELHORIZONTAL])
{
	/* Transform the pixel location to a location in the level array. */
	float levelX = x * ((float)LEVELHORIZONTAL / (float)HORIZONTAL);
	float levelY = y * ((float)LEVELVERTICAL / (float)VERTICAL);

	if (level[(int)levelY][(int)levelX] > 0)
	{
		printf("Inside Wall X: %f, Y:%f\n", levelX, levelY);
		//printf("%i\n", level[(int)levelY][(int)levelX]);
		return 1;
	}
	else
	{
		printf("NOT Inside Wall X: %f, Y:%f\n", levelX, levelY);
		return 0;
	}
	return 0;
}

void CastRays(struct Player *player, struct Vec2d rays[], int level[][LEVELHORIZONTAL])
{
	/* Here's the tricky part. I need to cast 480 rays in a 60 degree arc in the direction
	of the player, and then store the end points in the rays array for later rendering or
	drawing of walls. */

	/* Let's start by setting a ray casting start point to be -30 degrees from the player's
	direction vector. */

	struct Player rayPlayer;
	rayPlayer.location.x = player->location.x;
	rayPlayer.location.y = player->location.y;
	rayPlayer.direction.x = player->direction.x;
	rayPlayer.direction.y = player->direction.y;
	/* We want to step 30 degrees the first time. */

	rayPlayer.movementSpeed = .01;

	RotateArb(&rayPlayer, 30);

	/* Ok let's set the rotation angle to .125 degrees. */
	rayPlayer.turningSpeed = ((float)PI / 180.f) * .125;
	
	int hit = 0, attempt = 0;
	int i = 0, rayWall = 0, rayBounds = 0;

	for (i = 0; i < 1; i++)
	{
		hit = 0;
		rayPlayer.location.x = player->location.x;
		rayPlayer.location.y = player->location.y;

		while (!hit)
		{
			//Move(&rayPlayer, 0);
			if (isInsideWall(rayPlayer.location.x, rayPlayer.location.y, level))
			{
				rays[i].x = rayPlayer.location.x;
				rays[i].y = rayPlayer.location.y;
				hit = 1;
				rayWall++;
			}

			if (rayPlayer.location.x > HORIZONTAL || rayPlayer.location.y > VERTICAL)
			{
				rays[i].x = rayPlayer.location.x;
				rays[i].y = rayPlayer.location.y;
				hit = 1;
				rayBounds++;
			}

			attempt++;
			if (attempt > 50) { hit = 1; }
		}
		//RotateArb(&rayPlayer, -0.125);
	}
	printf("Wall: %i, Bounds: %i\n", rayWall, rayBounds);


}
