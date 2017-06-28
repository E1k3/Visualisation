#ifndef HEIGHTFIELDRENDERER_H
#define HEIGHTFIELDRENDERER_H

#include <algorithm>
#include <string>
#include <chrono>

#include <glm/glm.hpp>

#include "renderer.h"
#include "Data/ensemblemanager.h"
#include "inputmanager.h"

namespace vis
{
	class HeightfieldRenderer : public Renderer
	{
	public:
		/// @brief HeightfieldRenderer Constructor.
		explicit HeightfieldRenderer(EnsembleManager* ensemble, InputManager* input);
		void draw(float delta_time);

	private:
		EnsembleManager* _ensemble;
		InputManager* _input;

		unsigned _num_vertices;
		GLint _mvp_uniform;
		GLint _bounds_uniform;

		glm::vec3 _cam_position{-1.8f, -1.8f, 1.8f};
		glm::vec3 _cam_direction{-_cam_position};
	};
}

#endif // HEIGHTFIELDRENDERER_H
