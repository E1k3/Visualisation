#ifndef HEIGHTFIELD_H
#define HEIGHTFIELD_H

#include "visualisation.h"

namespace vis
{
	class Heightfield : public Visualisation
	{
	public:
		explicit Heightfield(InputManager& input, const std::vector<Field>& fields);
		~Heightfield() = default;

		virtual void setup_data() override;
		virtual void setup_shaders() override;

		virtual void update(float delta_time, float total_time) override;
		virtual void draw() const override;

	protected:
		std::vector<std::string> _vertex_shaders{"/home/eike/Documents/Code/Visualisation/Shader/heightfield_vs.glsl"};
		std::vector<std::string> _fragment_shaders{"/home/eike/Documents/Code/Visualisation/Shader/heightfield_fs.glsl",
												   "/home/eike/Documents/Code/Visualisation/Shader/palette.glsl"};

		// Data
		glm::vec2 _mean_bounds;	// (min, max)
		glm::vec2 _dev_bounds;	// (min, max)

	private:
		// Camera
		float _scale{1.f};
		glm::vec3 _camera_position{-2.f, -2.f, 2.f};

		// Uniform locations
		GLint _mvp_loc{-1};
		GLint _bounds_loc{-1};
		GLint _time_loc{-1};
	};
}

#endif // HEIGHTFIELD_H
