#include "Mode.hpp"

#include "Scene.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, W, S, A, D;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *maze = nullptr;
	Scene::Transform *player = nullptr;
    
    float player_radius = 0.3f;
    
    struct Wall {
        glm::vec3 min; //bottom-left corner
        glm::vec3 max; //top-right corner
    };

    std::vector<Wall> maze_walls;
    
	
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
     
	float wobble = 0.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
