#include <iostream>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/UDPSocket.hpp>
#include "level.h"
#include "player.hpp"
#include "server.hpp"

#define HORIZONTAL 640
#define VERTICAL 480
#define LEVELHORIZONTAL 40
#define LEVELVERTICAL 30
#define PI 3.14159265

void ProcessEvents(sf::RenderWindow &window);
void Render2d(sf::RenderWindow &window, Player player);
void Render3d(sf::RenderWindow &window);
void Rotate(Player *player, int dir);
void RotateArb(Player *player, float angle);
void Move(Player *player, int dir);
void MoveArb(Player *player, int distance);
int isInsideWall(float x, float y, int level[][LEVELHORIZONTAL]);
void CastRays(Player *player, sf::Vector2f rays[], int level[][LEVELHORIZONTAL]);
void DebugRays();
float VectorLength(sf::Vector2f vec1, sf::Vector2f vec2);
void CalcRayDistances();

int renderType = 1;

Server server(1234);
Player player;
sf::Vector2f rays[640];
float distances[640];

int main()
{
  sf::Thread thread(&Server::Listen, &server);

// start the thread
thread.launch();

sf::sleep(sf::seconds(5.0f));
sf::UdpSocket socket;
socket.bind(1235);
std::string message = "Hi, I am " + sf::IpAddress::getLocalAddress().toString();
socket.send(message.c_str(), message.size() + 1, "127.0.0.1", 1234);

// block execution until the thread is finished
thread.wait();


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
		CalcRayDistances();
		if (renderType > 0){ Render2d(window, player); }
		else { Render3d(window); }
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

			if (event.key.code == sf::Keyboard::J)
			{
				RotateArb(&player, -30);
			}

			if (event.key.code == sf::Keyboard::L)
			{
				RotateArb(&player, .125);
			}

			if (event.key.code == sf::Keyboard::I)
			{
				MoveArb(&player, 1);
				if (isInsideWall(player.location.x, player.location.y, level))
				{
					printf("Inside a wall.\n");
				}
				else { printf("\n"); }
			}
			
			if (event.key.code == sf::Keyboard::R)
			{
				DebugRays();
			}

			if (event.key.code == sf::Keyboard::Q)
			{
				renderType *= -1;
			}
		}
	}
}

void Render2d(sf::RenderWindow &window, Player player)
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
		sf::Vertex(lineFrom, sf::Color::Green),
		sf::Vertex(lineTo)
	};
	

	int i = 0;
	for (i = 0; i < 640; i++)
	{
		sf::Vector2f lineFrom(player.location.x + playerCircleRadius, player.location.y + playerCircleRadius);
		sf::Vector2f lineTo(rays[i].x, rays[i].y);
		sf::Vertex linex[] =
		{
			sf::Vertex(lineFrom),
			sf::Vertex(lineTo)
		};
		window.draw(linex, 2, sf::Lines);
		window.draw(line, 2, sf::Lines);
	}

	/* Display what's been drawn to the screen. */
	window.display();
}

void Render3d(sf::RenderWindow &window)
{
	window.clear();
	int i = 0;
	float height = 0;

	/* Draw 640 vertical lines across the screen that correspond to the height of the
	wall that was detected */
	for (i = 0; i < 640; i++)
	{
		/* Unfortunately a lot of the height equation numbers are voodoo. I tweaked them until they
		looked right on screen. I'm not using a proper view window, so this is a consequence. */
		height = (20.0 / distances[i]) * 800.0;

		sf::Vector2f lineFrom((float)i, 240.0 - (height / 2.0));
		sf::Vector2f lineTo((float)i, 240.0 + (height / 2.0));
		sf::Vertex line[] =
		{
			sf::Vertex(lineFrom, sf::Color::Red),
			sf::Vertex(lineTo, sf::Color::Blue)
		};
		window.draw(line, 2, sf::Lines);
	}

	window.display();
}

void Rotate(Player *player, int dir)
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

void RotateArb(Player *player, float degrees)
{
	/* Negative angle is counter-clockwise. */
	float angle = ((float)PI / 180.0) * degrees;
	/* Apply rotation to the direction vector. */
	float newX = (player->direction.x * cos(angle)) - (player->direction.y * sin(angle));
	float newY = (player->direction.x * sin(angle)) + (player->direction.y * cos(angle));
	player->direction.x = newX;
	player->direction.y = newY;
}

void Move(Player *player, int dir)
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

void MoveArb(Player *player, int distance)
{
	/* Negative direction is backwards. */
	float movement = distance;
	
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
		//printf("%i\n", level[(int)levelY][(int)levelX]);
		return 1;
	}
	else
	{
		return 0;
	}
	return 0;
}

void CastRays(Player *player, sf::Vector2f rays[], int level[][LEVELHORIZONTAL])
{
	/* Here's the tricky part. I need to cast 640 rays in a 60 degree arc in the direction
	of the player, and then store the end points in the rays array for later rendering or
	drawing of walls. */

	/* I'm treating the rays like a player moving forward until it hits a wall,
	so I'm going to set up rayPlayer to have the same location and direction as the player. */
	Player rayPlayer;
	rayPlayer.location.x = player->location.x;
	rayPlayer.location.y = player->location.y;
	rayPlayer.direction.x = player->direction.x;
	rayPlayer.direction.y = player->direction.y;
	
	/* Let's start by setting a ray casting start point to be -30 degrees from the player's
	direction vector. */
	RotateArb(&rayPlayer, -30);

	int hit = 0;
	int i = 0;

	for (i = 0; i < 640; i++)
	{
		/* Each iteration through the loop resets the hit status, and resets the
		rayPlayer back to the player location. */
		hit = 0;
		
		rayPlayer.location.x = player->location.x;
		rayPlayer.location.y = player->location.y;

		while (!hit)
		{
			/* Cast the ray forward by 1. */
			MoveArb(&rayPlayer, 1);
			
			/* Check to see if the ray is inside the wall, or past the level boundaries. 
			If so, record the position and set hit=1*/
			if (isInsideWall(rayPlayer.location.x, rayPlayer.location.y, level))
			{
				rays[i].x = rayPlayer.location.x;
				rays[i].y = rayPlayer.location.y;
				hit = 1;
			}

			if (rayPlayer.location.x > HORIZONTAL || rayPlayer.location.y > VERTICAL)
			{
				rays[i].x = rayPlayer.location.x;
				rays[i].y = rayPlayer.location.y;
				hit = 1;
			}
		}

		/* Rotate the rayPlayer so that it will start casting along a new direction. */
		RotateArb(&rayPlayer, 0.09375);
	}


}

void DebugRays()
{
	int i;
	for (i = 0; i < 480; i++)
	{
		printf("R: %i RX: %f, RY: %f\n", i, rays[i].x, rays[i].y);
	}
}

float VectorLength(sf::Vector2f vec1, sf::Vector2f vec2)
{
	float dx = vec1.x - vec2.x;
	float dy = vec1.y - vec2.y;
	return sqrtf((dx * dx) + (dy * dy));
}

void CalcRayDistances()
{
	int i = 0;
	for (i = 0; i < 640; i++)
	{
		distances[i] = VectorLength(player.location, rays[i]);
	}
}