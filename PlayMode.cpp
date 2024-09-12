#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint maze_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > maze_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("maze.pnct"));
	maze_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > maze_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("maze.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = maze_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = maze_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

const glm::vec3 PlayMode::gravity= glm::vec3(0.0f, -8.0f, 0.0f);
glm::vec3 player_velocity = glm::vec3(0.0f, 0.0f, 0.0f);

PlayMode::PlayMode() : scene(*maze_scene) {
    /*
	//get pointers to leg for convenience:
	for (auto &transform : scene.transforms) {
		if (transform.name == "Hip.FL") hip = &transform;
	}
     
    
	if (hip == nullptr) throw std::runtime_error("Hip not found.");

	hip_base_rotation = hip->rotation;
	upper_leg_base_rotation = upper_leg->rotation;
	lower_leg_base_rotation = lower_leg->rotation;
    */
    for (auto& transform : scene.transforms) {
        if (transform.name == "Maze") {
            maze = &transform;
        }else if (transform.name == "Sphere"){
            player = &transform;
        }else if (transform.name == "Door"){
            door = &transform;
        }else if (transform.name.find("Cube") != std::string::npos) {
            glm::vec3 wall;
            wall.x = -transform.position.y;
            wall.y = transform.position.z;
            wall.z = transform.position.x;
            glm::vec3 size = glm::vec3(transform.scale.x, transform.scale.y, transform.scale.z);

            glm::vec3 min = wall - size;
            glm::vec3 max = wall + size;

            maze_walls.push_back(Wall{min, max});
        }
    }
   
    
    if (maze == nullptr) throw std::runtime_error("Maze not found.");
    if (player == nullptr) throw std::runtime_error("Player not found.");
    if (door == nullptr) throw std::runtime_error("Door not found.");


	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
    
}

PlayMode::~PlayMode() {
}

/*
bool PlayMode::checkCollision(glm::vec3 playerPosition, float radius, glm::vec3 min, glm::vec3 max) {
    glm::vec3 closestPoint = glm::clamp(playerPosition, min, max);
    glm::vec3 difference = closestPoint - playerPosition;
    float distanceSquared = glm::dot(difference, difference);
    return distanceSquared <= (radius * radius);
}
 */


bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			A.downs += 1;
			A.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			D.downs += 1;
			D.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			W.downs += 1;
			W.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			S.downs += 1;
			S.pressed = true;
			return true;
        } else if (evt.key.keysym.sym == SDLK_LEFT) {
            left.downs += 1;
            left.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_RIGHT) {
            right.downs += 1;
            right.pressed = true;
            return true;
        } else if (evt.key.keysym.sym == SDLK_SPACE) {
            space.downs += 1;
            space.pressed = true;
            return true;
        }
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			A.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			D.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			W.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			S.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_LEFT) {
            left.pressed = false;
            return true;
        } else if (evt.key.keysym.sym == SDLK_RIGHT) {
            right.pressed = false;
            return true;
        }else if (evt.key.keysym.sym == SDLK_SPACE) {
            space.pressed = false;
            return true;
        }
        
	} else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {

	//slowly rotates through [0,1):
	wobble += elapsed / 10.0f;
	wobble -= std::floor(wobble);

    /*
	hip->rotation = hip_base_rotation * glm::angleAxis(
		glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);
	upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
		glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
		glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
     */

	//move camera:
	{
        //std::cout << "on_ground " << on_ground << "\n";

		//combine inputs into a move:
        constexpr float PlayerSpeed = 2.0f;
        constexpr float JumpSpeed = 7.0f;
        
        //update player position
        glm::vec3 newPosition = player->position;
        
        if (!on_ground) {
            player_velocity += gravity * elapsed;
           
        }
        if (space.pressed && on_ground) {
                player_velocity.y = JumpSpeed;
                on_ground = false;
        }
        
        newPosition += player_velocity * elapsed;
        if (left.pressed) newPosition.x -= PlayerSpeed * elapsed;
        if (right.pressed) newPosition.x += PlayerSpeed * elapsed;
        //check for collisions
        glm::vec3 player_min = newPosition - glm::vec3(player->scale.x, player->scale.y, player->scale.z);
        glm::vec3 player_max = newPosition + glm::vec3(player->scale.x, player->scale.y, player->scale.z);
        
        glm::vec3 door_min = door->position - glm::vec3(door->scale.x, door->scale.y, door->scale.z);
        glm::vec3 door_max = door->position + glm::vec3(door->scale.x, door->scale.y, door->scale.z);
        
        //check if player reaches the door from the left
        if (player_max.x >= door_min.x && player_min.x <= door_max.x &&
            player_max.y >= door_min.y && player_min.y <= door_max.y &&
            player_max.z >= door_min.z && player_min.z <= door_max.z) {
            float overlapX = std::min(player_max.x - door_min.x, door_max.x - player_min.x);
            float overlapY = std::min(player_max.y - door_min.y, door_max.y - player_min.y);
            float overlapZ = std::min(player_max.z - door_min.z, door_max.z - player_min.z);
            if (overlapX < overlapY && overlapX < overlapZ) {
                if (player_min.x < door_min.x) {
                    game_win = true;
                }
            }
        }
        
        if(!game_win){
            on_ground = false;
            for (auto& wall : maze_walls) {
                if (player_max.x >= wall.min.x && player_min.x <= wall.max.x &&
                    player_max.y >= wall.min.y && player_min.y <= wall.max.y &&
                    player_max.z >= wall.min.z && player_min.z <= wall.max.z) {
                    
                   
                    float overlapX = std::min(player_max.x - wall.min.x, wall.max.x - player_min.x);
                    float overlapY = std::min(player_max.y - wall.min.y, wall.max.y - player_min.y);
                    float overlapZ = std::min(player_max.z - wall.min.z, wall.max.z - player_min.z);
                    
                    
                    if (overlapX < overlapY && overlapX < overlapZ) {
                        //horizontal walls
                        if (player_min.x < wall.min.x) {
                            newPosition.x = wall.min.x - player->scale.x;
                        } else {
                            newPosition.x = wall.max.x + player->scale.x;
                        }
                    } else if (overlapY < overlapX && overlapY < overlapZ) {
                        //vertical walls
                        if (player_min.y < wall.min.y) {
                            newPosition.y = wall.min.y - player->scale.y;
                            player_velocity.y = 0.0f;
                        } else {
                            newPosition.y = wall.max.y + player->scale.y;
                            on_ground = true;  // Player is on the ground
                            player_velocity.y = 0.0f;
                        }
                    }
                }
                
            }

        }
      
        player->position = newPosition;
        
        /*
		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
        */
	}

	//reset button press counters:
	A.downs = 0;
	D.downs = 0;
	W.downs = 0;
	S.downs = 0;
    left.downs = 0;
    right.downs = 0;
    space.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.8f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));
        if(!game_win){
            constexpr float H = 0.09f;
            lines.draw_text("Arrow Key moves, Space jumps",
                glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
                glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                glm::u8vec4(0x00, 0x00, 0x00, 0x00));
            float ofs = 2.0f / drawable_size.y;
            lines.draw_text("Arrow Key moves, Space jumps",
                glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
                glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        }else{
            constexpr float H = 0.09f;
            float ofs = 2.0f / drawable_size.y;
            lines.draw_text("You Won",
                glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
                glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                glm::u8vec4(0xff, 0xff, 0xff, 0x00));
        }
	}
}
