#ifndef HEIGHTFIELD_H
#define HEIGHTFIELD_H

#include "visualization.h"

#include "primitives.h"
#include "text.h"

namespace vis
{
	/**
	 * @brief The Heightfield class renders fields of normal distribution parameters using a 3D height field based visualization.
	 */
	class Heightfield : public Visualization
	{
	public:
		explicit Heightfield(InputManager& input, const std::vector<Field>& fields);
		~Heightfield() = default;

		virtual void setup_data() override;
		virtual void setup_shaders() override;

		virtual void update(float delta_time, float total_time) override;
		virtual void draw() const override;

		virtual glm::ivec2 point_under_cursor() const override;

	protected:
		void setup_axes();

		std::vector<std::string> _vertex_shaders{"Shader/heightfield_vs.glsl"};
		std::vector<std::string> _fragment_shaders{"Shader/heightfield_fs.glsl",
												   "Shader/palette.glsl"};

		// Data
		glm::vec2 _mean_bounds;	// (min, max)
		glm::vec2 _dev_bounds;	// (min, max)

	private:
		// Axes
		Primitives _axes;
		Text _axes_labels;
		std::vector<float> _axes_divisions;

		// Cursor
		Primitives _cursor_indicator{{{0.f, 0.f, 0.f}, {0.f, 0.f, 1.5f}}};

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
