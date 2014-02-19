#include <SFML/System/Vector2.hpp>

struct Player
{
	/* Location on the screen. */
	sf::Vector2f location;
	/* Direction vector. */
	sf::Vector2f direction;
	/* Since this version of the game has no time keeping, just
	adjust these numbers until they feel right :) */
	float movementSpeed;
	float turningSpeed;
};
